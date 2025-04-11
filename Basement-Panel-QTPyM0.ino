#include <Adafruit_NeoPixel.h>
#include <Adafruit_TinyUSB.h>
#include <Arduino.h>

#include "tunnel_runner.h"

#define LED_PIN A3
#define BUTTON_PIN A2
#define LED_COUNT 1

const uint32_t Amber = Adafruit_NeoPixel::Color(0xFF, 0x77, 0x00);

void (*reset)(void) = 0;
bool buttonState = true;
bool rgbLedState = true;
Adafruit_NeoPixel rgbLed(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
TunnelRunnerTaskHandler tunnelRunner;

void setup()
{
  Serial.begin(115200);
  Serial.println("Starting setup...");

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  rgbLed.begin();
  rgbLed.setBrightness(5);
  rgbLed.setPixelColor(0, Amber);
  rgbLed.show();

  while (!tunnelRunner.setup())
  {
    Serial.println("Failed to setup TunnelRunnerTaskHandler");
    delay(5000);
  }

  Serial.println("TunnelRunnerTaskHandler setup complete");
  Serial.println("Setup complete");
}

void loop()
{
  tunnelRunner.tick();

  if (digitalRead(BUTTON_PIN) != buttonState)
  {
    buttonState = !buttonState;

    if (!buttonState)
    {
      rgbLedState = !rgbLedState;
      rgbLed.setPixelColor(0, rgbLedState ? Amber : 0);
      rgbLed.show();
    }
  }
}
