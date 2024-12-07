#include <server/server.h>
#include "addons/TokenHelper.h"

void FirebaseServer::fcsUploadCallback(CFS_UploadStatusInfo info)
{
    if (info.status == fb_esp_cfs_upload_status_init)
    {
        // Serial.printf("\nUploading data (%d)...\n", info.size);
        ESP_LOGD(TAG, "Uploading data (%d)...", info.size);
    }
    else if (info.status == fb_esp_cfs_upload_status_upload)
    {
        // Serial.printf("Uploaded %d%s\n", (int)info.progress, "%");
        ESP_LOGD(TAG, "Uploaded %d%s", (int)info.progress, "%");
    }
    else if (info.status == fb_esp_cfs_upload_status_complete)
    {
        // Serial.println("Upload completed ");?
        ESP_LOGD(TAG, "Upload completed");
    }
    else if (info.status == fb_esp_cfs_upload_status_process_response)
    {
        // Serial.print("Processing the response... ");
        ESP_LOGW(TAG, "Processing the response...");
    }
    else if (info.status == fb_esp_cfs_upload_status_error)
    {
        // Serial.printf("Upload failed, %s\n", info.errorMsg.c_str());
        ESP_LOGE(TAG, "Upload failed, %s", info.errorMsg.c_str());
    }
}

void FirebaseServer::firebase_init()
{
    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
    config.api_key = API_KEY;
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;
    config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h
    fbdo.setResponseSize(2048);
    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
}

void FirebaseServer::write_data(double temp, double humi, int32_t rssi, float batteryVoltage, float batteryPercentage)
{
    if (Firebase.ready() && (millis() - dataMillis > 60000 || dataMillis == 0))
    {
        dataMillis = millis();
        FirebaseJson content;
        String documentPath = "kharwan/Roof_1";

        content.set("fields/temperature/doubleValue", String(temp).c_str());
        content.set("fields/humidity/doubleValue", String(humi).c_str());
        
        content.set("fields/rssi/integerValue", String(rssi).c_str());
        content.set("fields/batteryVoltage/doubleValue", String(batteryVoltage).c_str());
        content.set("fields/batteryPercentage/doubleValue", String(batteryPercentage).c_str());

        if (Firebase.Firestore.patchDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str(), content.raw(), "temperature,humidity,rssi,batteryVoltage,batteryPercentage"))
        {
            // Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
            ESP_LOGI(TAG, "ok\n%s\n\n", fbdo.payload().c_str());
            return;
        }
        else
        {
            Serial.println(fbdo.errorReason());
        }
        if (Firebase.Firestore.createDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str(), content.raw()))
        {
            // Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
            ESP_LOGI(TAG, "ok\n%s\n\n", fbdo.payload().c_str());
            return;
        }
        else
        {
            // Serial.println(fbdo.errorReason());
            ESP_LOGI(TAG, fbdo.errorReason());
        }
    }
}
