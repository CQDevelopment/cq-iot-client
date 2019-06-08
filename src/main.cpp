#include <Arduino.h>
#include <cq_node.h>

#define PIN_ACTION D7

const uint8_t switchPins[] = {D1, D2, D5};

CqNode node(true);

void switchGet(uint8_t index)
{
  bool result = digitalRead(switchPins[index]) == HIGH ? true : false;

  // send here!
}

void switchSet(uint8_t index, bool state)
{
  digitalWrite(switchPins[index], state ? HIGH : LOW);
}

void setup()
{
  pinMode(PIN_ACTION, INPUT_PULLUP);

  node.SwitchCount = 3;
  node.SwitchGetFunction = &switchGet;
  node.SwitchSetFunction = &switchSet;

  if (digitalRead(PIN_ACTION) == LOW)
  {
    node.ClearConfiguration();
  }

  node.Begin();
}

void loop()
{
}