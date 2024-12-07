#ifndef _SERVER_H
#define _SERVER_H

#include <Arduino.h>
#include <Firebase_ESP_Client.h>
#include <credentials.h>

static const char *TAG = "Battery";

class FirebaseServer
{

private:
    FirebaseConfig config;
    FirebaseData fbdo;
    FirebaseAuth auth;
    unsigned long dataMillis = 0;
    int count = 0;

public:
    void fcsUploadCallback(CFS_UploadStatusInfo info);
    void firebase_init();
    void write_data(double temp, double humi, int32_t rssi, float batteryVoltage, float batteryPercentage);
};

#endif
