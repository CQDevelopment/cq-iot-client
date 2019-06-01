#include <Arduino.h>
#include <cq_node.h>

CqNode node(true);

void setup()
{
  pinMode(D7, INPUT);

  if (digitalRead(D7) == HIGH)
  {
    node.ClearConfiguration();
  }

  node.Begin();
}

void loop()
{
}