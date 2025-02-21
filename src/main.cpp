/*
    ssdi- esp32- something
    Password - XC-mho!-z5mHH8E9
*/

#include <freertos/FreeRTOS.h>
#include "wifi/WiFiSettings.h"
#include <ArduinoOTA.h>
#include <server/server.h>
#include <battery/battery.h>
#include <DHT.h>
#include "pins.h"
#include "storage/storage.h"
#include "SPIFFS.h"
#include <time/time.h>
#include <credentials.h>
#include "report/report.h"

// https://github.com/espressif/arduino-esp32/blob/master/libraries/ArduinoOTA/src/ArduinoOTA.cpp

DHT dht(DHT11PIN, DHT11);
BATTERY battery;
FirebaseServer server;
// LocalTime local_time;
// Storage storage;

TaskHandle_t UploadDataCore;
TaskHandle_t writeDataSDCore;
SemaphoreHandle_t sdCardMutex;

void readDHTValues(float &temp, float &humi)
{
    humi = dht.readHumidity();
    temp = dht.readTemperature();
}

void getWIFI(int32_t &rssi)
{
    rssi = WiFi.RSSI();
}

void print_IP()
{
    ESP_LOGI("MAIN", "IP Address: %s", WiFi.localIP().toString().c_str());
}

void getPower(float &voltage, float &current)
{
    voltage = battery.getBatteryVoltage();
    current = battery.batteyPercentage();
}

void uploadDataFun(void *pvParameters)
{
    for (;;)
    {
        float voltage, current;
        if (WiFi.status() == WL_CONNECTED)
        {
            int32_t rssi;
            float humi, temp;

            readDHTValues(temp, humi);
            getWIFI(rssi);
            getPower(voltage, current);

            ESP_LOGD("MAIN", "Temperature: %f, Humidity: %f, RSSI: %d, Voltage: %f, Current: %f", temp, humi, rssi, voltage, current);

            server.write_data(temp, humi, rssi, voltage, current);

            esp_sleep_enable_timer_wakeup(60000000); // 1 minute in microseconds
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
        else
        {
            report.printReport("MAIN", "void uploadDataFun(void *pvParameters) == WiFi not connected");
            vTaskDelay(300000 / portTICK_PERIOD_MS);
        }
    }
}

String dataBuffer = "";
void writeDataSDFun(void *pvParameters)
{
    char data[100];
    char tempSTR[10], humiSTR[10];

    char batteryVoltage[10], batteryPercentage[10];

    for (;;)
    {
        float humi = NAN, temp = NAN, voltage = NAN, current = NAN;
        temp = dht.readTemperature();
        humi = dht.readHumidity();
        voltage = battery.getBatteryVoltage();
        current = battery.batteyPercentage();

        if (isnan(temp) || isnan(humi))
        {
            ESP_LOGE("MAIN", "Failed to read from DHT sensor!");
            // report.printReport("MAIN", "Failed to read from DHT sensor!");
            // 5 min delay
            vTaskDelay(300000 / portTICK_PERIOD_MS);
            continue;
        }
        dtostrf(temp, 6, 2, tempSTR);
        dtostrf(humi, 6, 2, humiSTR);
        dtostrf(voltage, 6, 2, batteryVoltage);
        dtostrf(current, 6, 2, batteryPercentage);
        sprintf(data, "%s, %s, %s, %s, %s\n", local_time.getFormattedTime().c_str(), tempSTR, humiSTR, batteryVoltage, batteryPercentage);
        dataBuffer += data;

        // Write to storage only every 10 readings
        if (dataBuffer.length() >= 1000 || millis() % 60000 == 0)
        {
            if (xSemaphoreTake(sdCardMutex, portMAX_DELAY) == pdTRUE)
            {
                storage.append_file(FILE_PATH, dataBuffer.c_str());
                dataBuffer = ""; // Clear buffer after writing
                xSemaphoreGive(sdCardMutex);
            }
        }
        vTaskDelay(300000 / portTICK_PERIOD_MS);
    }
}

void setup_ota()
{

    WiFiSettings.secure = true;
    WiFiSettings.onPortal = []()
    {
        ArduinoOTA.setHostname(WiFiSettings.hostname.c_str());
        ArduinoOTA.setPassword(WiFiSettings.password.c_str());
        ArduinoOTA.begin();
    };
    WiFiSettings.onPortalWaitLoop = []()
    {
        ArduinoOTA.handle();
    };
    WiFiSettings.connect();
    ESP_LOGI("WiFi", "Password  %s", WiFiSettings.password.c_str());
}

void getCpuTemp()
{
    // Get the CPU temperature
    float temp = (float)temperatureRead() / 100;
    ESP_LOGI("MAIN", "CPU Temperature: %f", temp);
}

void indicator(int times)
{
    for (int i = 0; i < times; i++)
    {
        digitalWrite(LED, HIGH);
        delay(100);
        digitalWrite(LED, LOW);
        delay(100);
    }
}

void setup()
{

    Serial.begin(115200);
    pinMode(LED, OUTPUT);
    indicator(5);
    if (!SPIFFS.begin(true))
    {
        ESP_LOGE("MAIN", "Failed to mount SPIFFS");
        return;
    }
    else
        ESP_LOGE("MAIN", "SPIFFS mounted successfully");

    dht.begin();
    battery.init();
    storage.init();
    report.begin();

    if (!storage.file_exists(FILE_PATH))
    {
        storage.create_file(FILE_PATH);
    }

    sdCardMutex = xSemaphoreCreateMutex();
    if (sdCardMutex == NULL)
    {
        ESP_LOGE("MAIN", "Failed to create mutex!");
        // for (;;)
        // {
        // } // Halt execution if mutex creation fails
    }

    setup_ota();

    if (WiFi.status() == WL_CONNECTED)
    {
        server.firebase_init();
        local_time.begin();
        local_time.syncWithNTP();
    }

    delay(2000); // for being stable

    print_IP();

    ArduinoOTA.begin();

    xTaskCreatePinnedToCore(
        uploadDataFun,
        "Task1",
        8192,
        NULL,
        2,
        &UploadDataCore,
        0);
    xTaskCreatePinnedToCore(
        writeDataSDFun,
        "Task2",
        6192,
        NULL,
        1,
        &writeDataSDCore,
        1);
    delay(1000);
}
void loop()
{
    if (local_time.isWiFiConnected())
    {
        local_time.syncWithNTP();
    }

    String time = local_time.getFormattedTime(); // Assuming local_time is an instance of an RTC or NTP time object
    int hour = time.substring(0, 2).toInt();     // Extract the hour part of the time

    // Start deep sleep
    ESP_LOGI("MAIN", "Entering Deep Sleep Mode");
    esp_sleep_enable_timer_wakeup(sleepInterval); // 20 minutes in microseconds
    esp_deep_sleep_start();

    ArduinoOTA.handle();
}