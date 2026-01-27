#include <Arduino.h>
#include <WiFi.h>
#include <TaskScheduler.h>
#include <ArduinoJson.h>

#include "config.hpp"
#include "camera_module.hpp"
#include "wifi_module.hpp"
#include "http_upload.hpp"
#include "etc.hpp"

// 전역 객체
Scheduler g_ts;
Config g_config;
CameraModule g_camera;
WifiModule g_wifi;
HttpUploader g_uploader;

// 외부 함수 선언
extern String parseCmd(String _strLine);
extern void loadSettingsToModules();

// LED 핀 설정 (camera_module.hpp에서 정의됨)
#ifndef STATUS_LED_PIN
    #if defined(CAMERA_MODEL_XIAO_ESP32S3)
        #define STATUS_LED_PIN 21
    #elif defined(CAMERA_MODEL_ESP32S3_WROOM) || defined(CAMERA_MODEL_ESP32S3_EYE)
        #define STATUS_LED_PIN 2
    #elif defined(CAMERA_MODEL_AI_THINKER)
        #define STATUS_LED_PIN 33
    #else
        #define STATUS_LED_PIN 2
    #endif
#endif

// LED 상태 (active low or active high)
#if defined(CAMERA_MODEL_AI_THINKER)
    #define LED_ON  LOW
    #define LED_OFF HIGH
// [추가됨] XIAO ESP32S3는 Active Low (LOW일 때 켜짐)
#elif defined(CAMERA_MODEL_XIAO_ESP32S3)
    #define LED_ON  LOW
    #define LED_OFF HIGH
#else
    // 그 외 보드 (기본값)
    #define LED_ON  HIGH
    #define LED_OFF LOW
#endif


// // LED 블링크 태스크
// Task task_LedBlink(500, TASK_FOREVER, []()
// {
//     static bool ledState = false;
//     ledState = !ledState;
//     digitalWrite(STATUS_LED_PIN, ledState ? LED_ON : LED_OFF);
// }, &g_ts, false);  // 기본 비활성화

// [수정된 태스크 코드]
Task task_LedBlink(500, TASK_FOREVER, []()
{
    // WiFi가 연결되어 있다면 LED를 계속 켜둠 (Solid ON)
    if (g_wifi.isConnected()) 
    {
        digitalWrite(STATUS_LED_PIN, LED_ON);
    }
    // WiFi가 미연결 상태라면 깜빡임 (Blink)
    else 
    {
        static bool ledState = false;
        ledState = !ledState;
        digitalWrite(STATUS_LED_PIN, ledState ? LED_ON : LED_OFF);
    }
}, &g_ts, false);

// 시리얼 커맨드 처리 태스크
Task task_Cmd(100, TASK_FOREVER, []()
{
    if (Serial.available() > 0)
    {
        String _strLine = Serial.readStringUntil('\n');
        _strLine.trim();
        
        if (_strLine.length() > 0)
        {
            Serial.println(parseCmd(_strLine));
        }
    }
}, &g_ts, true);

// 자동 업로드 태스크 (설정된 경우)
Task task_AutoUpload(60000, TASK_FOREVER, []()
{
    if (!g_camera.isInitialized() || !g_wifi.isConnected())
    {
        return;
    }

    int autoUpload = g_config.get<int>("auto_upload", 0);
    if (autoUpload != 1)
    {
        return;
    }

    Serial.println("Auto upload triggered");
    
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

        String response;
        int httpCode = g_uploader.uploadImage(
            g_camera.getImageData(),
            g_camera.getImageSize(),
            response
        );

        g_camera.releaseBuffer();

        if (httpCode == 200 || httpCode == 201)
        {
            Serial.println("Auto upload success");
        }
        else
        {
            Serial.printf("Auto upload failed: %d\n", httpCode);
        }
    }
    else
    {
        if (useFlash)
        {
            g_camera.flashOff();
        }
        Serial.println("Auto capture failed");
    }
}, &g_ts, false);

void setup()
{
    // 상태 LED 초기화
    pinMode(STATUS_LED_PIN, OUTPUT);
    digitalWrite(STATUS_LED_PIN, LED_OFF);

    // 시리얼 초기화
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    
    delay(500);
    
    Serial.println();
    Serial.println(":-]");
    Serial.println("========================================");
#if defined(CAMERA_MODEL_XIAO_ESP32S3)
    Serial.println("XIAO ESP32S3 Sense CAM Uploader Starting...");
#elif defined(CAMERA_MODEL_ESP32S3_WROOM)
    Serial.println("ESP32-S3 WROOM CAM Uploader Starting...");
#elif defined(CAMERA_MODEL_AI_THINKER)
    Serial.println("ESP32-CAM Uploader Starting...");
#else
    Serial.println("ESP32 CAM Uploader Starting...");
#endif
    Serial.println("========================================");

    // 메모리 정보
    printHeapInfo();
    Serial.printf("Chip ID: %s\n", getChipID().c_str());
    Serial.printf("Config System Revision: %d\n", Config::SystemVersion);

    // 칩 정보 출력
    Serial.printf("Chip Model: %s\n", ESP.getChipModel());
    Serial.printf("Chip Revision: %d\n", ESP.getChipRevision());
    Serial.printf("CPU Freq: %d MHz\n", ESP.getCpuFreqMHz());

    // 설정 로드
    g_config.load();
    loadSettingsToModules();

    // 카메라 초기화
    Serial.println("Initializing camera...");
    if (g_camera.init())
    {
        Serial.println("Camera OK");
        
        // 저장된 해상도 적용
        if (g_config.hasKey("resolution"))
        {
            String res = g_config.get<String>("resolution");
            g_camera.setResolutionByName(res);
        }
    }
    else
    {
        Serial.println("Camera FAILED");
    }

    // 자동 WiFi 연결
    int autoConnect = g_config.get<int>("auto_connect", 0);
    if (autoConnect == 1 && g_wifi.getSSID().length() > 0)
    {
        Serial.println("Auto connecting WiFi...");
        if (g_wifi.connect())
        {
            Serial.printf("Connected! IP: %s\n", g_wifi.getIP().c_str());
        }
    }

    // 자동 업로드 태스크 설정
    int autoUploadInterval = g_config.get<int>("upload_interval", 60);
    task_AutoUpload.setInterval(autoUploadInterval * 1000);
    
    int autoUpload = g_config.get<int>("auto_upload", 0);
    if (autoUpload == 1)
    {
        task_AutoUpload.enable();
        Serial.printf("Auto upload enabled (interval: %ds)\n", autoUploadInterval);
    }

    // 상태 LED 블링크 시작
    task_LedBlink.enable();

    Serial.println("========================================");
    Serial.println("Ready! Type 'help' for commands");
    Serial.println("========================================");
    
    // 태스크 스케줄러 시작
    g_ts.startNow();
}

void loop()
{
    g_ts.execute();
}
