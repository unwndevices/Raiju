#include <Arduino.h>
#include "Instrument.h"
#include "WebApp.h"

#define DEBUG 1

Instrument raiju;
WebApp app;

void setup()
{
  if (DEBUG)
  {
    Serial.begin(115200);
    Serial.setDebugOutput(true);
  }
  raiju.begin();
  app.init();
}

void loop()
{
  raiju.update();
}
