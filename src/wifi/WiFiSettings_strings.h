#include <map>

namespace WiFiSettingsLanguage
{

    struct Texts
    {
        const __FlashStringHelper
            *title,
            *portal_wpa,
            *portal_password,
            *wait,
            *bye,
            *error_fs,
            *button_save,
            *button_restart,
            *scanning_short,
            *scanning_long,
            *rescan,
            *dot1x,
            *ssid,
            *wifi_password,
            *language;
        const char
            *init;
    };

#if !defined LANGUAGE_EN
#define LANGUAGE_ALL
#endif

    std::map<const String, const String> languages
    {

#if defined LANGUAGE_EN || defined LANGUAGE_ALL
        {"en", "English"},
#endif
    };

    bool available(const String &language)
    {
        return languages.count(language) == 1;
    }

    bool multiple()
    {
        return languages.size() > 1;
    }

    bool select(Texts &T, String &language)
    {
        if (!available(language))
        {
            if (available("en"))
                language = "en";
            else
                language = languages.begin()->first;
        }

#if defined LANGUAGE_EN || defined LANGUAGE_ALL
        if (language == "en")
        {
            T.title = F("Configuration");
            T.portal_wpa = F("Protect the configuration portal with a WiFi password");
            T.portal_password = F("WiFi password for the configuration portal");
            T.init = "default";
            T.wait = F("Wait for it...");
            T.bye = F("Bye!");
            T.error_fs = F("Error while writing to flash filesystem.");
            T.button_save = F("Save");
            T.button_restart = F("Restart device");
            T.scanning_short = F("Scanning...");
            T.scanning_long = F("Scanning for WiFi networks...");
            T.rescan = F("rescan");
            T.dot1x = F("(won't work: 802.1x is not supported)");
            T.ssid = F("WiFi network name (SSID)");
            T.wifi_password = F("WiFi password");
            T.language = F("Language");
            return true;
        }
#endif
        return false;
    }

} // namespace
