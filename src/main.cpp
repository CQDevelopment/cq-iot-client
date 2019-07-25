#include <Arduino.h>
#include <cq_iot_client.h>

#define PIN_ACTION D1
#define PIN_PIR D5

const uint8_t switchPins[] = {D6};

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

  pinMode(PIN_PIR, INPUT);

  client.SwitchCount = sizeof(switchPins);
  client.SwitchGetFunction = &switchGet;
  client.SwitchSetFunction = &switchSet;

  client.PushCount = 1;

  if (digitalRead(PIN_ACTION) == LOW)
  {
    client.ClearConfiguration();
  }

  client.Begin();
}

int lastPirState = LOW;

void loop()
{
  int pirState = digitalRead(PIN_PIR);

  if (pirState == HIGH)
  {
    if (lastPirState == LOW)
    {
      client.SendPush(0, "Alarm");
    }
  }

  lastPirState = pirState;

  client.Loop();
}