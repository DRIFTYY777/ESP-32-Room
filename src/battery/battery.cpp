#include "battery.h"
#include <esp32-hal-log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp32-hal-gpio.h>
#include <report/report.h>

static const char *TAG = "Battery";

void BATTERY::activateBatteryPin(bool activate)
{
    // Activate the battery voltage reading pin
    if (activate)
    {
        // gpio_set_level(BATT_PIN_ACT, 1);
        digitalWrite(BATT_PIN_ACT, HIGH);
        vTaskDelay(100 / portTICK_PERIOD_MS); // Small delay to allow the pin to stabilize
    }
    else
    {
        // gpio_set_level(BATT_PIN_ACT, 0);
        digitalWrite(BATT_PIN_ACT, LOW);
    }
}

void BATTERY::init()
{
    pinMode(BATT_PIN_ACT, OUTPUT);
    pinMode(BATT_CHARGING_PIN, INPUT);

    // Configure ADC
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_12);

    adc_chars = (esp_adc_cal_characteristics_t *)calloc(1, sizeof(esp_adc_cal_characteristics_t));
    if (!adc_chars)
    {
        ESP_LOGE(TAG, "Failed to allocate memory for ADC characteristics");
        report.printReport(TAG, "Failed to allocate memory for ADC characteristics");
        return;
    }

    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_12, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);

    switch (val_type)
    {
    case ESP_ADC_CAL_VAL_EFUSE_VREF:
        ESP_LOGI(TAG, "ADC calibration: eFuse Vref");
        break;
    case ESP_ADC_CAL_VAL_EFUSE_TP:
        ESP_LOGI(TAG, "ADC calibration: Two Point");
        break;
    default:
        ESP_LOGW(TAG, "ADC calibration: Using default Vref");
        break;
    }
}

void BATTERY::batteryTemperature()
{
}

bool BATTERY::isCharging()
{
    if (digitalRead(BATT_CHARGING_PIN) == HIGH)
    {
        return true;
    }
    return false;
}

float BATTERY::getBatteryVoltage()
{
    activateBatteryPin(true); // Activate the battery voltage reading pin
    uint32_t adc_reading = 0;
    for (int i = 0; i < 10; i++)
    {
        adc_reading += adc1_get_raw(ADC1_CHANNEL_0); // Take 10 samples to average
        vTaskDelay(10 / portTICK_PERIOD_MS);         // Small delay to allow ADC to stabilize
    }
    adc_reading /= 10; // Average the readings

    // Convert the raw ADC reading to voltage using the calibration characteristics
    float voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);

    // Apply voltage divider (if applicable) and return the battery voltage
    float batteryVoltage = voltage * ((R1 + R2) / (float)R2);
    activateBatteryPin(false); // Deactivate the battery voltage reading pin
    return batteryVoltage;
}

float BATTERY::batteyPercentage()
{
    float percentage = (getBatteryVoltage() / MAX_BATTERY_VOLTAGE) * 100;
    return percentage;
}
