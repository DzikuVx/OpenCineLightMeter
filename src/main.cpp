#include <Arduino.h>
#include <Adafruit_VEML7700.h>
#include "SSD1306.h"
#include "oled_display.h"
#include "QmuTactile.h"
#include "types.h"
#include <Wire.h>
#include "eeprom_storage.h"

#define LIGHT_SENSOR_TASK_MS 250

#define PIN_BUTTON_MODE 26

#define PIN_BUTTON_UP 13
#define PIN_BUTTON_DOWN 12
#define PIN_BUTTON_LEFT 14
#define PIN_BUTTON_RIGHT 27

#define PIN_BUTTON_HOLD 0

#define PIN_OLED_SDA 4
#define PIN_OLED_SCL 15
#define PIN_OLED_RST 16
#define OLED_ADDRESS 0x3c

#define EEPROM_SIZE 64
#define EEPROM_IDENT_ADDRESS 0
#define EEPROM_SETTINGS_ADDRESS 1
#define EEPROM_IDENT 0x69

QmuTactile buttonMode(PIN_BUTTON_MODE);

QmuTactile buttonUp(PIN_BUTTON_UP);
QmuTactile buttonDown(PIN_BUTTON_DOWN);
QmuTactile buttonLeft(PIN_BUTTON_LEFT);
QmuTactile buttonRight(PIN_BUTTON_RIGHT);

QmuTactile buttonHold(PIN_BUTTON_HOLD);

SSD1306 display(OLED_ADDRESS, PIN_OLED_SDA, PIN_OLED_SCL);
OledDisplay oledDisplay(&display);
Adafruit_VEML7700 veml = Adafruit_VEML7700();
TwoWire I2C1 = TwoWire(0);

String ISO_TABLE[] = {"25", "50", "100", "200", "400", "800", "1600", "3200", "6400", "128K", "256K"};
String APERTURE_TABLE[] = {"1.0", "1.4", "2", "2.8", "4", "5.6", "8", "11", "16", "22", "32"};
String SHUTTER_TABLE[] = {"60s", "30s", "15s", "8s", "4s", "2s", "1s", "1/2s", "1/4s", "1/8s", "1/15s", "1/30s", "1/60s", "1/125s", "1/250s", "1/500s", "1/1000s", "1/2000s", "1/4000s", "1/8000s", "1/16k", "1/32k"};
String TYPE_TABLE[] = {"Incident", "Reflected"};

static const adjustSetting_e ADJUST_SETTING_MATRIX[3][3] = {
  {ADJUST_SETTING_ISO,     ADJUST_SETTING_SHUTTER,  ADJUST_SETTING_ND_FILTER},
  {ADJUST_SETTING_SHUTTER, ADJUST_SETTING_APERTURE, ADJUST_SETTING_ND_FILTER},
  {ADJUST_SETTING_ISO,     ADJUST_SETTING_APERTURE, ADJUST_SETTING_ND_FILTER}
};

settings_t settings;

TaskHandle_t lightSensorTask;

void lightSensorTaskHandler(void *pvParameters)
{
  portTickType xLastWakeTime;
  const portTickType xPeriod = LIGHT_SENSOR_TASK_MS / portTICK_PERIOD_MS;
  xLastWakeTime = xTaskGetTickCount();

  for (;;)
  {
    lux = veml.readLux(VEML_LUX_AUTO);

    reflectedEv = log2(lux) + 3;
    incidentEv = log2(lux / 2.5f);

    if (settings.type == LIGHT_METER_TYPE_REFLECTED)
    {
      ev = reflectedEv;
    } else {
      ev = incidentEv;
    }

    // effective EV is the EV at ISO 100 + isoIndex where 0 is ISO100, 1 is ISO200, 2 is ISO400, etc.
    ev += settings.isoIndex;

    // Substract the effect of used ND filter
    ev -= settings.ndFilterIndex;

    if (settings.mode == LIGHT_METER_MODE_APERTURE) {

      const float shutter = 1.0f / pow(2, settings.shutterIndex);

      // Store computed aperture
      outputValue = sqrt(shutter * pow(2, ev));
    } else if (settings.mode == LIGHT_METER_MODE_SHUTTER){
      // Compute shutter time (seconds) from EV and selected aperture
      // Equation derivation from aperture mode: f^2 = shutter * 2^ev
      // => shutter = f^2 / 2^ev. With full-stop aperture indexing,
      // f^2 equals 2^(apertureIndex).
      const float apertureSquared = powf(2.0f, settings.apertureIndex);
      const float shutter = apertureSquared / powf(2.0f, ev);

      // Store computed shutter (seconds)
      outputValue = shutter;
    } else {
      // TODO This is ISO case
      //  ev = incidentEv;
    }

    oledDisplay.forceDisplay();

    // Put task to sleep
    vTaskDelayUntil(&xLastWakeTime, xPeriod);
  }

  vTaskDelete(NULL);
}

void setup() {

  while (!EEPROM.begin(EEPROM_SIZE)) {
    true;
  }

  Serial.begin(115200);

  // Setup I2C OLED
  pinMode(PIN_OLED_RST, OUTPUT);
  digitalWrite(PIN_OLED_RST, LOW); // set GPIO16 low to reset OLED
  delay(50);
  digitalWrite(PIN_OLED_RST, HIGH); // while OLED is running, must set GPIO16 to high
  Wire.begin(PIN_OLED_SDA, PIN_OLED_SCL);

  oledDisplay.init();
  oledDisplay.setPage(OLED_PAGE_EV);
  oledDisplay.setOnlyForcedDisplay(true);

  if (!veml.begin()) {
    oledDisplay.setPage(OLED_PAGE_ERROR);
    oledDisplay.forceDisplay();
    delay(1000);
    while (true)
      ;
  }

  //Init all buttons
  buttonMode.start();
  buttonUp.start();
  buttonDown.start();
  buttonLeft.start();
  buttonRight.start();

  buttonHold.start();

  xTaskCreatePinnedToCore(
      lightSensorTaskHandler, /* Function to implement the task */
      "lightSensorTask",      /* Name of the task */
      10000,                  /* Stack size in words */
      NULL,                   /* Task input parameter */
      0,                      /* Priority of the task */
      &lightSensorTask,       /* Task handle. */
      0);

  if (EEPROM.read(EEPROM_IDENT_ADDRESS) == EEPROM_IDENT) {
    EEPROM_readAnything(EEPROM_SETTINGS_ADDRESS, settings);
  } else {
    EEPROM.write(EEPROM_IDENT_ADDRESS, EEPROM_IDENT);
    EEPROM_writeAnything(EEPROM_SETTINGS_ADDRESS, settings);
    EEPROM.commit();
  }
}

float lux;
float ev;
float evIso;
float reflectedEv;
float incidentEv;

float outputValue = 0;

uint8_t propertyChangeIndex = 0;

void loop()
{

  buttonMode.loop();

  buttonUp.loop();
  buttonDown.loop();
  buttonLeft.loop();
  buttonRight.loop();
  
  buttonHold.loop();

  // if (buttonMode.getState() == TACTILE_STATE_LONG_PRESS) {

  //     if (settings.mode == LIGHT_METER_MODE_APERTURE) {
  //         settings.mode = LIGHT_METER_MODE_SHUTTER;
  //     } else if (settings.mode == LIGHT_METER_MODE_SHUTTER) {
  //         settings.mode = LIGHT_METER_MODE_ISO;
  //     } else if (settings.mode == LIGHT_METER_MODE_ISO) {
  //         settings.mode = LIGHT_METER_MODE_ND;
  //     } else {
  //         settings.mode = LIGHT_METER_MODE_APERTURE;
  //     }

  //     oledDisplay.forceDisplay();
  // }

  // Button logic

  //Up and down buttons select a property to change based on current mode
  if (buttonUp.getState() == TACTILE_STATE_SHORT_PRESS) {
    propertyChangeIndex--;
    if (propertyChangeIndex < 0) {
      propertyChangeIndex = 2;
    }
    
    settings.adjustSetting = ADJUST_SETTING_MATRIX[settings.mode][propertyChangeIndex];
    
    oledDisplay.forceDisplay();
  }
  if (buttonDown.getState() == TACTILE_STATE_SHORT_PRESS) {
    propertyChangeIndex++;
    if (propertyChangeIndex > 2) {
      propertyChangeIndex = 0;
    }
    
    settings.adjustSetting = ADJUST_SETTING_MATRIX[settings.mode][propertyChangeIndex];

    oledDisplay.forceDisplay();
  }

  if (settings.mode == LIGHT_METER_MODE_APERTURE) {

    if (buttonLeft.getState() == TACTILE_STATE_SHORT_PRESS) {
      if (settings.adjustSetting == ADJUST_SETTING_ISO) {
        settings.isoIndex--;

        if (settings.isoIndex < ISO_INDEX_MIN) {
          settings.isoIndex = ISO_INDEX_MIN;
        }
      } else if (settings.adjustSetting == ADJUST_SETTING_SHUTTER) {

        settings.shutterIndex--;

        if (settings.shutterIndex < SHUTTER_INDEX_MIN) {
          settings.shutterIndex = SHUTTER_INDEX_MIN;
        }
      } else if (settings.adjustSetting == ADJUST_SETTING_TYPE) {

        if (settings.type == LIGHT_METER_TYPE_INCIDENT) {
          settings.type = LIGHT_METER_TYPE_REFLECTED;
        } else {
          settings.type = LIGHT_METER_TYPE_INCIDENT;
        }
      } else if (settings.adjustSetting == ADJUST_SETTING_ND_FILTER) {

        settings.ndFilterIndex--;

        if (settings.ndFilterIndex < ND_FILTER_INDEX_MIN) {
          settings.ndFilterIndex = ND_FILTER_INDEX_MIN;
        }
      }
      
      oledDisplay.forceDisplay();
      EEPROM_writeAnything(EEPROM_SETTINGS_ADDRESS, settings);
      EEPROM.commit();
    }

    if (buttonRight.getState() == TACTILE_STATE_SHORT_PRESS)
    {
      if (settings.adjustSetting == ADJUST_SETTING_ISO)
      {
        settings.isoIndex++;

        if (settings.isoIndex > ISO_INDEX_MAX)
        {
          settings.isoIndex = ISO_INDEX_MAX;
        }
      }
      else if (settings.adjustSetting == ADJUST_SETTING_SHUTTER)
      {
        settings.shutterIndex++;

        if (settings.shutterIndex > SHUTTER_INDEX_MAX)
        {
          settings.shutterIndex = SHUTTER_INDEX_MAX;
        }
      }
      else if (settings.adjustSetting == ADJUST_SETTING_TYPE)
      {

        if (settings.type == LIGHT_METER_TYPE_INCIDENT)
        {
          settings.type = LIGHT_METER_TYPE_REFLECTED;
        }
        else
        {
          settings.type = LIGHT_METER_TYPE_INCIDENT;
        }
      }
      else if (settings.adjustSetting == ADJUST_SETTING_ND_FILTER)
      {

        settings.ndFilterIndex++;

        if (settings.ndFilterIndex > ND_FILTER_INDEX_MAX)
        {
          settings.ndFilterIndex = ND_FILTER_INDEX_MAX;
        }
      }

      oledDisplay.forceDisplay();
      EEPROM_writeAnything(EEPROM_SETTINGS_ADDRESS, settings);
      EEPROM.commit();
    }
  }

  // static uint32_t nextSerialUpdate = 0;
  // if (millis() > nextSerialUpdate) {
  //     nextSerialUpdate = millis() + 100;
  //     Serial.print("reflected: "); Serial.println(reflectedEv);
  //     Serial.print("incident: "); Serial.println(incidentEv);
  // }

  oledDisplay.loop();
}
