#ifndef HTTP_UPLOAD_HPP
#define HTTP_UPLOAD_HPP

#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <vector>

class HttpUploader
{
private:
    String m_serverUrl;     // 예: http://192.168.1.100:8080
    String m_uploadPath;    // 예: /api/v1/camera/upload
    String m_authToken;     // 인증 토큰
    String m_deviceId;      // 디바이스 ID
    int m_timeout = 30000;  // 30초 타임아웃

public:
    HttpUploader() 
    {
        m_uploadPath = "/api/v1/camera/upload";
        m_deviceId = "";
    }
    ~HttpUploader() {}

    // 설정
    inline void setServerUrl(const String& url) { m_serverUrl = url; }
    inline void setUploadPath(const String& path) { m_uploadPath = path; }
    inline void setAuthToken(const String& token) { m_authToken = token; }
    inline void setDeviceId(const String& id) { m_deviceId = id; }
    inline void setTimeout(int timeout) { m_timeout = timeout; }

    // Getters
    inline String getServerUrl() const { return m_serverUrl; }
    inline String getUploadPath() const { return m_uploadPath; }
    inline String getAuthToken() const { return m_authToken; }
    inline String getDeviceId() const { return m_deviceId; }
    inline String getFullUrl() const { return m_serverUrl + m_uploadPath; }

    // 업로드
    int uploadImage(uint8_t* data, size_t len, const String& fileName = "");
    int uploadImage(uint8_t* data, size_t len, String& response, const String& fileName = "");

    // 커맨드 파싱
    void parseCmd(std::vector<String> &tokens, JsonDocument &_res_doc);
};

#endif // HTTP_UPLOAD_HPP
