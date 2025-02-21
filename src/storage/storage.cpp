#include "storage.h"
#include "pins.h"

static const char *TAG = "Storage";

uint8_t Storage::readByte(uint32_t address)
{
    uint8_t result;

    digitalWrite(FLASH_CS, LOW);               // Select the flash chip
    spiFlash.transfer(0x03);                   // Read command
    spiFlash.transfer((address >> 16) & 0xFF); // Address MSB
    spiFlash.transfer((address >> 8) & 0xFF);  // Address Mid
    spiFlash.transfer(address & 0xFF);         // Address LSB
    result = spiFlash.transfer(0x00);          // Read data byte
    digitalWrite(FLASH_CS, HIGH);              // Deselect the flash chip

    return result;
}

void Storage::init()
{
    spiFlash.begin(FLASH_CS, FLASH_SCK, FLASH_SO, FLASH_SI);
    spiFlash.setFrequency(SPI_SPEED);

    pinMode(FLASH_CS, OUTPUT);
    digitalWrite(FLASH_CS, HIGH);

    pinMode(FLASH_WP, OUTPUT);
    pinMode(FLASH_HOLD, OUTPUT);
    digitalWrite(FLASH_WP, HIGH);   // Disable write protection
    digitalWrite(FLASH_HOLD, HIGH); // Disable hold mode
}

void Storage::clearFlash()
{
    digitalWrite(FLASH_CS, LOW);
    spiFlash.transfer(0x06); // Write Enable
    digitalWrite(FLASH_CS, HIGH);

    digitalWrite(FLASH_CS, LOW);
    spiFlash.transfer(0xC7); // Chip Erase
    digitalWrite(FLASH_CS, HIGH);

    delay(10000); // Wait for erase to complete
}

void Storage::create_file(const char *filepath)
{
    // create file on external flash
    File file = SPIFFS.open(filepath, FILE_WRITE);
    if (!file)
    {
        // Serial.println("Failed to create file");
        ESP_LOGE(TAG, "Failed to create file");
        return;
    }
}

String Storage::read_file(const char *filepath)
{
    // Read a file from external flash
    File file = SPIFFS.open(filepath, FILE_READ);
    if (!file)
    {
        // Log error if file can't be opened
        ESP_LOGE(TAG, "Failed to open file: %s", filepath);
        return String(); // Return empty string if file is not found
    }

    // Read file content into a string
    String fileContent = "";
    while (file.available())
    {
        fileContent += (char)file.read(); // Append each byte as a character
    }
    // Close the file after reading
    file.close();
    // Return the file contents as a string
    ESP_LOGI(TAG, "File content read successfully from %s", filepath);
    return fileContent;
}

void Storage::delete_file(const char *filepath)
{
    // Delete a file from external flash
    if (!SPIFFS.remove(filepath))
    {
        // Log error if file can't be deleted
        ESP_LOGE(TAG, "Failed to delete file: %s", filepath);
        return;
    }
    // Log success if file is deleted
    ESP_LOGI(TAG, "File deleted successfully: %s", filepath);
}

void Storage::printFlashDetails()
{
    uint8_t manufacturerID, deviceID;
    uint32_t JEDEC_ID;

    digitalWrite(FLASH_CS, LOW);
    spiFlash.transfer(0x9F); // JEDEC ID
    manufacturerID = spiFlash.transfer(0x00);
    deviceID = spiFlash.transfer(0x00);
    JEDEC_ID = spiFlash.transfer(0x00) << 16;
    JEDEC_ID |= spiFlash.transfer(0x00) << 8;
    JEDEC_ID |= spiFlash.transfer(0x00);
    digitalWrite(FLASH_CS, HIGH);

    // Serial.printf("Manufacturer ID: %02X\n", manufacturerID);
    // Serial.printf("Device ID: %02X\n", deviceID);
    // Serial.printf("JEDEC ID: %06X\n", JEDEC_ID);
    ESP_LOGI(TAG, "Manufacturer ID: %02X", manufacturerID);
    ESP_LOGI(TAG, "Device ID: %02X", deviceID);
    ESP_LOGI(TAG, "JEDEC ID: %06X", JEDEC_ID);
}

void Storage::append_file(const char *filepath, const char *data)
{
    // Append data to a file on external flash
    File file = SPIFFS.open(filepath, FILE_APPEND);
    if (!file)
    {
        // Log error if file can't be opened
        ESP_LOGE(TAG, "Failed to open file: %s", filepath);
        return;
    }
    // Append data to the file
    file.print(data);
    // Close the file after appending
    file.close();
    // Log success if data is appended
    ESP_LOGI(TAG, "Data appended successfully to %s", filepath);
}

void Storage::append_file(const char *filepath, String *data)
{
    // Append data to a file on external flash
    File file = SPIFFS.open(filepath, FILE_APPEND);
    if (!file)
    {
        // Log error if file can't be opened
        ESP_LOGE(TAG, "Failed to open file: %s", filepath);
        return;
    }
    // Append data to the file
    file.print(*data);
    // Close the file after appending
    file.close();
    // Log success if data is appended
    ESP_LOGI(TAG, "Data appended successfully to %s", filepath);
}

size_t Storage::total_space()
{
    return SPIFFS.totalBytes();
}

size_t Storage::used_space()
{
    return SPIFFS.usedBytes();
}

size_t Storage::free_space()
{
    return SPIFFS.totalBytes() - SPIFFS.usedBytes();
}

size_t Storage::file_size(const char *filepath)
{
    // Get the size of a file on external flash
    File file = SPIFFS.open(filepath, FILE_READ);
    if (!file)
    {
        // Log error if file can't be opened
        ESP_LOGE(TAG, "Failed to open file: %s", filepath);
        return 0;
    }
    // Get the file size
    size_t fileSize = file.size();
    // Close the file after getting the size
    file.close();
    // Return the file size
    return fileSize;
}

bool Storage::file_exists(const char *filepath)
{
    // Check if a file exists on external flash
    return SPIFFS.exists(filepath);
}


Storage storage; 
