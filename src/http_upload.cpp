#include "http_upload.hpp"
#include <WiFi.h>

int HttpUploader::uploadImage(uint8_t* data, size_t len, const String& fileName)
{
    String response;
    return uploadImage(data, len, response, fileName);
}

int HttpUploader::uploadImage(uint8_t* data, size_t len, String& response, const String& fileName)
{
    if (m_serverUrl.length() == 0)
    {
        Serial.println("Server URL not set");
        return -1;
    }

    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("WiFi not connected");
        return -2;
    }

    HTTPClient http;
    String fullUrl = getFullUrl();
    
    Serial.printf("Uploading to: %s\n", fullUrl.c_str());
    Serial.printf("Image size: %d bytes\n", len);

    http.begin(fullUrl);
    http.setTimeout(m_timeout);
    
    // 헤더 설정
    http.addHeader("Content-Type", "image/jpeg");
    http.addHeader("device-id", m_deviceId);
    
    if (m_authToken.length() > 0)
    {
        http.addHeader("auth-token", m_authToken);
    }
    
    if (fileName.length() > 0)
    {
        http.addHeader("file-name", fileName);
    }

    // POST 요청
    int httpCode = http.POST(data, len);

    if (httpCode > 0)
    {
        response = http.getString();
        Serial.printf("HTTP Response code: %d\n", httpCode);
        Serial.println("Response: " + response);
    }
    else
    {
        Serial.printf("HTTP POST failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
    return httpCode;
}

void HttpUploader::parseCmd(std::vector<String> &tokens, JsonDocument &_res_doc)
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

                // [수정] Config 키와 이름을 통일 (server_url, server_path, auth_token, device_id)
                // 기존 짧은 이름(url, path 등)도 호환성을 위해 유지하거나 제거할 수 있습니다.
                // 여기서는 명확한 관리를 위해 Config 키를 우선으로 처리하도록 변경합니다.

                if (key == "server_url" || key == "url") // url은 편의상 허용하되 저장은 server_url 개념
                {
                    setServerUrl(value);
                    _res_doc["result"] = "ok";
                    _res_doc["ms"] = "server url set";
                    _res_doc["server_url"] = value; // 응답 키도 server_url로 통일
                }
                else if (key == "server_path" || key == "path")
                {
                    setUploadPath(value);
                    _res_doc["result"] = "ok";
                    _res_doc["ms"] = "upload path set";
                    _res_doc["server_path"] = value;
                }
                else if (key == "auth_token" || key == "token")
                {
                    setAuthToken(value);
                    _res_doc["result"] = "ok";
                    _res_doc["ms"] = "auth token set";
                }
                else if (key == "device_id" || key == "deviceid" || key == "device")
                {
                    setDeviceId(value);
                    _res_doc["result"] = "ok";
                    _res_doc["ms"] = "device id set";
                    _res_doc["device_id"] = value;
                }
                else if (key == "timeout")
                {
                    setTimeout(value.toInt());
                    _res_doc["result"] = "ok";
                    _res_doc["ms"] = "timeout set";
                }
                else
                {
                    _res_doc["result"] = "fail";
                    _res_doc["ms"] = "unknown key (server_url/server_path/auth_token/device_id/timeout)";
                }
            }
            else
            {
                _res_doc["result"] = "fail";
                _res_doc["ms"] = "need key and value";
            }
        }
        else if (subCmd == "status")
        {
            _res_doc["result"] = "ok";
            _res_doc["server_url"] = m_serverUrl;   // 키 이름 통일
            _res_doc["server_path"] = m_uploadPath; // 키 이름 통일
            _res_doc["fullUrl"] = getFullUrl();
            _res_doc["device_id"] = m_deviceId;     // 키 이름 통일
            _res_doc["auth_token"] = m_authToken;
            _res_doc["timeout"] = m_timeout;
        }
        else
        {
            _res_doc["result"] = "fail";
            _res_doc["ms"] = "unknown sub command (set/status)";
        }
    }
    else
    {
        _res_doc["result"] = "fail";
        _res_doc["ms"] = "need sub command (set/status)";
    }
}