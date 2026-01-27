#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <ArduinoJson.h>
#include <EEPROM.h>
#include <vector>
#include <nvs_flash.h>

class Config
{
public:
    const static int SystemVersion = 1;
    static const size_t EEPROM_SIZE = 2048;
    static const int EEPROM_START_ADDRESS = 0;

    String jsonDoc;

    Config()
    {
        // NVS 초기화
        esp_err_t err = nvs_flash_init();
        if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
        {
            ESP_ERROR_CHECK(nvs_flash_erase());
            err = nvs_flash_init();
        }
        ESP_ERROR_CHECK(err);

        EEPROM.begin(EEPROM_SIZE);
        load();
    }

    void load()
    {
        char buffer[EEPROM_SIZE];
        for (size_t i = 0; i < EEPROM_SIZE; ++i)
        {
            buffer[i] = EEPROM.read(i);
        }

        if (buffer[0] != '{' && buffer[0] != '[')
        {
            jsonDoc = "{}";
        }
        else
        {
            jsonDoc = String(buffer);
        }
    }

    void save()
    {
        for (size_t i = 0; i < EEPROM_SIZE; ++i)
        {
            if (i < jsonDoc.length())
            {
                EEPROM.write(i, jsonDoc[i]);
            }
            else
            {
                EEPROM.write(i, 0);
            }
        }
        EEPROM.commit();
    }

    template <typename T>
    void set(const char *key, T value)
    {
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, jsonDoc);
        if (error)
        {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
            return;
        }

        doc[key] = value;
        serializeJson(doc, jsonDoc);
        save();
    }

    template <typename T>
    T get(const char *key, T defaultValue = T()) const
    {
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, jsonDoc);
        if (error)
        {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
            return defaultValue;
        }

        // 키가 존재하는지 확인 (타입 체크 대신 키 존재 여부만 체크)
        if (!doc.containsKey(key))
        {
            return defaultValue;
        }

        // as<T>()가 자동으로 타입 변환(숫자 -> 문자열)을 처리합니다.
        return doc[key].as<T>();
    }

    inline bool hasKey(const char *key) const
    {
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, jsonDoc);
        if (error)
        {
            return false;
        }
        return doc[key].is<JsonVariant>();
    }

    inline String dump() const
    {
        return jsonDoc;
    }

    inline void clear()
    {
        jsonDoc = "{}";
        save();
    }

    void parseCmd(std::vector<String> &tokens, JsonDocument &_res_doc);
};

// 숫자 체크 유틸리티
inline bool isNumber(String str)
{
    for (unsigned int i = 0; i < str.length(); i++)
    {
        if (!isDigit(str[i]) && str[i] != '-')
        {
            return false;
        }
    }
    return true;
}

#endif // CONFIG_HPP
