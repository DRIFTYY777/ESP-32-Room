#include <freertos/FreeRTOS.h>
#include "wifi/WiFiSettings.h"
#include <ArduinoOTA.h>
#include <server/server.h>
#include <battery/battery.h>
#include <DHT.h>
#include "pins.h"
#include <storage/storage.h>
#include "SPIFFS.h"
#include <time/time.h>

// https://github.com/espressif/arduino-esp32/blob/master/libraries/ArduinoOTA/src/ArduinoOTA.cpp

enum modes
{
    SLOW = 5,
    FAST = 1,
};

DHT dht(DHT11PIN, DHT11);
BATTERY battery;
FirebaseServer server;
LocalTime local_time;
Storage storage;

const uint64_t sleepInterval = 20 * 60 * 1000000ULL; //(20 minutes)

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
            vTaskDelay(20000 / portTICK_PERIOD_MS);
        }
        else
        {
            digitalWrite(LED, HIGH);
            delay(600);
            digitalWrite(LED, LOW);
        }
        if (voltage > 2.5)
        {
            vTaskDelay(modes::FAST);
        }
        else
        {
            vTaskDelay(modes::SLOW);
        }
    }
}

String dataBuffer = "";
void writeDataSDFun(void *pvParameters)
{
    char data[100];
    char tempSTR[10], humiSTR[10];
    for (;;)
    {
        float humi = NAN, temp = NAN;
        temp = dht.readTemperature();
        humi = dht.readHumidity();

        if (isnan(temp) || isnan(humi))
        {
            ESP_LOGE("MAIN", "Failed to read from DHT sensor!");
            vTaskDelay(5000 / portTICK_PERIOD_MS);
            continue;
        }
        dtostrf(temp, 6, 2, tempSTR);
        dtostrf(humi, 6, 2, humiSTR);
        sprintf(data, "%s, %s, %s\n", local_time.getFormattedTime().c_str(), tempSTR, humiSTR);
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
        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
}

void setup_ota()
{
    ArduinoOTA.setHostname(WiFiSettings.hostname.c_str());
    ArduinoOTA.setPassword(WiFiSettings.password.c_str());
    ArduinoOTA.begin();
}

void setup()
{

    Serial.begin(115200);

    if (!SPIFFS.begin(true))
    {
        ESP_LOGE("MAIN", "Failed to mount SPIFFS");
        return;
    }
    else
        ESP_LOGE("MAIN", "SPIFFS mounted successfully");

    battery.init();
    storage.init();
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

    dht.begin();

    WiFiSettings.secure = true;
    WiFiSettings.onPortal = []()
    {
        setup_ota();
    };
    WiFiSettings.onPortalWaitLoop = []()
    {
        ArduinoOTA.handle();
    };
    WiFiSettings.connect();
    ESP_LOGI("WiFi", "Password  %s", WiFiSettings.password.c_str());

    setup_ota();

    if (WiFi.status() == WL_CONNECTED)
    {
        server.firebase_init();
        local_time.begin();
        local_time.syncWithNTP();
    }

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

    String time = local_time.getFormattedTime();
    int hour = time.substring(0, 2).toInt();

    // if (hour >= 20 || hour < 6)
    // {
    //     // Enter sleep mode at night
    //     // wake esp after 20 minutes for next reading and upload, write to SD only in night then go to sleep again
    //     esp_sleep_enable_timer_wakeup(sleepInterval);
    //     esp_deep_sleep_start();
    // }
    ArduinoOTA.handle();
}