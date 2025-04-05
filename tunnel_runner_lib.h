#pragma once

#include <Arduino.h>
#include <functional>

#include "coordinate.h"

class TunnelRunner
{
private:
    const int TunnelSpeed = 1;
    const int RunnerSpeed = 2;
    const int CrashDelay = 30;
    const int ErrorDelay = 100;
    const int TunnelMinWidth = 3;
    const int TunnelDifficultyCooldown = 200;

    // constant
    int _width;
    int _height;
    int _tunnelVisibleLength;
    int _tunnelMaxWidth;
    Direction _tunnelDirection;
    bool _tunnelIsVertical;

    bool **_tunnelWalls;
    Location _tunnelLocation; // furthest from runner side, left or top empty space
    int _tunnelWidth;
    int _tunnelCooldown;
    bool _tunnelShrinking;
    int _tunnelDifficultyCooldown;

    Location _runnerLocation;
    int _runnerCooldown;
    int _resetDelay;

    uint32_t _pathColor;
    uint32_t _wallColor;
    uint32_t _runnerColor;

    // function callback to draw pixels
    std::function<void(int, int, uint32_t)> _drawPixel;
    // std::function<void(uint32_t)> _setStatus;

public:
    TunnelRunner(
        int width, int height,
        Direction tunnelDirection,
        uint32_t pathColor, uint32_t wallColor, uint32_t runnerColor,
        std::function<void(int, int, uint32_t)> drawPixel);

    void init();
    bool update(); // returns true if any pixel changed

private:
    bool advanceTunnel();
    bool moveRunner();
    void drawTunnel();

    bool isInBounds(int x, int y);
    bool isInBounds(Location loc);
    bool isWall(int x, int y);
    bool isWall(Location loc);
};

TunnelRunner::TunnelRunner(
    int width, int height,
    Direction tunnelDirection,
    uint32_t pathColor, uint32_t wallColor, uint32_t runnerColor,
    std::function<void(int, int, uint32_t)> drawPixel)
{
    _width = width;
    _height = height;
    _tunnelDirection = tunnelDirection;
    _tunnelIsVertical = tunnelDirection.x == 0;
    _tunnelMaxWidth = _tunnelIsVertical ? _width : _height;
    _tunnelVisibleLength = _tunnelIsVertical ? _height : _width;
    _pathColor = pathColor;
    _wallColor = wallColor;
    _runnerColor = runnerColor;
    _drawPixel = drawPixel;

    _tunnelWalls = new bool *[_width];
    for (int i = 0; i < _height; i++)
    {
        _tunnelWalls[i] = new bool[_height];
    }

    Serial.println("width:" + String(_width));
    Serial.println("height" + String(_height));
    // Serial.println("tunnelDirection:" + _tunnelDirection.x + "," + _tunnelDirection.y);
    Serial.println("tunnelIsVertical" + String(_tunnelIsVertical));
    Serial.println("tunnelMaxWidth" + String(_tunnelMaxWidth));
    Serial.println("tunnelVisibleLength" + String(_tunnelVisibleLength));
}

void TunnelRunner::init()
{
    Serial.println("Initializing tunnel");

    _resetDelay = -1;
    _runnerCooldown = 0;
    _tunnelCooldown = 0;
    _tunnelShrinking = true;

    // fill tunnel with walls (true)
    for (int x = 0; x < _width; x++)
    {
        for (int y = 0; y < _height; y++)
        {
            _tunnelWalls[x][y] = true;
        }
    }

    _tunnelWidth = _tunnelMaxWidth - 2;

    if (_tunnelDirection == Left)
    {
        _runnerLocation = {1, _height / 2}; // runner starts at middle-left
        _tunnelLocation = {0, 1};           // tunnel moves to left, so start at top-left
    }
    else if (_tunnelDirection == Right)
    {
        _runnerLocation = {_width - 2, _height / 2}; // runner starts at middle-right
        _tunnelLocation = {_width - 1, 1};           // tunnel moves to right, so start at top-right
    }
    else if (_tunnelDirection == Up)
    {
        _runnerLocation = {_width / 2, 1}; // runner starts at middle-top
        _tunnelLocation = {1, 0};          // tunnel moves up, so start at top-left
    }
    else if (_tunnelDirection == Down)
    {
        _runnerLocation = {_width / 2, _height - 2}; // runner starts at middle-bottom
        _tunnelLocation = {1, _height - 1};          // tunnel moves down, so start at bottom-left
    }

    // clear straight path to far side
    for (int i = 0; i < _tunnelVisibleLength; i++)
    {
        for (int j = 0; j < _tunnelWidth; j++)
        {
            if (_tunnelIsVertical)
            {
                _tunnelWalls[_tunnelLocation.x + j][_tunnelLocation.y] = false;
            }
            else
            {
                _tunnelWalls[_tunnelLocation.x][_tunnelLocation.y + j] = false;
            }
        }

        _tunnelLocation -= _tunnelDirection; // moving in opposite direction
    }

    _tunnelLocation += _tunnelDirection; // move back to edge
    Serial.print("Runner location: (");
    Serial.print(_runnerLocation.x);
    Serial.print(",");
    Serial.print(_runnerLocation.y);
    Serial.println(")");
    Serial.print("Tunnel location: (");
    Serial.print(_tunnelLocation.x);
    Serial.print(",");
    Serial.print(_tunnelLocation.y);
    Serial.println(")");
}

bool TunnelRunner::update()
{
    // try
    // {
    if (_resetDelay > 0)
    {
        _resetDelay--;
        if (_resetDelay <= 0)
        {
            init();
            drawTunnel();
            return true;
        }
        return false;
    }

    bool update = false;

    update |= moveRunner();
    update |= advanceTunnel();

    if (isWall(_runnerLocation))
    {
        Serial.println("Runner crashed");
        drawTunnel();
        _resetDelay = CrashDelay;
        return true;
    }

    if (update)
    {
        drawTunnel();
    }

    return update;
    // }
    // catch (const std::exception &e)
    // {
    //     Serial.println("Error in tunnel runner update: " + e.what());
    //     Serial.flush();
    //     delay(5000);
    //     _resetDelay = ErrorDelay;
    //     return false;
    // }
}

bool TunnelRunner::advanceTunnel()
{
    if (_tunnelCooldown-- > 0)
    {
        return false;
    }
    _tunnelCooldown = TunnelSpeed;

    // get location at left of vertical tunnel or top of horiz tunnel at near/runner side
    Location startLoc, endLoc;
    if (_tunnelDirection == Left)
    {
        startLoc = {0, 0}; // tunnel moves to left, so start at top-left
        endLoc = {_width - 1, 0};
    }
    else if (_tunnelDirection == Right)
    {
        startLoc = {_width - 1, 0}; // tunnel moves to right, so start at top-right
        endLoc = {0, 0};
    }
    else if (_tunnelDirection == Up)
    {
        startLoc = {0, 0}; // tunnel moves up, so start at top-left
        endLoc = {0, _height - 1};
    }
    else if (_tunnelDirection == Down)
    {
        startLoc = {0, _height - 1}; // tunnel moves down, so start at bottom-left
        endLoc = {0, 0};
    }

    // move all walls in direction of tunnel
    for (Location curLoc = startLoc; curLoc != endLoc; curLoc -= _tunnelDirection)
    {
        for (int j = 0; j < _tunnelMaxWidth; j++)
        {
            // copy wall from opposite direction of tunnel movement
            if (_tunnelIsVertical)
            {
                _tunnelWalls[curLoc.x + j][curLoc.y] = _tunnelWalls[curLoc.x + j][curLoc.y - _tunnelDirection.y];
            }
            else
            {
                _tunnelWalls[curLoc.x][curLoc.y + j] = _tunnelWalls[curLoc.x - _tunnelDirection.x][curLoc.y + j];
            }
        }
    }

    if (_tunnelDifficultyCooldown-- < 0)
    {
        _tunnelDifficultyCooldown = TunnelDifficultyCooldown;

        _tunnelWidth += _tunnelShrinking ? -1 : 1;
        Serial.println("Changed tunnel width to " + String(_tunnelWidth));

        if (_tunnelWidth < TunnelMinWidth)
        {
            _tunnelWidth = TunnelMinWidth + 1;
            _tunnelShrinking = false;
            Serial.println("Tunnel now expanding");
        }
        else if (_tunnelWidth > _tunnelMaxWidth)
        {
            _tunnelWidth = _tunnelMaxWidth - 1;
            _tunnelShrinking = true;
            Serial.println("Tunnel now shrinking");
        }

        // when tunnel shrinks decide which direction it shifts randomly
        if (random() % 2 == 0)
        {
            _tunnelLocation = _tunnelIsVertical
                                  ? _tunnelLocation + Right
                                  : _tunnelLocation + Down;
        }
    }
    // randomly change tunnel location (shift center of tunnel)
    else if (random() % 10 >= 5)
    {
        Location tunnelOppositeLoc;
        Direction tunnelShift;
        if (_tunnelIsVertical)
        {
            tunnelOppositeLoc = _tunnelLocation + (_tunnelWidth - 1) * Right;
            if (_tunnelLocation.x <= 0)
                tunnelShift = Right;
            else if (_tunnelLocation.x + _tunnelWidth >= _width)
                tunnelShift = Left;
            else
                tunnelShift = (random() % 2 == 0 ? Left : Right);
        }
        else
        {
            tunnelOppositeLoc = _tunnelLocation + (_tunnelWidth - 1) * Down;
            if (_tunnelLocation.y <= 0)
                tunnelShift = Down;
            else if (_tunnelLocation.y + _tunnelWidth >= _height)
                tunnelShift = Up;
            else
                tunnelShift = (random() % 2 == 0 ? Up : Down);
        }

        Location newLoc = _tunnelLocation + tunnelShift;
        if (isInBounds(newLoc) && isInBounds(tunnelOppositeLoc + tunnelShift))
        {
            _tunnelLocation = newLoc;
        }
    }

    // now generate new walls for tunnel at far end
    for (int i = 0; i < _tunnelMaxWidth; i++)
    {
        if (_tunnelIsVertical)
        {
            _tunnelWalls[i][_tunnelLocation.y] = i < _tunnelLocation.x || i >= _tunnelLocation.x + _tunnelWidth;
        }
        else
        {
            _tunnelWalls[_tunnelLocation.x][i] = i < _tunnelLocation.y || i >= _tunnelLocation.y + _tunnelWidth;
        }
    }

    _tunnelCooldown = TunnelSpeed;
    return true;
}

bool TunnelRunner::moveRunner()
{
    bool update = false;

    // move runner
    if (_runnerCooldown > 0)
    {
        _runnerCooldown--;
        return false;
    }

    // runner always tries to center self in tunnel

    // get average of tunnel width for next 3 sections ahead of runner
    int totalWidth = 0;
    int count = 0;
    for (int i = 0; i < 3; i++)
    {
        Location lookAheadLoc = _runnerLocation - (i + 1) * _tunnelDirection;

        for (int j = 0; j < _tunnelMaxWidth; j++)
        {
            if (_tunnelIsVertical)
            {
                if (!isWall(j, lookAheadLoc.y))
                {
                    totalWidth += j;
                    count++;
                }
            }
            else
            {
                if (!isWall(lookAheadLoc.x, j))
                {
                    totalWidth += j;
                    count++;
                }
            }
        }
    }

    int lookAheadMiddleLoc = round((float)totalWidth / (float)count);
    int distToMiddle = lookAheadMiddleLoc - _runnerLocation.x;
    if (distToMiddle != 0)
    {
        Location newLoc = _runnerLocation + (_tunnelIsVertical
                                                 ? (distToMiddle > 0 ? Right : Left)
                                                 : (distToMiddle > 0 ? Down : Up));

        if (isInBounds(newLoc) && !isWall(newLoc))
        {
            // only reset cooldown if runner actually moved
            _runnerCooldown = RunnerSpeed;
            _runnerLocation = newLoc;
            update = true;
        }
    }

    return true;
}

void TunnelRunner::drawTunnel()
{
    for (int x = 0; x < _width; x++)
    {
        for (int y = 0; y < _height; y++)
        {
            _drawPixel(x, y, _tunnelWalls[x][y] ? _wallColor : _pathColor);
        }
    }

    _drawPixel(_runnerLocation.x, _runnerLocation.y, _runnerColor);
}

inline bool TunnelRunner::isInBounds(int x, int y)
{
    return x >= 0 && x < _width && y >= 0 && y < _height;
}

inline bool TunnelRunner::isInBounds(Location loc)
{
    return isInBounds(loc.x, loc.y);
}

inline bool TunnelRunner::isWall(int x, int y)
{
    return _tunnelWalls[x][y];
}

inline bool TunnelRunner::isWall(Location loc)
{
    return isWall(loc.x, loc.y);
}