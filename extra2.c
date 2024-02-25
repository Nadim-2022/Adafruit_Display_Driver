void Adafruit_HX8357_CS_HIGH(void) {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET); // HIGH
}



void Adafruit_HX8357_writecommand(uint8_t c) {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET); // LOW
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET); // LOW
    HAL_SPI_Transmit(&hspi1, &c, 1, 100);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET); // HIGH
}

void Adafruit_HX8357_writedata(uint8_t c) {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET); // HIGH
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET); // LOW
    HAL_SPI_Transmit(&hspi1, &c, 1, 100);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET); // HIGH
}

void Adafruit_HX8357_commandList(const uint8_t *addr) {
    uint8_t  numCommands, numArgs;
    uint16_t ms;

    numCommands = *addr++;   // Number of commands to follow
    while (numCommands--) {                 // For each command...
        Adafruit_HX8357_writecommand(*addr++); //   Read, issue command
        numArgs  = *addr++;       //   Number of args to follow
        ms       = numArgs & DELAY; //   If high bit set, delay follows args
        numArgs &= ~DELAY;        //   Mask out delay bit
        while (numArgs--) {                 //   For each argument...
            Adafruit_HX8357_writedata(*addr++);  //     Read, issue argument
        }

        if (ms) {
            ms = *addr++; // Read post-command delay time (ms)
            if (ms == 255) ms = 500; // If 255, delay for 500 ms
            HAL_Delay(ms);
        }
    }
}

void Adafruit_HX8357_begin(uint8_t type) {
    if (type == HX8357D) {
        Adafruit_HX8357_commandList(initd);
    } else {
        Adafruit_HX8357_commandList(initb);
    }
}

void Adafruit_HX8357_setAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    uint32_t xa = ((uint32_t)x << 16) | (x + w - 1);
    uint32_t ya = ((uint32_t)y << 16) | (y + h - 1);
    Adafruit_HX8357_writecommand(HX8357_CASET); // Column addr set
    Adafruit_HX8357_writedata(xa >> 24);
    Adafruit_HX8357_writedata(xa >> 16);
    Adafruit_HX8357_writedata(xa >> 8);
    Adafruit_HX8357_writedata(xa);
    Adafruit_HX8357_writecommand(HX8357_PASET); // Row addr set
    Adafruit_HX8357_writedata(ya >> 24);
    Adafruit_HX8357_writedata(ya >> 16);
    Adafruit_HX8357_writedata(ya >> 8);
    Adafruit_HX8357_writedata(ya);
    Adafruit_HX8357_writecommand(HX8357_RAMWR); // write to RAM
}

void Adafruit_HX8357_pushColor(uint16_t color) {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET); // HIGH
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET); // LOW
    HAL_SPI_Transmit(&hspi1, (uint8_t *) &color, 2, 100);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET); // HIGH
}

void Adafruit_HX8357_fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    Adafruit_HX8357_setAddrWindow(x, y, w, h);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET); // HIGH
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET); // LOW
    for (y = h; y > 0; y--) {
        for (x = w; x > 0; x--) {
            HAL_SPI_Transmit(&hspi1, (uint8_t *) &color, 2, 100);
        }
    }
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET); // HIGH
}

void Adafruit_HX8357_fillScreen(uint16_t color) {
    Adafruit_HX8357_fillRect(0, 0, HX8357_TFTWIDTH, HX8357_TFTHEIGHT, color);
}



void Adafruit_HX8357_drawPixel(int16_t x, int16_t y, uint16_t color) {
    if ((x < 0) || (x >= HX8357_TFTWIDTH) || (y < 0) || (y >= HX8357_TFTHEIGHT))
        return;
    Adafruit_HX8357_setAddrWindow(x, y, 1, 1);
    Adafruit_HX8357_pushColor(color);
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
        int16_t temp;
        temp = x0;
        x0 = y0;
        y0 = temp;
        temp = x1;
        x1 = y1;
        y1 = temp;
    }
    if (x0 > x1) {
        int16_t temp;
        temp = x0;
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

void Adafruit_HX8357_fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
    Adafruit_HX8357_drawFastVLine(x0, y0 - r, 2 * r + 1, color);
    Adafruit_HX8357_fillCircleHelper(x0, y0, r, 3, 0, color);
}

void Adafruit_HX8357_drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;
    Adafruit_HX8357_drawPixel(x0, y0 + r, color);
    Adafruit_HX8357_drawPixel(x0, y0 - r, color);
    Adafruit_HX8357_drawPixel(x0 + r, y0, color);
    Adafruit_HX8357_drawPixel(x0 - r, y0, color);
    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;
        Adafruit_HX8357_drawPixel(x0 + x, y0 + y, color);
        Adafruit_HX8357_drawPixel(x0 - x, y0 + y, color);
        Adafruit_HX8357_drawPixel(x0 + x, y0 - y, color);
        Adafruit_HX8357_drawPixel(x0 - x, y0 - y, color);
        Adafruit_HX8357_drawPixel(x0 + y, y0 + x, color);
        Adafruit_HX8357_drawPixel(x0 - y, y0 + x, color);
        Adafruit_HX8357_drawPixel(x0 + y, y0 - x, color);
        Adafruit_HX8357_drawPixel(x0 - y, y0 - x, color);
    }
}


void Adafruit_HX8357_drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
    Adafruit_HX8357_drawLine(x0, y0, x1, y1, color);
    Adafruit_HX8357_drawLine(x1, y1, x2, y2, color);
    Adafruit_HX8357_drawLine(x2, y2, x0, y0, color);
}

void AAdafruit_HX8357_drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size) {
    if ((x >= HX8357_TFTWIDTH) || // Clip right
        (y >= HX8357_TFTHEIGHT) || // Clip bottom
        ((x + 6 * size - 1) < 0) || // Clip left
        ((y + 8 * size - 1) < 0))   // Clip top
        return;
    for (int8_t i = 0; i < 6; i++) {
        uint8_t line;
        if (i == 5)
            line = 0x0;
        else
            line = font[c * 5 + i];
        for (int8_t j = 0; j < 8; j++) {
            if (line & 0x1) {
                if (size == 1) // default size
                    Adafruit_HX8357_drawPixel(x + i, y + j, color);
                else {  // big size
                    Adafruit_HX8357_fillRect(x + (i * size), y + (j * size), size, size, color);
                }
            } else if (bg != color) {
                if (size == 1) // default size
                    Adafruit_HX8357_drawPixel(x + i, y + j, bg);
                else {  // big size
                    Adafruit_HX8357_fillRect(x + i * size, y + j * size, size, size, bg);
                }
            }
            line >>= 1;
        }
    }
}

void Adafruit_HX8357_drawString(int16_t x, int16_t y, char *c, uint16_t color, uint16_t bg, uint8_t size) {
    while (*c) {
        Adafruit_HX8357_drawChar(x, y, *c, color, bg, size);
        x += 6 * size;
        c++;
    }
}

void Adafruit_HX8357_setRotation(uint8_t m) {
    Adafruit_HX8357_writecommand(HX8357_MADCTL);
    rotation = m % 4; // can't be higher than 3
    switch (rotation) {
        case 0:
            Adafruit_HX8357_writedata(MADCTL_MX | MADCTL_MY | MADCTL_RGB);
            _width = HX8357_TFTWIDTH;
            _height = HX8357_TFTHEIGHT;
            break;
        case 1:
            Adafruit_HX8357_writedata(MADCTL_MV | MADCTL_MY | MADCTL_RGB);
            _width = HX8357_TFTHEIGHT;
            _height = HX8357_TFTWIDTH;
            break;
        case 2:
            Adafruit_HX8357_writedata(MADCTL_RGB);
            _width = HX8357_TFTWIDTH;
            _height = HX8357_TFTHEIGHT;
            break;
        case 3:
            Adafruit_HX8357_writedata(MADCTL_MX | MADCTL_MV | MADCTL_RGB);
            _width = HX8357_TFTHEIGHT;
            _height = HX8357_TFTWIDTH;
            break;
    }
}

void Adafruit_HX8357_invertDisplay(uint8_t i) {
    Adafruit_HX8357_writecommand(i ? HX8357_INVON : HX8357_INVOFF);
}

void Adafruit_HX8357_displayOn(void) {
    Adafruit_HX8357_writecommand(HX8357_DISPON);
}

void Adafruit_HX8357_displayOff(void) {
    Adafruit_HX8357_writecommand(HX8357_DISPOFF);
}

void Adafruit_HX8357_sleepIn(void) {
    Adafruit_HX8357_writecommand(HX8357_SLPIN);
}

void Adafruit_HX8357_sleepOut(void) {
    Adafruit_HX8357_writecommand(HX8357_SLPOUT);
}

void Adafruit_HX8357_setBacklight(uint8_t data) {
    // no back light control
}

