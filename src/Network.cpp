#include "Network.h"
#include "addons/TokenHelper.h"

#define WIFI_SSID "DRIFTYY777 7440"
#define WIFI_PASSWORD "this is password"

#define API_KEY "api goes here"
#define FIREBASE_PROJECT_ID "project id or project name like that-esp"

#define USER_EMAIL "email.com"
#define USER_PASSWORD "password"

String documentPath = "kharwan/Roof_1";
String documentPath2 = "kharwan/Server_";

static Network *instance = NULL;

unsigned long dataMillis = 0;

FirebaseJson content;

Network::Network()
{
    instance = this;
}

void WiFiEventGotIP()
{
    Serial.print("LOCAL IP ADDRESS: ");
    Serial.println(WiFi.localIP());
}

void WiFiEventDisconnected()
{
    Serial.println("WIFI DISCONNECTED!");
    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void WiFiEventConnected()
{
    WiFiEventDisconnected();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED)
    {
        WiFi.scanNetworks();
        delay(200);
    }
    Serial.println("WIFI CONNECTED! BUT WAIT FOR THE LOCAL IP ADD");
    WiFiEventGotIP();
}

void Network::initNetwork()
{
    WiFiEventConnected();
    instance->firebaseInit();
    WiFi.setAutoConnect(true);
}

void FirestoreTokenStatusCallback(TokenInfo info)
{
    Serial.printf("Token Info: type = %s, status = %s\n", getTokenType(info).c_str(), getTokenStatus(info).c_str());
}

void Network::firebaseInit()
{
    config.api_key = API_KEY;
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;
    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
    config.token_status_callback = tokenStatusCallback;
    config.token_status_callback = FirestoreTokenStatusCallback;
    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
}

void Network::firestoreDataUpdate(double temp, double humi)
{
    Serial.println(" T:" + String(temp) + " H:" + String(humi));
    if (WiFi.status() == WL_CONNECTED && Firebase.ready() && (millis() - dataMillis > 400 || dataMillis == 0))
    {
        content.set("fields/temperature/doubleValue", String(temp).c_str());
        content.set("fields/humidity/doubleValue", String(humi).c_str());
        
        if (Firebase.Firestore.patchDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str(), content.raw(), "temperature,humidity"))
        {
            Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
            return;
        }
        else
        {
            Serial.println(fbdo.errorReason());
        }
        if (Firebase.Firestore.createDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str(), content.raw()))
        {
            Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
            return;
        }
        else
        {
            Serial.println(fbdo.errorReason());
        }
    }
}

void Network::firestoreCpuData(double Cpu_Temp, double CpuFrequencyMhz)
{
    if (WiFi.status() == WL_CONNECTED && Firebase.ready() && (millis() - dataMillis > 400 || dataMillis == 0))
    {
        
        content.set("fields/Cpu_temperature/doubleValue", String(Cpu_Temp).c_str());
        content.set("fields/Cpu_frequency/doubleValue", String(CpuFrequencyMhz).c_str());

        if (Firebase.Firestore.patchDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath2.c_str(), content.raw(), "Cpu Temperature, Cpu_frequency"))
        {
            Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
            return;
        }
        else
        {
            Serial.println(fbdo.errorReason());
        }
        if (Firebase.Firestore.createDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath2.c_str(), content.raw()))
        {
            Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
            return;
        }
        else
        {
            Serial.println(fbdo.errorReason());
        }
    }
}
