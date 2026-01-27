#include "wifi_module.hpp"

bool WifiModule::connect()
{
    if (m_ssid.length() == 0)
    {
        Serial.println("SSID not set");
        return false;
    }

    Serial.printf("Connecting to WiFi: %s\n", m_ssid.c_str());
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(m_ssid.c_str(), m_password.c_str());

    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED)
    {
        if (millis() - startTime > m_connectTimeout)
        {
            Serial.println("WiFi connection timeout");
            return false;
        }
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    Serial.printf("WiFi connected, IP: %s\n", WiFi.localIP().toString().c_str());
    m_connected = true;
    return true;
}

bool WifiModule::connect(const String& ssid, const String& password)
{
    m_ssid = ssid;
    m_password = password;
    return connect();
}

void WifiModule::disconnect()
{
    WiFi.disconnect();
    m_connected = false;
    Serial.println("WiFi disconnected");
}

void WifiModule::parseCmd(std::vector<String> &tokens, JsonDocument &_res_doc)
{
    int _tokenCount = tokens.size();

    if (_tokenCount > 1)
    {
        String subCmd = tokens[1];

        if (subCmd == "set")
        {
            if (_tokenCount > 3)
            {
                String key = tokens[2];
                String value = tokens[3];

                if (key == "ssid")
                {
                    setSSID(value);
                    _res_doc["result"] = "ok";
                    _res_doc["ms"] = "ssid set";
                }
                else if (key == "password" || key == "pass" || key == "pw")
                {
                    setPassword(value);
                    _res_doc["result"] = "ok";
                    _res_doc["ms"] = "password set";
                }
                else if (key == "timeout")
                {
                    setConnectTimeout(value.toInt());
                    _res_doc["result"] = "ok";
                    _res_doc["ms"] = "timeout set";
                }
                else
                {
                    _res_doc["result"] = "fail";
                    _res_doc["ms"] = "unknown key (ssid/password/timeout)";
                }
            }
            else
            {
                _res_doc["result"] = "fail";
                _res_doc["ms"] = "need key and value";
            }
        }
        else if (subCmd == "connect")
        {
            // wifi connect ssid password 형태도 지원
            if (_tokenCount > 3)
            {
                if (connect(tokens[2], tokens[3]))
                {
                    _res_doc["result"] = "ok";
                    _res_doc["ms"] = "connected";
                    _res_doc["ip"] = getIP();
                }
                else
                {
                    _res_doc["result"] = "fail";
                    _res_doc["ms"] = "connection failed";
                }
            }
            else
            {
                if (connect())
                {
                    _res_doc["result"] = "ok";
                    _res_doc["ms"] = "connected";
                    _res_doc["ip"] = getIP();
                }
                else
                {
                    _res_doc["result"] = "fail";
                    _res_doc["ms"] = "connection failed";
                }
            }
        }
        else if (subCmd == "disconnect")
        {
            disconnect();
            _res_doc["result"] = "ok";
            _res_doc["ms"] = "disconnected";
        }
        else if (subCmd == "status")
        {
            _res_doc["result"] = "ok";
            _res_doc["connected"] = isConnected();
            _res_doc["ssid"] = m_ssid;
            if (isConnected())
            {
                _res_doc["ip"] = getIP();
                _res_doc["rssi"] = getRSSI();
                _res_doc["mac"] = getMac();
            }
        }
        else if (subCmd == "scan")
        {
            Serial.println("Scanning WiFi networks...");
            int n = WiFi.scanNetworks();
            
            _res_doc["result"] = "ok";
            _res_doc["count"] = n;
            
            JsonArray networks = _res_doc["networks"].to<JsonArray>();
            for (int i = 0; i < n && i < 10; i++)  // 최대 10개까지만
            {
                JsonObject network = networks.add<JsonObject>();
                network["ssid"] = WiFi.SSID(i);
                network["rssi"] = WiFi.RSSI(i);
                network["enc"] = WiFi.encryptionType(i) != WIFI_AUTH_OPEN;
            }
            
            WiFi.scanDelete();
        }
        else
        {
            _res_doc["result"] = "fail";
            _res_doc["ms"] = "unknown sub command";
        }
    }
    else
    {
        _res_doc["result"] = "fail";
        _res_doc["ms"] = "need sub command (set/connect/disconnect/status/scan)";
    }
}
