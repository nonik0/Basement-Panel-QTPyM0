#include <Adafruit_NeoPixel.h>
#include <Adafruit_TinyUSB.h> 
#include <Arduino.h>

#include "tunnel_runner.h"

#define LED_PIN A3
#define BUTTON_PIN A2
#define LED_COUNT 1

void (*reset)(void) = 0;
Adafruit_NeoPixel rgbLed(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
TunnelRunnerTaskHandler tunnelRunner;

void setup()
{
  Serial.begin(115200);
  Serial.println("Starting setup...");

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  rgbLed.begin();
  rgbLed.setBrightness(5);
  rgbLed.setPixelColor(0, rgbLed.Color(0xFF, 0x77, 0x00));
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

  bool buttonState = digitalRead(BUTTON_PIN);
  if (buttonState == LOW)
  {
    reset();
  }
}
