#include <Arduino.h>
#include <cq_node.h>

#define PIN_ACTION D7

CqNode node(true);

void setup()
{
  pinMode(PIN_ACTION, INPUT_PULLUP);

  if (digitalRead(PIN_ACTION) == LOW)
  {
    node.ClearConfiguration();
  }

  node.Begin();
}

void loop()
{
}