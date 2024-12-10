#ifndef __TIME_H__
#define __TIME_H__

#include <ESP32Time.h>
#include <WiFi.h>
#include <NTPClient.h>

class LocalTime
{
private:
    ESP32Time time;
    NTPClient ntp;
    WiFiUDP udp;

public:
    LocalTime(const char *ntpServer = "pool.ntp.org", int offsetSec = 19800, int updateInterval = 60000)
        : ntp(udp, ntpServer, offsetSec, updateInterval)
    {
    }
    void begin();
    void syncWithNTP();
    String getFormattedTime();
    bool isWiFiConnected();
};

#endif