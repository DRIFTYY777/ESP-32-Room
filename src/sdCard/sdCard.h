#ifndef SD_CARD_H_
#define SD_CARD_H_

#include "FS.h"
#include "SPIFFS.h"

#include "stdint.h"
#include "stdbool.h"

#define FILE_PATH "/data.txt"

class SD_CARD
{

private:
public:
    void init();
    void sd_get_file(const char *path, void *data);
    void appendFile(fs::FS &fs, const char *path, const char *message);
    size_t sd_file_size(const char *path);

    esp_err_t changeSDformat();
};

#endif

// /********************************
//  *   SD CARD pin configuration
//  * ******************************/

// #define VSPI_MOSI       23
// #define VSPI_MISO       19
// #define VSPI_CLK        18
// #define VSPI_CS0        5