#include <Ticker.h>
#include "Network.h"
#include "DHT.h"
#include "SD_CARD.h"
#include <iostream>
#include <string.h>
#include <NTPClient.h>

#ifndef ESP32
#pragma message(THIS EXAMPLE IS FOR ESP32 ONLY !)
#error Select ESP32 board.
#endif

#define DHT11PIN 22       // Define pin numbers dht sensor
DHT dht(DHT11PIN, DHT11); // Define global Again dht sensor
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

Network *network;
SD_CARD *sd_card;

String formattedDate;
String dayStamp;
String timeStamp;
String dataMessage;

void initNetwork()
{
    network = new Network();
    network->initNetwork();
}

void getTimeStamp() // this man taking time from server and store it in its own global variables
{
    while (!timeClient.update()) // loop for updating time
    {
        timeClient.forceUpdate();
    }

    formattedDate = timeClient.getFormattedTime();

    Serial.println(formattedDate);
    int splitT = formattedDate.indexOf("T");
    dayStamp = formattedDate.substring(0, splitT);
    Serial.println(dayStamp);
    timeStamp = formattedDate.substring(splitT + 1, formattedDate.length() - 1);
    Serial.println(timeStamp);
}

void Time_Init()
{
    timeClient.begin(); // server on
    timeClient.setTimeOffset(19800);
    getTimeStamp();
}

void initSd()
{
    sd_card = new SD_CARD();
    sd_card->initSd();
}

void setup()
{
    Serial.begin(115200);
    Serial.println();
    dht.begin();
    // initSd();
    initNetwork();
    Time_Init();
}

void loop()
{

    // double humidity = dht.readHumidity();
    double humidity = 35165;
    // double temperature = dht.readTemperature();
    double temperature = 6516;
    String dataMessage = "Day =  " + String(dayStamp) + "  Time =  " + String(timeStamp) + "    T = " + String(temperature) + "     H = " + String(humidity) + "\r\n"; // this is printing data in sd card with date and time
    // delay(4000);

    float Cpu_Temperature = temperatureRead();

    Serial.println(dataMessage);
    // sd_card->appendFile(SPIFFS, "/data.txt", dataMessage.c_str());
    network->firestoreDataUpdate(temperature, humidity);
    network->firestoreCpuData(Cpu_Temperature, 1);
    yield();
}
