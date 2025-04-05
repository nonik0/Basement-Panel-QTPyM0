#pragma once

#include <Arduino.h>

using namespace std;

struct Coordinate
{
  int x;
  int y;
};

// negation
Coordinate operator-(const Coordinate &coord)
{
  return {-coord.x, -coord.y};
}

Coordinate operator+(const Coordinate &lhs, const Coordinate &rhs)
{
  return {lhs.x + rhs.x, lhs.y + rhs.y};
}

Coordinate operator-(const Coordinate &lhs, const Coordinate &rhs)
{
  return {lhs.x - rhs.x, lhs.y - rhs.y};
}

Coordinate operator*(const Coordinate &lhs, const Coordinate &rhs)
{
  return {lhs.x * rhs.x, lhs.y * rhs.y};
}

Coordinate operator*(const int &lhs, const Coordinate &rhs)
{
  return {rhs.x * lhs, rhs.y * lhs};
}

bool operator==(const Coordinate &lhs, const Coordinate &rhs)
{
  return lhs.x == rhs.x && lhs.y == rhs.y;
}

bool operator!=(const Coordinate &lhs, const Coordinate &rhs)
{
  return !(lhs == rhs);
}

Coordinate operator+=(Coordinate &lhs, const Coordinate &rhs)
{
  lhs.x += rhs.x;
  lhs.y += rhs.y;
  return lhs;
}

Coordinate operator-=(Coordinate &lhs, const Coordinate &rhs)
{
  lhs.x -= rhs.x;
  lhs.y -= rhs.y;
  return lhs;
}

Coordinate operator*=(Coordinate &lhs, const Coordinate &rhs)
{
  lhs.x *= rhs.x;
  lhs.y *= rhs.y;
  return lhs;
}

namespace std
{
  template <>
  struct hash<Coordinate>
  {
    size_t operator()(const Coordinate &loc) const
    {
      return hash<int>()(loc.x) ^ hash<int>()(loc.y);
    }
  };
}

using Location = Coordinate;
using Direction = Coordinate; // technically should enforce to length 1 vectors

const Location NullLocation = Location{-1, -1};

const Direction Left = {-1, 0};
const Direction Right = {1, 0};
const Direction Up = {0, -1};
const Direction Down = {0, 1};
const Direction Directions[] = {Left, Right, Up, Down};

void shuffle(Direction *list, int size)
{
  for (int i = 0; i < size; i++)
  {
    int index = random(size);
    Direction temp = list[i];
    list[i] = list[index];
    list[index] = temp;
  }
}

bool upOrDown(Direction dir)
{
  return dir == Up || dir == Down;
}

bool leftOrRight(Direction dir)
{
  return dir == Left || dir == Right;
}