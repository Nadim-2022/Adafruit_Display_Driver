/*
 This is a library for the HX8357 320x480 TFT display
 Create funtion to use HX8357 thorugh SPI interface with STM32H743ZI
 Use STM32H743ZI SPI1 and HAL library to write the functions to control HX8357
 Write all the functions to control HX8357
 Write the functions to draw the pixel, draw the line, draw the rectangle, draw the circle, draw the triangle, draw the character, draw the string, fill the rectangle, fill the circle
    Write the functions to control the backlight of HX8357
    Write the functions to control the rotation of HX8357
    Write the functions to control the display on and off
    Write the functions to control the sleep mode of HX8357
    Write the functions to control the inversion of HX8357
    Write the functions to control the gamma of HX8357
    Write the functions to control the color of HX8357
    Write the functions to control the brightness of HX8357
    Write the functions to control the contrast of HX8357
  Use c language to write the functions

 */
#include "Adafruit_HX8357.h"
#include "malloc.h"
#include "string.h"

#define DELAY 0x80

#define MADCTL_MY  0x80 ///< Bottom to top
#define MADCTL_MX  0x40 ///< Right to left
#define MADCTL_MV  0x20 ///< Reverse Mode
#define MADCTL_ML  0x10 ///< LCD refresh Bottom to top
#define MADCTL_RGB 0x00 ///< Red-Green-Blue pixel order
#define MADCTL_BGR 0x08 ///< Blue-Green-Red pixel order
#define MADCTL_MH  0x04 ///< LCD refresh right to left

// THIS REALLY SHOULD GO IN SPITFT (PART OF ADAFRUIT_GFX),
// AND CAN BE FURTHER EXPANDED (e.g. add 12 MHz for M0, 24 for M4),
// BUT TEMPORARILY FOR NOW IT'S HERE:

#if defined (ARDUINO_ARCH_ARC32)
#define SPI_DEFAULT_FREQ 16000000
#elif defined (__AVR__) || defined(TEENSYDUINO)
#define SPI_DEFAULT_FREQ 8000000
#elif defined(ESP8266) || defined (ARDUINO_MAXIM)
#define SPI_DEFAULT_FREQ 16000000
#elif defined(ESP32)
#define SPI_DEFAULT_FREQ 24000000
#elif defined(RASPI)
#define SPI_DEFAULT_FREQ 24000000
#else
#define SPI_DEFAULT_FREQ 24000000
#endif

static const uint8_t
        initb[] = {
        HX8357B_SETPOWER, 3,
        0x44, 0x41, 0x06,
        HX8357B_SETVCOM, 2,
        0x40, 0x10,
        HX8357B_SETPWRNORMAL, 2,
        0x05, 0x12,
        HX8357B_SET_PANEL_DRIVING, 5,
        0x14, 0x3b, 0x00, 0x02, 0x11,
        HX8357B_SETDISPLAYFRAME, 1,
        0x0c,                      // 6.8mhz
        HX8357B_SETPANELRELATED, 1,
        0x01,                      // BGR
        0xEA, 3,                     // seq_undefined1, 3 args
        0x03, 0x00, 0x00,
        0xEB, 4,                     // undef2, 4 args
        0x40, 0x54, 0x26, 0xdb,
        HX8357B_SETGAMMA, 12,
        0x00, 0x15, 0x00, 0x22, 0x00, 0x08, 0x77, 0x26, 0x66, 0x22, 0x04, 0x00,
        HX8357_MADCTL, 1,
        0xC0,
        HX8357_COLMOD, 1,
        0x55,
        HX8357_PASET, 4,
        0x00, 0x00, 0x01, 0xDF,
        HX8357_CASET, 4,
        0x00, 0x00, 0x01, 0x3F,
        HX8357B_SETDISPMODE, 1,
        0x00,                      // CPU (DBI) and internal oscillation ??
        HX8357_SLPOUT, 0x80 + 120/5, // Exit sleep, then delay 120 ms
        HX8357_DISPON, 0x80 +  10/5, // Main screen turn on, delay 10 ms
        0                            // END OF COMMAND LIST
}, initd[] = {
        HX8357_SWRESET, 0x80 + 10/5, // Soft reset, then delay 10 ms
        HX8357D_SETC, 3,
        0xFF, 0x83, 0x57,
        0xFF, 0x80 + 300/5,          // No command, just delay 300 ms
        HX8357_SETRGB, 4,
        0x80, 0x00, 0x06, 0x06,    // 0x80 enables SDO pin (0x00 disables)
        HX8357D_SETCOM, 1,
        0x25,                      // -1.52V
        HX8357_SETOSC, 1,
        0x68,                      // Normal mode 70Hz, Idle mode 55 Hz
        HX8357_SETPANEL, 1,
        0x05,                      // BGR, Gate direction swapped
        HX8357_SETPWR1, 6,
        0x00,                      // Not deep standby
        0x15,                      // BT
        0x1C,                      // VSPR
        0x1C,                      // VSNR
        0x83,                      // AP
        0xAA,                      // FS
        HX8357D_SETSTBA, 6,
        0x50,                      // OPON normal
        0x50,                      // OPON idle
        0x01,                      // STBA
        0x3C,                      // STBA
        0x1E,                      // STBA
        0x08,                      // GEN
        HX8357D_SETCYC, 7,
        0x02,                      // NW 0x02
        0x40,                      // RTN
        0x00,                      // DIV
        0x2A,                      // DUM
        0x2A,                      // DUM
        0x0D,                      // GDON
        0x78,                      // GDOFF
        HX8357D_SETGAMMA, 34,
        0x02, 0x0A, 0x11, 0x1d, 0x23, 0x35, 0x41, 0x4b, 0x4b,
        0x42, 0x3A, 0x27, 0x1B, 0x08, 0x09, 0x03, 0x02, 0x0A,
        0x11, 0x1d, 0x23, 0x35, 0x41, 0x4b, 0x4b, 0x42, 0x3A,
        0x27, 0x1B, 0x08, 0x09, 0x03, 0x00, 0x01,
        HX8357_COLMOD, 1,
        0x55,                      // 16 bit
        HX8357_MADCTL, 1,
        0xC0,
        HX8357_TEON, 1,
        0x00,                      // TW off
        HX8357_TEARLINE, 2,
        0x00, 0x02,
        HX8357_SLPOUT, 0x80 + 150/5, // Exit Sleep, then delay 150 ms
        HX8357_DISPON, 0x80 +  50/5, // Main screen turn on, delay 50 ms
        0,                           // END OF COMMAND LIST
};

// only  c language

void Adafruit_HX8357_CS_HIGH(void) {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET); // HIGH
}

void Adafruit_HX8357_CS_LOW(void) {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET); // LOW
}

void Adafruit_HX8357_DC_HIGH(void) {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET); // HIGH
}

void Adafruit_HX8357_DC_LOW(void) {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET); // LOW
}

// Command execution function

void Adafruit_HX8357_commandList(const uint8_t *addr) {
    uint8_t  numCommands, numArgs;
    uint16_t ms;

    numCommands = *addr++;   // Number of commands to follow
    while (numCommands--) { // For each command...
        Adafruit_HX8357_DC_LOW();
        Adafruit_HX8357_CS_LOW();
        uint8_t cmd = *addr++;
        HAL_SPI_Transmit(&hspi1, &cmd, 1, 100);
        numArgs = *addr++;   // Number of args to follow
        ms = numArgs & DELAY; // If high bit set, delay follows args
        numArgs &= ~DELAY;    // Mask out delay bit
        while (numArgs--) { // For each argument...
            uint8_t arg = *addr++;
            HAL_SPI_Transmit(&hspi1, &arg, 1, 100);
        }
        Adafruit_HX8357_CS_HIGH();
        Adafruit_HX8357_DC_HIGH();
        if (ms) {
            ms = *addr++; // Read post-command delay time (ms)
            if (ms == 255) ms = 500; // If 255, delay for 500 ms
            HAL_Delay(ms);
        }
    }
}

void Adafruit_HX8357_begin(void) {
    Adafruit_HX8357_CS_HIGH();
    Adafruit_HX8357_DC_HIGH();
    Adafruit_HX8357_CS_LOW();
    Adafruit_HX8357_commandList(initb);
    Adafruit_HX8357_CS_HIGH();
    Adafruit_HX8357_DC_HIGH();
    Adafruit_HX8357_CS_LOW();
    Adafruit_HX8357_commandList(initd);
    Adafruit_HX8357_CS_HIGH();
}

void Adafruit_HX8357_setAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    uint32_t xa = ((uint32_t) x << 16) | (x + w - 1);
    uint32_t ya = ((uint32_t) y << 16) | (y + h - 1);
    Adafruit_HX8357_DC_LOW();
    Adafruit_HX8357_CS_LOW();
    uint8_t cmd = HX8357_CASET;
    HAL_SPI_Transmit(&hspi1, &cmd, 1, 100);
    Adafruit_HX8357_DC_HIGH();
    HAL_SPI_Transmit(&hspi1, (uint8_t *) &xa, 4, 100);
    Adafruit_HX8357_DC_LOW();
    cmd = HX8357_PASET;
    HAL_SPI_Transmit(&hspi1, &cmd, 1, 100);
    Adafruit_HX8357_DC_HIGH();
    HAL_SPI_Transmit(&hspi1, (uint8_t *) &ya, 4, 100);
    Adafruit_HX8357_CS_HIGH();
}

void Adafruit_HX8357_fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    Adafruit_HX8357_setAddrWindow(x, y, w, h);
    Adafruit_HX8357_DC_HIGH();
    Adafruit_HX8357_CS_LOW();
    uint8_t cmd = HX8357_RAMWR;
    HAL_SPI_Transmit(&hspi1, &cmd, 1, 100);
    Adafruit_HX8357_DC_LOW();
    uint8_t hi = color >> 8, lo = color;
    for (y = h; y > 0; y--) {
        for (x = w; x > 0; x--) {
            HAL_SPI_Transmit(&hspi1, &hi, 1, 100);
            HAL_SPI_Transmit(&hspi1, &lo, 1, 100);
        }
    }
    Adafruit_HX8357_CS_HIGH();
}


void Adafruit_HX8357_fillScreen(uint16_t color) {
    Adafruit_HX8357_fillRect(0, 0, Adafruit_HX8357_width(), Adafruit_HX8357_height(), color);
}

void Adafruit_HX8357_drawPixel(int16_t x, int16_t y, uint16_t color) {
    if ((x < 0) || (x >= Adafruit_HX8357_width()) || (y < 0) || (y >= Adafruit_HX8357_height()))
        return;
    Adafruit_HX8357_setAddrWindow(x, y, 1, 1);
    Adafruit_HX8357_DC_HIGH();
    Adafruit_HX8357_CS_LOW();
    uint8_t cmd = HX8357_RAMWR;
    HAL_SPI_Transmit(&hspi1, &cmd, 1, 100);
    Adafruit_HX8357_DC_LOW();
    uint8_t hi = color >> 8, lo = color;
    HAL_SPI_Transmit(&hspi1, &hi, 1, 100);
    HAL_SPI_Transmit(&hspi1, &lo, 1, 100);
    Adafruit_HX8357_CS_HIGH();
}

void Adafruit_HX8357_drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
    Adafruit_HX8357_fillRect(x, y, 1, h, color);
}

void Adafruit_HX8357_drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
    Adafruit_HX8357_fillRect(x, y, w, 1, color);
}

void Adafruit_HX8357_drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    int16_t steep = abs(y1 - y0) > abs(x1 - x0);
    if (steep) {
        int16_t temp = x0;
        x0 = y0;
        y0 = temp;
        temp = x1;
        x1 = y1;
        y1 = temp;
    }
    if (x0 > x1) {
        int16_t temp = x0;
        x0 = x1;
        x1 = temp;
        temp = y0;
        y0 = y1;
        y1 = temp;
    }
    int16_t dx, dy;
    dx = x1 - x0;
    dy = abs(y1 - y0);
    int16_t err = dx / 2;
    int16_t ystep;
    if (y0 < y1) {
        ystep = 1;
    } else {
        ystep = -1;
    }
    for (; x0 <= x1; x0++) {
        if (steep) {
            Adafruit_HX8357_drawPixel(y0, x0, color);
        } else {
            Adafruit_HX8357_drawPixel(x0, y0, color);
        }
        err -= dy;
        if (err < 0) {
            y0 += ystep;
            err += dx;
        }
    }
}

void Adafruit_HX8357_drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    Adafruit_HX8357_drawFastHLine(x, y, w, color);
    Adafruit_HX8357_drawFastHLine(x, y + h - 1, w, color);
    Adafruit_HX8357_drawFastVLine(x, y, h, color);
    Adafruit_HX8357_drawFastVLine(x + w - 1, y, h, color);
}

void Adafruit_HX8357_fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
    Adafruit_HX8357_drawFastVLine(x0, y0 - r, 2 * r + 1, color);
    Adafruit_HX8357_fillCircleHelper(x0, y0, r, 3, 0, color);
}

void Adafruit_HX8357_fillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, int16_t delta, uint16_t color) {
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;
    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;
        if (cornername & 0x1) {
            Adafruit_HX8357_drawFastVLine(x0 + x, y0 - y, 2 * y + 1 + delta, color);
            Adafruit_HX8357_drawFastVLine(x0 + y, y0 - x, 2 * x + 1 + delta, color);
        }
        if (cornername & 0x2) {
            Adafruit_HX8357_drawFastVLine(x0 - x, y0 - y, 2 * y + 1 + delta, color);
            Adafruit_HX8357_drawFastVLine(x0 - y, y0 - x, 2 * x + 1 + delta, color);
        }
    }
}



