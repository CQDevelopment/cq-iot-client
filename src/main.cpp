#include <Arduino.h>
#include <CqNodeConfigurator.h>

CqNodeConfigurator configurator(true);

void setup()
{
  pinMode(D7, INPUT);

  if (digitalRead(D7) == HIGH)
  {
    configurator.Clear();
  }

  configurator.Begin();
}

void loop()
{
}