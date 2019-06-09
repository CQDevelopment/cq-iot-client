#include <Arduino.h>
#include <cq_iot_client.h>

#define PIN_ACTION D7

const uint8_t switchPins[] = {D1, D2, D5};

CqIotClient client(true);

void switchGet(uint8_t index)
{
  bool result = digitalRead(switchPins[index]) == HIGH ? true : false;

  // send here!
}

void switchSet(uint8_t index, bool state)
{
  digitalWrite(switchPins[index], state ? HIGH : LOW);

  switchGet(index);
}

void setup()
{
  pinMode(PIN_ACTION, INPUT_PULLUP);

  client.SwitchCount = 3;
  client.SwitchGetFunction = &switchGet;
  client.SwitchSetFunction = &switchSet;

  if (digitalRead(PIN_ACTION) == LOW)
  {
    client.ClearConfiguration();
  }

  client.Begin();
}

void loop()
{
}