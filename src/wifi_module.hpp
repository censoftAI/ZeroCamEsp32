#ifndef WIFI_MODULE_HPP
#define WIFI_MODULE_HPP

#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <vector>

class WifiModule
{
private:
    String m_ssid;
    String m_password;
    bool m_connected = false;
    unsigned long m_connectTimeout = 10000;  // 10초

public:
    WifiModule() {}
    ~WifiModule() {}

    // WiFi 연결
    bool connect();
    bool connect(const String& ssid, const String& password);
    void disconnect();
    
    // 상태 확인
    inline bool isConnected() const { return WiFi.status() == WL_CONNECTED; }
    inline String getSSID() const { return m_ssid; }
    inline String getPassword() const { return m_password; }
    inline String getIP() const { return WiFi.localIP().toString(); }
    inline int getRSSI() const { return WiFi.RSSI(); }
    inline String getMac() const { return WiFi.macAddress(); }
    
    // 설정
    inline void setSSID(const String& ssid) { m_ssid = ssid; }
    inline void setPassword(const String& password) { m_password = password; }
    inline void setConnectTimeout(unsigned long timeout) { m_connectTimeout = timeout; }

    // 커맨드 파싱
    void parseCmd(std::vector<String> &tokens, JsonDocument &_res_doc);
};

#endif // WIFI_MODULE_HPP
