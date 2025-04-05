#pragma once

#include <Adafruit_LEDBackpack.h>

#include "tunnel_runner_lib.h"

class TunnelRunnerTaskHandler
{
private:
    static const uint8_t I2C_ADDR_1 = 0x72;
    static const uint8_t I2C_ADDR_2 = 0x73;
    static const uint8_t I2C_ADDR_3 = 0x74;
    static const uint8_t TASK_PRIORITY = 5;
    static const uint8_t GAP = 4;
    static const uint8_t HEIGHT = 16;   
    const int TUNNEL_DELAY_MS = 20;

    Adafruit_8x16minimatrix _matrix1 = Adafruit_8x16minimatrix();
    Adafruit_8x16minimatrix _matrix2 = Adafruit_8x16minimatrix();
    Adafruit_8x16minimatrix _matrix3 = Adafruit_8x16minimatrix();
    TunnelRunner *_tunnelRunner;

    bool _display = true;
    unsigned long _lastUpdate = 0;

public:
    TunnelRunnerTaskHandler() {}

    bool setup();
    void tick();

private:
    void drawPixel(int x, int y, uint32_t c);
};

bool TunnelRunnerTaskHandler::setup()
{
    Serial.println("Starting TunnelRunner8x48 setup");

    if (!_matrix1.begin(I2C_ADDR_1) || !_matrix2.begin(I2C_ADDR_2) || !_matrix3.begin(I2C_ADDR_3))
    {
        //log_e("Matrix 1, 2, or 3 not found");
        return false;
    }

    _matrix1.setRotation(2);
    _matrix2.setRotation(2);
    _matrix3.setRotation(2);
    _matrix1.setBrightness(1);
    _matrix2.setBrightness(1);
    _matrix3.setBrightness(1);

    _tunnelRunner = new TunnelRunner(
        _matrix1.width(),
        _matrix1.height() * 3 + GAP * 2,
        Up,
        LED_OFF, // path
        LED_ON,  // wall
        LED_ON,  // runner
        [this](int x, int y, uint32_t c) { drawPixel(x, y, c); });

    _tunnelRunner->init();

    Serial.println("TunnelRunner8x16 setup complete");
    return true;
}

void TunnelRunnerTaskHandler::tick()
{
    if (millis() - _lastUpdate < TUNNEL_DELAY_MS)
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

    if (_tunnelRunner->update())
    {
        _matrix1.writeDisplay();
        _matrix2.writeDisplay();
        _matrix3.writeDisplay();
    }
}

void TunnelRunnerTaskHandler::drawPixel(int x, int y, uint32_t c)
{
    if (y >= 0 && y < HEIGHT)
    {
        _matrix1.drawPixel(x, y, c);
    }
    else if (y >= HEIGHT + GAP && y < HEIGHT * 2 + GAP)
    {
        _matrix2.drawPixel(x, y - HEIGHT - GAP, c);
    }
    else if (y >= HEIGHT * 2 + GAP * 2 && y < HEIGHT * 3 + GAP * 2)
    {
        _matrix3.drawPixel(x, y - HEIGHT * 2 - GAP * 2, c);
    }
}
