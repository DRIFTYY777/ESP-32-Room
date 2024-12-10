#include <server/server.h>
#include "addons/TokenHelper.h"
#include "server.h"

void FirebaseServer::fcmInit()
{
    config.service_account.data.client_email = USER_EMAIL;
    config.service_account.data.project_id = FIREBASE_PROJECT_ID;
    config.service_account.data.private_key = PRIVATE_KEY;
}

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
    ESP_LOGI(TAG, "Firebase initialized successfully.");
}

void FirebaseServer::write_data(double temp, double humi, int32_t rssi, float batteryVoltage, float batteryPercentage)
{
    // if (Firebase.ready() && (millis() - dataMillis > 60000 || dataMillis == 0))
    // {
    //     dataMillis = millis();
    //     FirebaseJson content;
    //     String documentPath = "kharwan/Roof_1";

    //     content.set("fields/temperature/stringValue", String(temp).c_str());
    //     content.set("fields/humidity/stringValue", String(humi).c_str());

    //     content.set("fields/rssi/stringValue", String(rssi).c_str());
    //     content.set("fields/batteryVoltage/stringValue", String(batteryVoltage).c_str());
    //     content.set("fields/batteryPercentage/stringValue", String(batteryPercentage).c_str());

    //     if (Firebase.Firestore.patchDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str(), content.raw(), "temperature,humidity,rssi,batteryVoltage,batteryPercentage"))
    //     {
    //         // Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
    //         ESP_LOGI(TAG, "ok\n%s\n\n", fbdo.payload().c_str());
    //         return;
    //     }
    //     else
    //     {
    //         Serial.println(fbdo.errorReason());
    //     }
    //     if (Firebase.Firestore.createDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str(), content.raw()))
    //     {
    //         // Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
    //         ESP_LOGI(TAG, "ok\n%s\n\n", fbdo.payload().c_str());
    //         return;
    //     }
    //     else
    //     {
    //         // Serial.println(fbdo.errorReason());
    //         ESP_LOGI(TAG, "%s", fbdo.errorReason().c_str());
    //     }
    // }
    if (!Firebase.ready())
    {
        ESP_LOGW(TAG, "Firebase is not ready. Skipping upload.");
        return;
    }
    if (millis() - dataMillis < 60000 && dataMillis != 0)
    {
        return; // Avoid frequent uploads
    }
    dataMillis = millis();


    // Create JSON payload
    FirebaseJson payload;
    payload.set("fields/temperature/stringValue", String(temp).c_str());
    payload.set("fields/humidity/stringValue", String(humi).c_str());
    payload.set("fields/rssi/stringValue", String(rssi).c_str());
    payload.set("fields/batteryVoltage/stringValue", String(batteryVoltage).c_str());
    payload.set("fields/batteryPercentage/stringValue", String(batteryPercentage).c_str());

    String documentPath = "kharwan/Roof_1";

     // Try to patch or create the Firestore document
    if (Firebase.Firestore.patchDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str(), payload.raw(), "temperature,humidity,rssi,batteryVoltage,batteryPercentage")) {
        ESP_LOGI(TAG, "Data patched successfully: %s", fbdo.payload().c_str());
    } else if (Firebase.Firestore.createDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str(), payload.raw())) {
        ESP_LOGI(TAG, "Data created successfully: %s", fbdo.payload().c_str());
    } else {
        ESP_LOGE(TAG, "Failed to upload data: %s", fbdo.errorReason().c_str());
    }
}

void FirebaseServer::fcm_send_message(String message, String title, String token)
{
    FCM_HTTPv1_JSON_Message msg;
    msg.token = token;
    msg.notification.body = message;
    msg.notification.title = title;
    FirebaseJson payload;
    payload.add("message", message);
    payload.add("title", title);
    msg.data = payload.raw();
    if (Firebase.FCM.send(&fbdo, &msg))
    {
        // Serial.printf("ok\n%s\n\n", Firebase.FCM.payload(&fbdo).c_str());
        ESP_LOGI(TAG, "ok\n%s\n\n", Firebase.FCM.payload(&fbdo).c_str());
    }
    else
    {
        // Serial.println(fbdo.errorReason());
        ESP_LOGI(TAG, "%s", fbdo.errorReason().c_str());
    }
}
