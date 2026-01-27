#ifndef ETC_HPP
#define ETC_HPP

#include <Arduino.h>

inline String getChipID() {
    uint64_t chipid = ESP.getEfuseMac();
    char chipid_str[13];
    sprintf(chipid_str, "%04X%08X", (uint16_t)(chipid >> 32), (uint32_t)chipid);
    return String(chipid_str);
}

inline void printHeapInfo() {
    Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
    if (psramFound()) {
        Serial.printf("PSRAM size: %d bytes\n", ESP.getPsramSize());
        Serial.printf("Free PSRAM: %d bytes\n", ESP.getFreePsram());
    }
}

#endif // ETC_HPP
