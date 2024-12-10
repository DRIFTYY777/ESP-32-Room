/*
    25Q32CSIG Flash Chip is used for storage
    32Mbit (4M x 8)
    Storing weather data and other data (Reports, etc)
    2 modes ready for use:
        - SPI Mode
        - QSPI Mode
*/

#ifndef __STORAGE_H__
#define __STORAGE_H__

#include <SPI.h>
#include <SPIFFS.h>

#define SPI_SPEED 8000000 // 8 MHz (adjust as needed)

class Storage
{
private:
    SPIClass spiFlash;
    uint8_t readByte(uint32_t address);

public:
    Storage() : spiFlash(VSPI) {}; // Initialize with VSPI
    void init();
    void clearFlash();
    void create_file(const char *filepath);
    String read_file(const char *filepath);
    void delete_file(const char *filepath);
    void printFlashDetails();
    void append_file(const char *filepath, const char *data);
    void append_file(const char *filepath, String *data);
    size_t total_space();
    size_t used_space();
    size_t free_space();
    size_t file_size(const char *filepath);


    bool file_exists(const char *filepath);
};

#endif