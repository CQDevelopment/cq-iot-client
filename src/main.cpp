#include <Arduino.h>
#include <cq_iot_client.h>

#define PIN_ACTION D7

const uint8_t switchPins[] = {D1, D2, D5};

CqIotClient client(true);

void switchGet(uint8_t index)
{
  bool state = digitalRead(switchPins[index]) == HIGH ? true : false;

  client.SendSwitchState(index, state);
}

void switchSet(uint8_t index, uint8_t state)
{
  digitalWrite(switchPins[index], state == 1 ? HIGH : LOW);

  switchGet(index);
}

void setup()
{
  pinMode(PIN_ACTION, INPUT_PULLUP);

  for (uint8_t i = 0; i < sizeof(switchPins); i++)
  {
    pinMode(switchPins[i], OUTPUT);
  }

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