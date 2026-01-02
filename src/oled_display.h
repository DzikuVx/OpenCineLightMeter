#ifndef OLED_DISPLAY
#define OLED_DISPLAY

#include "SSD1306.h"
#include "types.h"

#define OLED_COL_COUNT 64
#define OLED_DISPLAY_PAGE_COUNT 1

extern float lux;
extern float ev;
extern float evIso;
extern float reflectedEv;
extern float incidentEv;

extern settings_t settings;
extern float outputValue;

extern String ISO_TABLE[];
extern String APERTURE_TABLE[];
extern String SHUTTER_TABLE[];
extern String TYPE_TABLE[];

class OledDisplay {
    public:
        OledDisplay(SSD1306 *display);
        void init();
        void loop();
        void setPage(uint8_t page);
        void forceDisplay();
        void setOnlyForcedDisplay(bool onlyForcedDisplay);
    private:
        SSD1306 *_display;
        void renderPageAperture();
        void renderPageShutter();
        void renderWidgetEv();
        void page();
        uint8_t _page = OLED_PAGE_NONE;
        bool _forceDisplay = false;
        bool _onlyForcedDisplay = false;
};


#endif