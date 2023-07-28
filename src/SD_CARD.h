#ifndef SD_CARD_H_
#define SD_CARD_H_

#include "FS.h"
#include "SPIFFS.h"
#define FORMAT_SPIFFS_IF_FAILED true

class SD_CARD
{
private:
public:
    SD_CARD();
    void initSd();
    void appendFile(fs::FS &fs, const char *path, const char *message);
};

#endif