#include <Arduino.h>
#include <cq_node.h>

#define PIN_ACTION D7

const uint8_t switchPins[] = {D1, D2, D5};

CqNode node(true);

void switchPin(uint8_t index, bool state)
{
  digitalWrite(switchPins[index], state ? HIGH : LOW);
}

void setup()
{
  pinMode(PIN_ACTION, INPUT_PULLUP);

  node.SwitchPinCount = 3;
  node.SwitchPinFunction = &switchPin;

  if (digitalRead(PIN_ACTION) == LOW)
  {
    node.ClearConfiguration();
  }

  node.Begin();
}

void loop()
{
}