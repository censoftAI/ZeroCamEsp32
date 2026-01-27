#ifndef CAMERA_MODULE_HPP
#define CAMERA_MODULE_HPP

#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>          
#include "esp_camera.h"

// ===========================================
// 카메라 핀 정의 - 보드별 설정
// ===========================================

// Seeed Studio XIAO ESP32S3 Sense
#if defined(CAMERA_MODEL_XIAO_ESP32S3)

#define PWDN_GPIO_NUM     -1
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM     10
#define SIOD_GPIO_NUM     40
#define SIOC_GPIO_NUM     39

#define Y9_GPIO_NUM       48
#define Y8_GPIO_NUM       11
#define Y7_GPIO_NUM       12
#define Y6_GPIO_NUM       14
#define Y5_GPIO_NUM       16
#define Y4_GPIO_NUM       18
#define Y3_GPIO_NUM       17
#define Y2_GPIO_NUM       15

#define VSYNC_GPIO_NUM    38
#define HREF_GPIO_NUM     47
#define PCLK_GPIO_NUM     13

#define FLASH_GPIO_NUM    -1   // XIAO ESP32S3 Sense에는 별도 Flash LED 없음
#define STATUS_LED_PIN    21   // 내장 LED (옵션)

// ESP32-S3 WROOM CAM (Freenove, 디바이스마트 등)
#elif defined(CAMERA_MODEL_ESP32S3_WROOM)

#define PWDN_GPIO_NUM     -1
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM     15
#define SIOD_GPIO_NUM     4
#define SIOC_GPIO_NUM     5

#define Y9_GPIO_NUM       16
#define Y8_GPIO_NUM       17
#define Y7_GPIO_NUM       18
#define Y6_GPIO_NUM       12
#define Y5_GPIO_NUM       10
#define Y4_GPIO_NUM       8
#define Y3_GPIO_NUM       9
#define Y2_GPIO_NUM       11

#define VSYNC_GPIO_NUM    6
#define HREF_GPIO_NUM     7
#define PCLK_GPIO_NUM     13

#define FLASH_GPIO_NUM    -1   // WS2812 LED는 GPIO48 (별도 제어 필요)
#define STATUS_LED_PIN    2    // 내장 LED

// AI-Thinker ESP32-CAM
#elif defined(CAMERA_MODEL_AI_THINKER)

#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5

#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

#define FLASH_GPIO_NUM     4
#define STATUS_LED_PIN    33   // 내장 빨간 LED

// ESP32-WROVER-KIT (Freenove ESP32-Wrover CAM)
#elif defined(CAMERA_MODEL_WROVER_KIT)

#define PWDN_GPIO_NUM    -1
#define RESET_GPIO_NUM   -1
#define XCLK_GPIO_NUM    21
#define SIOD_GPIO_NUM    26
#define SIOC_GPIO_NUM    27

#define Y9_GPIO_NUM      35
#define Y8_GPIO_NUM      34
#define Y7_GPIO_NUM      39
#define Y6_GPIO_NUM      36
#define Y5_GPIO_NUM      19
#define Y4_GPIO_NUM      18
#define Y3_GPIO_NUM       5
#define Y2_GPIO_NUM       4

#define VSYNC_GPIO_NUM   25
#define HREF_GPIO_NUM    23
#define PCLK_GPIO_NUM    22

#define FLASH_GPIO_NUM   -1
#define STATUS_LED_PIN    2

// ESP32-S3-EYE
#elif defined(CAMERA_MODEL_ESP32S3_EYE)

#define PWDN_GPIO_NUM    -1
#define RESET_GPIO_NUM   -1
#define XCLK_GPIO_NUM    15
#define SIOD_GPIO_NUM     4
#define SIOC_GPIO_NUM     5

#define Y9_GPIO_NUM      16
#define Y8_GPIO_NUM      17
#define Y7_GPIO_NUM      18
#define Y6_GPIO_NUM      12
#define Y5_GPIO_NUM      10
#define Y4_GPIO_NUM       8
#define Y3_GPIO_NUM       9
#define Y2_GPIO_NUM      11

#define VSYNC_GPIO_NUM    6
#define HREF_GPIO_NUM     7
#define PCLK_GPIO_NUM    13

#define FLASH_GPIO_NUM   -1
#define STATUS_LED_PIN    3

// 기본값 (AI-Thinker 호환)
#else

#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5

#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

#define FLASH_GPIO_NUM     4
#define STATUS_LED_PIN    33

#endif

class CameraModule
{
private:
    bool m_initialized = false;
    camera_fb_t *m_fb = nullptr;
    framesize_t m_frameSize = FRAMESIZE_VGA;  // 기본 해상도

public:
    CameraModule() {}
    ~CameraModule() 
    {
        if (m_fb)
        {
            esp_camera_fb_return(m_fb);
        }
    }

    bool init();
    bool capture();
    void releaseBuffer();
    
    // Getters
    inline bool isInitialized() const { return m_initialized; }
    inline camera_fb_t* getFrameBuffer() const { return m_fb; }
    inline size_t getImageSize() const { return m_fb ? m_fb->len : 0; }
    inline uint8_t* getImageData() const { return m_fb ? m_fb->buf : nullptr; }
    
    // 해상도 설정
    bool setResolution(framesize_t size);
    bool setResolutionByName(const String& name);
    String getResolutionName() const;
    
    // Flash LED 제어
    void flashOn();
    void flashOff();
    void flashBlink(int times, int delayMs = 100);

    // 커맨드 파싱
    void parseCmd(std::vector<String> &tokens, JsonDocument &_res_doc);
};

#endif // CAMERA_MODULE_HPP
