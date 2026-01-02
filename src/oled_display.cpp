#include "oled_display.h"
#include "Arduino.h"
#include "types.h"
#include "Lato_Bold_8.h"

OledDisplay::OledDisplay(SSD1306 *display) {
    _display = display;
}

void OledDisplay::init() {
    _display->init();
    _display->setFont(ArialMT_Plain_10);
}

void OledDisplay::loop() {
    page();
}

void OledDisplay::forceDisplay() {
    _forceDisplay = true;
}

void OledDisplay::setOnlyForcedDisplay(bool onlyForcedDisplay) {
    _onlyForcedDisplay = onlyForcedDisplay;
}

void OledDisplay::setPage(uint8_t page) {
    _page = page;
}

void OledDisplay::page() {

    static uint32_t lastUpdate = 0;

    bool timePassed = millis() - lastUpdate >= 100;

    if (_onlyForcedDisplay && timePassed) {
        timePassed = false;
    }

    bool toDisplay = _forceDisplay || timePassed || lastUpdate == 0;

    if (!toDisplay) {
        return;
    }

    _forceDisplay = false;

    switch (_page) {
        
        case OLED_PAGE_APERTURE:
            renderPageAperture();
            break;

        case OLED_PAGE_SHUTTER:
            renderPageShutter();
            break;

        case OLED_PAGE_ERROR:
            _display->clear();
            _display->setFont(ArialMT_Plain_24);
            _display->drawString(0, 0, "Error");
            _display->display();
            break;
    }

    lastUpdate = millis();
}

void OledDisplay::renderWidgetEv() {
    _display->setFont(ArialMT_Plain_16);
    _display->drawString(4, 48, String(ev, 1));
    _display->setFont(Lato_Bold_8);
    _display->drawString((ev >= 10) ? 38:34, 54, "EV");
}

void OledDisplay::renderPageAperture() {
    _display->clear();

    renderWidgetEv();

    if (settings.adjustSetting == ADJUST_SETTING_ISO) {
        _display->drawCircle(68, 10, 3);
    } else if (settings.adjustSetting == ADJUST_SETTING_SHUTTER)  {
        _display->drawCircle(68, 31, 3);
    } else {
        _display->drawCircle(68, 52, 3);
    }

    //We want to round the aperture to the nearest 1/3 stop
    float aperture = outputValue;
    //Convert aperture to f-stop
    float fStop = 2.0 * log2(aperture);
    float roundedFStop = round(fStop * 3.0) / 3.0;
    float roundedAperture = pow(2.0, roundedFStop / 2.0);
    float iso = 100 * pow(2, settings.isoIndex);
    int32_t nd = pow(2, settings.ndFilterIndex);  

    if (outputValue < 0.5) {
        _display->setFont(ArialMT_Plain_24);
        _display->drawString(0, 0, "-low-");
    } else if (outputValue > 32) {
        _display->setFont(ArialMT_Plain_24);
        _display->drawString(0, 0, "-high-");
    } else {
        _display->setFont(ArialMT_Plain_24);
        _display->drawString(0, 0, "f/" + String(roundedAperture, 1));
    }

    _display->setFont(Lato_Bold_8);
    _display->drawString(76, 0, "ISO");
    _display->setFont(ArialMT_Plain_10);
    _display->drawString(76, 9, String(iso, 0));

    _display->setFont(Lato_Bold_8);
    _display->drawString(76, 22, "Shutter");
    _display->setFont(ArialMT_Plain_10);
    _display->drawString(76, 31, SHUTTER_TABLE[settings.shutterIndex + SHUTTER_TABLE_OFFSET]);

    _display->setFont(Lato_Bold_8);
    _display->drawString(76, 44, "ND Filter");
    _display->setFont(ArialMT_Plain_10);
    if (settings.ndFilterIndex > 0) {
        _display->drawString(76, 53, "ND" + String(nd));
    } else {
        _display->drawString(76, 53, "None");
    }

    _display->setFont(Lato_Bold_8);
    if (settings.type == LIGHT_METER_TYPE_INCIDENT) {
        _display->drawString(4, 38, "Incident");
    } else {
        _display->drawString(4, 38, "Reflected");
    }

     
    _display->display();
}

void OledDisplay::renderPageShutter() {
    _display->clear();

    renderWidgetEv();

    if (settings.adjustSetting == ADJUST_SETTING_ISO) {
        _display->drawCircle(68, 10, 3);
    } else if (settings.adjustSetting == ADJUST_SETTING_APERTURE)  {
        _display->drawCircle(68, 31, 3);
    } else {
        _display->drawCircle(68, 52, 3);
    }

    // Convert computed shutter seconds (outputValue) to nearest standard label
    float shutterSeconds = outputValue;

    if (shutterSeconds <= 0.0f) {
        _display->setFont(ArialMT_Plain_24);
        _display->drawString(0, 0, "-high-");
    } else {
        float shutterIndexF = -log2(shutterSeconds);
        int shutterIndexRounded = (int)round(shutterIndexF);

        if (shutterIndexRounded < SHUTTER_INDEX_MIN) {
            shutterIndexRounded = SHUTTER_INDEX_MIN;
        } else if (shutterIndexRounded > SHUTTER_INDEX_MAX) {
            shutterIndexRounded = SHUTTER_INDEX_MAX;
        }

        uint8_t tableIndex = shutterIndexRounded + SHUTTER_TABLE_OFFSET;

        _display->setFont(ArialMT_Plain_24);
        _display->drawString(0, 0, SHUTTER_TABLE[tableIndex]);
    }

    float iso = 100 * pow(2, settings.isoIndex);
    int32_t nd = pow(2, settings.ndFilterIndex);  

    _display->setFont(Lato_Bold_8);
    _display->drawString(76, 0, "ISO");
    _display->setFont(ArialMT_Plain_10);
    _display->drawString(76, 9, String(iso, 0));

    _display->setFont(Lato_Bold_8);
    _display->drawString(76, 22, "Aperture");
    _display->setFont(ArialMT_Plain_10);
    _display->drawString(76, 31, APERTURE_TABLE[settings.apertureIndex + APERTURE_TABLE_OFFSET]);

    _display->setFont(Lato_Bold_8);
    _display->drawString(76, 44, "ND Filter");
    _display->setFont(ArialMT_Plain_10);
    if (settings.ndFilterIndex > 0) {
        _display->drawString(76, 53, "ND" + String(nd));
    } else {
        _display->drawString(76, 53, "None");
    }

    _display->setFont(Lato_Bold_8);
    if (settings.type == LIGHT_METER_TYPE_INCIDENT) {
        _display->drawString(4, 38, "Incident");
    } else {
        _display->drawString(4, 38, "Reflected");
    }
     
    _display->display();
}