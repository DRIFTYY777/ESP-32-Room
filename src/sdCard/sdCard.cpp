#include "sdCard.h"
#include "esp_log.h"
#include <driver/sdmmc_types.h>
#include <driver/sdspi_host.h>
#include "pins.h"
#include "vfs_fat_internal.h"
#include "ff.h"
#include <SD.h>
#include "pins.h"

static const char *TAG = "SD_CARD";



size_t SD_CARD::sd_file_size(const char *path)
{
    FILE *fd = fopen(path, "rb");

    fseek(fd, 0, SEEK_END);
    size_t actual_size = ftell(fd);
    fseek(fd, 0, SEEK_SET);

    ESP_LOGI(TAG, "Size: %i bytes", actual_size);
    fclose(fd);

    return actual_size;
}

void SD_CARD::sd_get_file(const char *path, void *data)
{
    const size_t BLOCK_SIZE = 512; // We're going to read file in chunk of 512 Bytes
    size_t r = 0;
    FILE *fd = fopen(path, "rb"); // Open the file in binary read mode
    if (fd == NULL)
    {
        ESP_LOGE(TAG, "Error opening: %s ", path);
    }
    while (true)
    {
        size_t count = fread((uint8_t *)data + r, 1, BLOCK_SIZE, fd);
        r += count;
        if (count < BLOCK_SIZE)
            break;
    }
    fclose(fd);
}

void SD_CARD::init()
{
    SPIFFS.begin(true);

    if (!SD.begin(SD_CS))
    {
        ESP_LOGE(TAG, "Card Mount Failed");
        return;
    }
    else
    {
        ESP_LOGI(TAG, "Init SD Card");
        uint8_t cardType = SD.cardType();
        ESP_LOGI(TAG, "SD Card Type: %s", cardType == CARD_NONE ? "No card" : cardType == CARD_MMC ? "MMC"
                                                                          : cardType == CARD_SD    ? "SDSC"
                                                                          : cardType == CARD_SDHC  ? "SDHC"
                                                                                                   : "UNKNOWN");
    }
    ESP_LOGI(TAG, "SD Card Size: %lluMB", SD.cardSize() / (1024 * 1024));
    ESP_LOGI(TAG, "Total space: %lluMB", SD.totalBytes() / (1024 * 1024));
    ESP_LOGI(TAG, "Used space: %lluMB", SD.usedBytes() / (1024 * 1024));
}

void SD_CARD::appendFile(fs::FS &fs, const char *path, const char *message)
{
    ESP_LOGI(TAG, "Appending to file: %s\r\n", path);

    File file = fs.open(path, FILE_APPEND);
    if (!file)
    {
        ESP_LOGI(TAG, "Failed to open file for appending");
        return;
    }
    if (file.print(message))
        ESP_LOGI(TAG, "− message appended");
    else
        ESP_LOGI(TAG, "− append failed");
}

esp_err_t SD_CARD::changeSDformat()
{
    char drv[3] = {'0', ':', 0};
    const size_t workbuf_size = 4096;
    void *workbuf = NULL;
    esp_err_t err = ESP_OK;
    ESP_LOGW("sdcard", "Formatting the SD card");

    size_t allocation_unit_size = 16 * 1024;
    int sector_size_default = 512;

    workbuf = ff_memalloc(workbuf_size);
    if (workbuf == NULL)
    {
        return ESP_ERR_NO_MEM;
    }

    size_t alloc_unit_size = esp_vfs_fat_get_allocation_unit_size(
        sector_size_default,
        allocation_unit_size);

#if (ESP_IDF_VERSION_MAJOR < 5)
    FRESULT res = f_mkfs(drv, FM_ANY, alloc_unit_size, workbuf, workbuf_size);
#else
    const MKFS_PARM opt = {(BYTE)FM_ANY, 0, 0, 0, alloc_unit_size};
    FRESULT res = f_mkfs(drv, &opt, workbuf, workbuf_size);
#endif /* ESP_IDF_VERSION_MAJOR */
    if (res != FR_OK)
    {
        err = ESP_FAIL;
        ESP_LOGE("sdcard", "f_mkfs failed (%d)", res);
    }

    free(workbuf);

    ESP_LOGI("sdcard", "Successfully formatted the SD card");

    return err;
}
