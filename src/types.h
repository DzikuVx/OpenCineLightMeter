#include "Arduino.h"

#pragma once

#ifndef TYPES_H
#define TYPES_H

enum lightMeterMode_e {
    LIGHT_METER_TYPE_INCIDENT = 0,
    LIGHT_METER_TYPE_REFLECTED,
    LIGHT_METER_TYPE_COUNT
};

enum lightMeterCompute_e {
    LIGHT_METER_MODE_APERTURE = 0,
    LIGHT_METER_MODE_SHUTTER,
    LIGHT_METER_MODE_ISO,
    LIGHT_METER_MODE_ND,
    LIGHT_METER_MODE_COUNT
};

enum oledPages_e {
    OLED_PAGE_NONE,
    OLED_PAGE_APERTURE,
    OLED_PAGE_SHUTTER,
    OLED_PAGE_ISO,
    OLED_PAGE_ND,
    OLED_PAGE_ERROR
};

enum adjustSetting_e {
    ADJUST_SETTING_ISO = 0,
    ADJUST_SETTING_APERTURE,
    ADJUST_SETTING_SHUTTER,
    ADJUST_SETTING_ND_FILTER,
    ADJUST_SETTING_TYPE,
    ADJUST_SETTING_MODE,
    ADJUST_SETTING_COUNT
};

typedef struct settings_s
{
	int8_t isoIndex = 0;
    int8_t apertureIndex = 0;
    int8_t shutterIndex = 6;
    int8_t ndFilterIndex = 0;

    lightMeterMode_e type = LIGHT_METER_TYPE_INCIDENT;
    lightMeterCompute_e mode = LIGHT_METER_MODE_APERTURE;
    adjustSetting_e adjustSetting = ADJUST_SETTING_ISO;

} settings_t;

#define ISO_TABLE_OFFSET 2
#define SHUTTER_TABLE_OFFSET 6

#define SHUTTER_INDEX_MIN -6
#define SHUTTER_INDEX_MAX 15 

#define ISO_INDEX_MIN -2
#define ISO_INDEX_MAX 10

#define ND_FILTER_INDEX_MIN 0
#define ND_FILTER_INDEX_MAX 10

#define APERTURE_INDEX_MIN 0
#define APERTURE_INDEX_MAX 10
#define APERTURE_TABLE_OFFSET 0

#endif