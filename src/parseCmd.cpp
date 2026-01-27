#include <Arduino.h>
#include <vector>

#include "tonkey.hpp"
#include "config.hpp"
#include "camera_module.hpp"
#include "wifi_module.hpp"
#include "http_upload.hpp"

#include "etc.hpp"

tonkey g_MainParser;

extern Config g_config;
extern CameraModule g_camera;
extern WifiModule g_wifi;
extern HttpUploader g_uploader;

// 설정값들을 모듈에 로드
void loadSettingsToModules()
{
    // WiFi 설정 로드
    if (g_config.hasKey("ssid"))
    {
        g_wifi.setSSID(g_config.get<String>("ssid"));
    }
    if (g_config.hasKey("password"))
    {
        g_wifi.setPassword(g_config.get<String>("password"));
    }

    // 서버 설정 로드
    if (g_config.hasKey("server_url"))
    {
        g_uploader.setServerUrl(g_config.get<String>("server_url"));
    }
    
    if (g_config.hasKey("server_path"))
    {
        g_uploader.setUploadPath(g_config.get<String>("server_path"));
    }
    
    if (g_config.hasKey("auth_token"))
    {
        g_uploader.setAuthToken(g_config.get<String>("auth_token"));
    }

    if (g_config.hasKey("device_id"))
    {
        g_uploader.setDeviceId(g_config.get<String>("device_id"));
    }
    else {
        // 기본 디바이스 ID 설정
        g_uploader.setDeviceId("esp32cam" + getChipID());
    }
}

// 모듈 설정값을 Config에 저장
void saveSettingsFromModules()
{
    // WiFi 설정
    if (g_wifi.getSSID().length() > 0)
    {
        g_config.set("ssid", g_wifi.getSSID());
    }
    if (g_wifi.getPassword().length() > 0)
    {
        g_config.set("password", g_wifi.getPassword());
    }

    // 서버 설정
    if (g_uploader.getServerUrl().length() > 0)
    {
        g_config.set("server_url", g_uploader.getServerUrl());
    }
    if (g_uploader.getUploadPath().length() > 0)
    {
        g_config.set("server_path", g_uploader.getUploadPath());
    }
    if (g_uploader.getAuthToken().length() > 0)
    {
        g_config.set("auth_token", g_uploader.getAuthToken());
    }
    if (g_uploader.getDeviceId().length() > 0)
    {
        g_config.set("device_id", g_uploader.getDeviceId());
    }
}

String parseCmd(String _strLine)
{
    JsonDocument _res_doc;

    g_MainParser.parse(_strLine);
    if (g_MainParser.getTokenCount() > 0)
    {
        String cmd = g_MainParser.getToken(0);

        // 토큰들을 벡터로 변환
        std::vector<String> tokens;
        for (int i = 0; i < g_MainParser.getTokenCount(); i++)
        {
            tokens.push_back(g_MainParser.getToken(i));
        }

        if (cmd == "about")
        {
            _res_doc["result"] = "ok";
            _res_doc["os"] = "cronos-v1";
            _res_doc["app"] = "esp32cam-uploader";
            _res_doc["version"] = "1.0.0";
            _res_doc["author"] = "gbox3d";
            _res_doc["chipid"] = (uint32_t)(ESP.getEfuseMac() & 0xFFFFFFFF);
            _res_doc["psram"] = psramFound();
        }
        else if (cmd == "reboot")
        {
            _res_doc["result"] = "ok";
            _res_doc["ms"] = "rebooting...";
            String response;
            serializeJson(_res_doc, response);
            Serial.println(response);
            delay(100);
            ESP.restart();
        }
        else if (cmd == "heap")
        {
            _res_doc["result"] = "ok";
            _res_doc["free_heap"] = ESP.getFreeHeap();
            if (psramFound())
            {
                _res_doc["psram_size"] = ESP.getPsramSize();
                _res_doc["psram_free"] = ESP.getFreePsram();
            }
        }
        else if (cmd == "config")
        {
            g_config.parseCmd(tokens, _res_doc);
        }
        else if (cmd == "wifi")
        {
            g_wifi.parseCmd(tokens, _res_doc);
        }
        else if (cmd == "camera" || cmd == "cam")
        {
            g_camera.parseCmd(tokens, _res_doc);
        }
        else if (cmd == "server")
        {
            g_uploader.parseCmd(tokens, _res_doc);
        }
        else if (cmd == "upload")
        {
            // 단축 명령: upload [filename]
            // 카메라 캡처 후 바로 업로드
            if (!g_camera.isInitialized())
            {
                _res_doc["result"] = "fail";
                _res_doc["ms"] = "camera not initialized";
            }
            else if (!g_wifi.isConnected())
            {
                _res_doc["result"] = "fail";
                _res_doc["ms"] = "wifi not connected";
            }
            else if (g_uploader.getServerUrl().length() == 0)
            {
                _res_doc["result"] = "fail";
                _res_doc["ms"] = "server url not set";
            }
            else
            {
                // 플래시 켜고 캡처
                bool useFlash = g_config.get<int>("use_flash", 0) == 1;
                if (useFlash)
                {
                    g_camera.flashOn();
                    delay(100);
                }

                if (g_camera.capture())
                {
                    if (useFlash)
                    {
                        g_camera.flashOff();
                    }

                    String fileName = "";
                    if (tokens.size() > 1)
                    {
                        fileName = tokens[1];
                    }

                    String response;
                    int httpCode = g_uploader.uploadImage(
                        g_camera.getImageData(),
                        g_camera.getImageSize(),
                        response,
                        fileName
                    );

                    g_camera.releaseBuffer();

                    if (httpCode == 200 || httpCode == 201)
                    {
                        _res_doc["result"] = "ok";
                        _res_doc["ms"] = "uploaded";
                        _res_doc["httpCode"] = httpCode;
                        
                        // 서버 응답 파싱 시도
                        JsonDocument serverRes;
                        if (deserializeJson(serverRes, response) == DeserializationError::Ok)
                        {
                            _res_doc["server"] = serverRes;
                        }
                        else
                        {
                            _res_doc["serverResponse"] = response;
                        }
                    }
                    else
                    {
                        _res_doc["result"] = "fail";
                        _res_doc["ms"] = "upload failed";
                        _res_doc["httpCode"] = httpCode;
                    }
                }
                else
                {
                    if (useFlash)
                    {
                        g_camera.flashOff();
                    }
                    _res_doc["result"] = "fail";
                    _res_doc["ms"] = "capture failed";
                }
            }
        }
        else if (cmd == "saveall")
        {
            // 모든 설정 저장
            saveSettingsFromModules();
            _res_doc["result"] = "ok";
            _res_doc["ms"] = "all settings saved";
        }
        else if (cmd == "autoconnect")
        {
            // 저장된 WiFi 설정으로 자동 연결
            loadSettingsToModules();
            if (g_wifi.connect())
            {
                _res_doc["result"] = "ok";
                _res_doc["ms"] = "auto connected";
                _res_doc["ip"] = g_wifi.getIP();
            }
            else
            {
                _res_doc["result"] = "fail";
                _res_doc["ms"] = "auto connect failed";
            }
        }
        else if (cmd == "help")
        {
            _res_doc["result"] = "ok";
            _res_doc["commands"] = "about,reboot,heap,config,wifi,camera,server,upload,saveall,autoconnect,help";
            _res_doc["config"] = "load/save/dump/clear/set/get";
            _res_doc["wifi"] = "set ssid/password, connect, disconnect, status, scan";
            _res_doc["camera"] = "init, capture, status, resolution, flash on/off/blink";
            _res_doc["server"] = "set url/path/token/deviceid/timeout, status";
            _res_doc["upload"] = "capture and upload (shortcut)";
        }
        else
        {
            _res_doc["result"] = "fail";
            _res_doc["ms"] = "unknown command";
        }
    }
    else
    {
        _res_doc["result"] = "fail";
        _res_doc["ms"] = "need command";
    }

    String response;
    serializeJson(_res_doc, response);
    return response;
}
