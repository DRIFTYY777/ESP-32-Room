#ifndef __BATTERY_H__
#define __BATTERY_H__

#include "pins.h"
#include <esp_adc_cal.h>

const int DEFAULT_VREF = 1100; // Use adc2_vref_to_gpio() to obtain a better estimate

const int MAX_ANALOG_VAL = 4095;
const float MAX_BATTERY_VOLTAGE = 4.2; // Max LiPoly voltage of a 3.7 battery is 4.2
const int BAT_CAPACITY = 1000;         // 1000mAh battery capacity

static esp_adc_cal_characteristics_t *adc_chars;

class BATTERY
{
private:
    int R1 = 1000000; // 1M ohm
    int R2 = 10000;

    void activateBatteryPin(bool activate);

public:
    void init();
    float getBatteryVoltage();
    float batteyPercentage();
};

#endif