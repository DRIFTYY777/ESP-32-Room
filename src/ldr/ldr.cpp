#include "ldr.h"
#include <esp32-hal-gpio.h>
#include "pins.h"
#include <Arduino.h>

void LDR::init()
{
    pinMode(LDR_PIN, INPUT);
}

int LDR::readRaw()
{
    return analogRead(LDR_PIN);
}

int LDR::sunlightIntensity()
{
    int raw = readRaw();
    return map(raw, 0, 4095, 0, 100);
}
