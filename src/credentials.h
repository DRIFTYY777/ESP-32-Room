#ifndef __CREDENTIALS_H__
#define __CREDENTIALS_H__


#define API_KEY ""
#define FIREBASE_PROJECT_ID "that-esp"
#define USER_EMAIL ""
#define USER_PASSWORD ""

#define SERVER_KEY ""

#define SENDER_ID ""

const char PRIVATE_KEY[] PROGMEM = "-----BEGIN PRIVATE KEY-----XXXXXXXXXXXX-----END PRIVATE KEY-----\n";

//FILE_PATH
#define FILE_PATH "/data.txt"
#define REPORT_FILE_PATH "/report.txt"

// Sleep interval
const uint64_t sleepInterval = 20 * 60 * 1000000ULL; //(20 minutes)


#endif
