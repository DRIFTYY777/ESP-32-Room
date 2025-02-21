#include "report.h"
#include "storage/storage.h"
#include "time/time.h"
#include "credentials.h"

const char *TAG = "Report";

void Report::begin()
{
    if (!storage.file_exists(REPORT_FILE_PATH))
    {
        storage.create_file(REPORT_FILE_PATH);
        storage.append_file(REPORT_FILE_PATH, "Report Init...\n");
        ESP_LOGI(TAG, "Report file created");
    }
}

/// @brief Print the report when buffer is full to the console and write it to the file
/// @param TAG
/// @param message
void Report::printReport(const char *TAG, const char *message)
{
    String report = "[" + local_time.getFormattedTime() + "]" + "[" + TAG + "] " + message + "\n";
    storage.append_file(REPORT_FILE_PATH, &report);
    ESP_LOGI(TAG, "%s", report.c_str());
}

/// @brief
/// @param TAG
/// @param message
/// @param RAWtime
void Report::printReport(const char *TAG, const char *message, String RAWtime)
{
    String report = "[" + RAWtime + "]" + "[" + TAG + "] " + message + "\n";
    storage.append_file(REPORT_FILE_PATH, &report);
}

Report report;