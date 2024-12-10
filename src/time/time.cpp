#include "time.h"

static const char *TAG = "LOCAL_TIME";

void LocalTime::begin()
{
    ntp.begin();
}

void LocalTime::syncWithNTP()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        ntp.update();
        unsigned long epochTime = ntp.getEpochTime();
        // ESP_LOGD(TAG, "Epoch Time: %lu", epochTime);

        // Sync RTC with NTP time
        time.setTime(epochTime);
        // ESP_LOGD(TAG, "Time set to: %04d-%02d-%02d %02d:%02d:%02d\n",
        //       time.getYear(), time.getMonth() + 1, time.getDay(),
        //       time.getHour(), time.getMinute(), time.getSecond());

        // Serial.println("RTC synchronized with NTP.");
        // ESP_LOGD(TAG, "RTC synchronized with NTP (Kolkata Time).");
    }
    else
    {
        // Serial.println("WiFi not connected. Cannot sync with NTP.");
        ESP_LOGD(TAG, "WiFi not connected. Cannot sync with NTP.");
    }
}

String LocalTime::getFormattedTime()
{
    // Get current time from RTC
    char buffer[20];
    snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d %02d:%02d:%02d",
             time.getYear(), time.getMonth(), time.getDay(),
             time.getHour(), time.getMinute(), time.getSecond());
    return String(buffer);
}

bool LocalTime::isWiFiConnected()
{
    return WiFi.status() == WL_CONNECTED;
}
