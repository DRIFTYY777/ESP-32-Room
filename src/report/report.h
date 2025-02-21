#ifndef __REPORT_H__
#define __REPORT_H__

#include <WString.h>
#include <Arduino.h>

class Report
{
private:
public:
    void begin();
    void printReport(const char *TAG, const char *message);
    void printReport(const char *TAG, const char *message, String RAWtime);
};

extern Report report;

#endif