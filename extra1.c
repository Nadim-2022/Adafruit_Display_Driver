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
