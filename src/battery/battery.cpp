#include "battery.h"
#include <esp32-hal-log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp32-hal-gpio.h>

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

    // // init BATT_PIN_ACT for mosfet driver to enable battery voltage reading esp-idf
    // gpio_config_t io_conf;
    // //interrupt of rising edge
    // io_conf.intr_type == GPIO_PIN_INTR_DISABLE;
    // //bit mask of the pins, use GPIO4/5 here
    // io_conf.pin_bit_mask = (1ULL << BATT_PIN_ACT);
    // //set as input mode
    // io_conf.mode = GPIO_MODE_OUTPUT;
    // //enable pull-up mode
    // io_conf.pull_up_en = GPIO_PULLUP_DISABLE;

    // init BATT_PIN_ACT ardunio
    pinMode(BATT_PIN_ACT, OUTPUT);

    // Configure the ADC width and channel for reading battery voltage (using GPIO 36)
    adc1_config_width(ADC_WIDTH_12Bit);                         // 12-bit ADC width
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_12); // GPIO 36 (ADC1_CHANNEL_0)

    // Initialize ADC calibration characteristics
    adc_chars = (esp_adc_cal_characteristics_t *)calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_12, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);

    if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF)
    {
        ESP_LOGI(TAG, "eFuse Vref");
    }
    else if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP)
    {
        ESP_LOGI(TAG, "Two Point");
    }
    else
    {
        ESP_LOGI(TAG, "Default");
    }
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
