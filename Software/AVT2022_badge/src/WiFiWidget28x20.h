#include <PlatformDetermination.h>

const uint8_t WiFi_NoConnection[] PROGMEM = {
    0x00, 0xfc, 0x03, 0x00, 0x00, 0xfc, 0x03, 0x00, 0xc0, 0x03, 0x3c, 0x00,
    0xc0, 0x03, 0x3c, 0x00, 0x3c, 0x00, 0xc0, 0x03, 0x3c, 0x00, 0xc0, 0x03,
    0x03, 0x00, 0x00, 0x0c, 0x03, 0x00, 0x00, 0x0c, 0x0c, 0x00, 0x00, 0x03,
    0x0c, 0x00, 0x00, 0x03, 0x30, 0x00, 0xc0, 0x00, 0x30, 0x00, 0xc0, 0x00,
    0xc0, 0x00, 0x30, 0x00, 0xc0, 0x00, 0x30, 0x00, 0x00, 0x03, 0x0c, 0x00,
    0x00, 0x03, 0x0c, 0x00, 0x00, 0x0c, 0x03, 0x00, 0x00, 0x0c, 0x03, 0x00,
    0x00, 0xf0, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x00
};

const uint8_t WiFi_One[] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x00,
    0x00, 0x0c, 0x03, 0x00, 0x00, 0x0c, 0x03, 0x00
};

const uint8_t WiFi_Two[] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xfc, 0x03, 0x00, 0x00, 0xfc, 0x03, 0x00, 0x00, 0x03, 0x0c, 0x00,
    0x00, 0x03, 0x0c, 0x00, 0x00, 0xf0, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x00,
    0x00, 0x0c, 0x03, 0x00, 0x00, 0x0c, 0x03, 0x00
};

const uint8_t WiFi_Three[] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xfc, 0x03, 0x00, 0x00, 0xfc, 0x03, 0x00, 0xc0, 0x03, 0x3c, 0x00,
    0xc0, 0x03, 0x3c, 0x00, 0x30, 0x00, 0xc0, 0x00, 0x30, 0x00, 0xc0, 0x00,
    0x00, 0xfc, 0x03, 0x00, 0x00, 0xfc, 0x03, 0x00, 0x00, 0x03, 0x0c, 0x00,
    0x00, 0x03, 0x0c, 0x00, 0x00, 0xf0, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x00,
    0x00, 0x0c, 0x03, 0x00, 0x00, 0x0c, 0x03, 0x00
};

const uint8_t WiFi_Four[] PROGMEM = {
    0x00, 0xfc, 0x03, 0x00, 0x00, 0xfc, 0x03, 0x00, 0xc0, 0x03, 0x3c, 0x00,
    0xc0, 0x03, 0x3c, 0x00, 0x3c, 0x00, 0xc0, 0x03, 0x3c, 0x00, 0xc0, 0x03,
    0x03, 0xfc, 0x03, 0x0c, 0x03, 0xfc, 0x03, 0x0c, 0xc0, 0x03, 0x3c, 0x00,
    0xc0, 0x03, 0x3c, 0x00, 0x30, 0x00, 0xc0, 0x00, 0x30, 0x00, 0xc0, 0x00,
    0x00, 0xfc, 0x03, 0x00, 0x00, 0xfc, 0x03, 0x00, 0x00, 0x03, 0x0c, 0x00,
    0x00, 0x03, 0x0c, 0x00, 0x00, 0xf0, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x00,
    0x00, 0x0c, 0x03, 0x00, 0x00, 0x0c, 0x03, 0x00
};


static const uint8_t* iconsWiFi[] PROGMEM = { WiFi_NoConnection, WiFi_One, WiFi_Two, WiFi_Three, WiFi_Four };
