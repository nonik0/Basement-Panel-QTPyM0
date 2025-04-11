#pragma once

#include <Adafruit_LEDBackpack.h>

class SpiralTaskHandler
{
private:
    static const uint8_t I2C_ADDR_1 = 0x71;
    static const uint8_t I2C_ADDR_2 = 0x73;
    static const uint8_t I2C_ADDR_3 = 0x72;
    static const uint8_t TASK_PRIORITY = 5;
    static const uint16_t HEIGHT = 8;
    static const uint16_t WIDTH = 8;
    static const uint16_t MATRIX_COUNT = 3;
    static const uint16_t ROW_LENGTH = WIDTH * MATRIX_COUNT;
    static const uint16_t PIXEL_COUNT = HEIGHT * WIDTH * MATRIX_COUNT;
    static const int SPIRAL_DELAY_MS = 20;

    Adafruit_8x8matrix _matrix1 = Adafruit_8x8matrix();
    Adafruit_8x8matrix _matrix2 = Adafruit_8x8matrix();
    Adafruit_8x8matrix _matrix3 = Adafruit_8x8matrix();
    Adafruit_8x8matrix *_matrices[MATRIX_COUNT] = {&_matrix1, &_matrix2, &_matrix3};

    bool _display = true;
    unsigned long _lastUpdate = 0;
    uint16_t _pixelIndex = 0; // used to derive matrix index, col, and row
    uint16_t _matrixIndex = 0;
    uint16_t _matrixCol = 0;
    uint16_t _matrixRow = 0;

public:
    SpiralTaskHandler() {}

    bool setup();
    void tick();
};

bool SpiralTaskHandler::setup()
{
    Serial.println("Starting Spiral setup");

    if (!_matrix1.begin(I2C_ADDR_1) || !_matrix2.begin(I2C_ADDR_2) || !_matrix3.begin(I2C_ADDR_3))
    {
        // log_e("Matrix 1, 2, or 3 not found");
        return false;
    }

    _matrix1.setBrightness(1);
    _matrix2.setBrightness(1);
    _matrix3.setBrightness(1);

    Serial.println("Spiral8 setup complete");
    return true;
}

void SpiralTaskHandler::tick()
{
    if (millis() - _lastUpdate < SPIRAL_DELAY_MS)
    {
        return;
    }
    _lastUpdate = millis();

    if (!_display)
    {
        _matrix1.clear();
        _matrix1.writeDisplay();
        _matrix2.clear();
        _matrix2.writeDisplay();
        _matrix3.clear();
        _matrix3.writeDisplay();
        return;
    }

    _matrices[_matrixIndex]->drawPixel(_matrixCol, _matrixRow, LED_OFF);
    _matrices[_matrixIndex]->writeDisplay();

    _pixelIndex = (_pixelIndex + 1) % PIXEL_COUNT;

    uint16_t colRaw = _pixelIndex % ROW_LENGTH;
    _matrixCol = colRaw % WIDTH;
    _matrixRow = _pixelIndex / ROW_LENGTH;
    _matrixIndex = colRaw / WIDTH;

    _matrices[_matrixIndex]->drawPixel(_matrixCol, _matrixRow, LED_ON);
    _matrices[_matrixIndex]->writeDisplay();

}
