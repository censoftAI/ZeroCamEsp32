#include "camera_module.hpp"

bool CameraModule::init()
{
    camera_config_t config;
    
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    config.frame_size = m_frameSize;
    config.jpeg_quality = 12;  // 0-63, 낮을수록 고품질
    config.fb_count = 1;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;

    // PSRAM이 있으면 고해상도 사용
    if (psramFound())
    {
        Serial.printf("PSRAM found: %d bytes\n", ESP.getPsramSize());
        config.jpeg_quality = 10;
        config.fb_count = 2;
        config.grab_mode = CAMERA_GRAB_LATEST;
    }
    else
    {
        // PSRAM 없으면 저해상도로 제한
        Serial.println("No PSRAM found, using DRAM");
        config.frame_size = FRAMESIZE_SVGA;
        config.fb_location = CAMERA_FB_IN_DRAM;
    }

    // 카메라 초기화
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK)
    {
        Serial.printf("Camera init failed with error 0x%x\n", err);
        m_initialized = false;
        return false;
    }

    // 센서 설정
    sensor_t *s = esp_camera_sensor_get();
    if (s)
    {
        // 센서 정보 출력
        Serial.printf("Camera PID: 0x%02X\n", s->id.PID);
        
        s->set_brightness(s, 0);     // -2 to 2
        s->set_contrast(s, 0);       // -2 to 2
        s->set_saturation(s, 0);     // -2 to 2
        s->set_special_effect(s, 0); // 0: No Effect
        s->set_whitebal(s, 1);       // 0: disable, 1: enable
        s->set_awb_gain(s, 1);       // 0: disable, 1: enable
        s->set_wb_mode(s, 0);        // 0: Auto
        s->set_exposure_ctrl(s, 1);  // 0: disable, 1: enable
        s->set_aec2(s, 0);           // 0: disable, 1: enable
        s->set_ae_level(s, 0);       // -2 to 2
        s->set_aec_value(s, 300);    // 0 to 1200
        s->set_gain_ctrl(s, 1);      // 0: disable, 1: enable
        s->set_agc_gain(s, 0);       // 0 to 30
        s->set_gainceiling(s, (gainceiling_t)0);
        s->set_bpc(s, 0);            // 0: disable, 1: enable
        s->set_wpc(s, 1);            // 0: disable, 1: enable
        s->set_raw_gma(s, 1);        // 0: disable, 1: enable
        s->set_lenc(s, 1);           // 0: disable, 1: enable
        s->set_hmirror(s, 0);        // 0: disable, 1: enable
        s->set_vflip(s, 0);          // 0: disable, 1: enable
        s->set_dcw(s, 1);            // 0: disable, 1: enable
        s->set_colorbar(s, 0);       // 0: disable, 1: enable
    }

    // Flash LED 핀 설정
#if FLASH_GPIO_NUM >= 0
    pinMode(FLASH_GPIO_NUM, OUTPUT);
    digitalWrite(FLASH_GPIO_NUM, LOW);
#endif

    m_initialized = true;
    Serial.println("Camera initialized successfully");
    return true;
}

bool CameraModule::capture()
{
    if (!m_initialized)
    {
        Serial.println("Camera not initialized");
        return false;
    }

    // 이전 프레임 해제
    if (m_fb)
    {
        esp_camera_fb_return(m_fb);
        m_fb = nullptr;
    }

    // 새 프레임 캡처
    m_fb = esp_camera_fb_get();
    if (!m_fb)
    {
        Serial.println("Camera capture failed");
        return false;
    }

    Serial.printf("Captured image: %d bytes\n", m_fb->len);
    return true;
}

void CameraModule::releaseBuffer()
{
    if (m_fb)
    {
        esp_camera_fb_return(m_fb);
        m_fb = nullptr;
    }
}

bool CameraModule::setResolution(framesize_t size)
{
    sensor_t *s = esp_camera_sensor_get();
    if (!s)
    {
        return false;
    }

    if (s->set_framesize(s, size) == 0)
    {
        m_frameSize = size;
        return true;
    }
    return false;
}

bool CameraModule::setResolutionByName(const String& name)
{
    framesize_t size;
    
    if (name == "QQVGA" || name == "qqvga")
        size = FRAMESIZE_QQVGA;    // 160x120
    else if (name == "QCIF" || name == "qcif")
        size = FRAMESIZE_QCIF;     // 176x144
    else if (name == "HQVGA" || name == "hqvga")
        size = FRAMESIZE_HQVGA;    // 240x176
    else if (name == "QVGA" || name == "qvga")
        size = FRAMESIZE_QVGA;     // 320x240
    else if (name == "CIF" || name == "cif")
        size = FRAMESIZE_CIF;      // 400x296
    else if (name == "VGA" || name == "vga")
        size = FRAMESIZE_VGA;      // 640x480
    else if (name == "SVGA" || name == "svga")
        size = FRAMESIZE_SVGA;     // 800x600
    else if (name == "XGA" || name == "xga")
        size = FRAMESIZE_XGA;      // 1024x768
    else if (name == "SXGA" || name == "sxga")
        size = FRAMESIZE_SXGA;     // 1280x1024
    else if (name == "UXGA" || name == "uxga")
        size = FRAMESIZE_UXGA;     // 1600x1200
    else
        return false;

    return setResolution(size);
}

String CameraModule::getResolutionName() const
{
    switch (m_frameSize)
    {
        case FRAMESIZE_QQVGA: return "QQVGA(160x120)";
        case FRAMESIZE_QCIF:  return "QCIF(176x144)";
        case FRAMESIZE_HQVGA: return "HQVGA(240x176)";
        case FRAMESIZE_QVGA:  return "QVGA(320x240)";
        case FRAMESIZE_CIF:   return "CIF(400x296)";
        case FRAMESIZE_VGA:   return "VGA(640x480)";
        case FRAMESIZE_SVGA:  return "SVGA(800x600)";
        case FRAMESIZE_XGA:   return "XGA(1024x768)";
        case FRAMESIZE_SXGA:  return "SXGA(1280x1024)";
        case FRAMESIZE_UXGA:  return "UXGA(1600x1200)";
        default: return "Unknown";
    }
}

void CameraModule::flashOn()
{
#if FLASH_GPIO_NUM >= 0
    digitalWrite(FLASH_GPIO_NUM, HIGH);
#endif
}

void CameraModule::flashOff()
{
#if FLASH_GPIO_NUM >= 0
    digitalWrite(FLASH_GPIO_NUM, LOW);
#endif
}

void CameraModule::flashBlink(int times, int delayMs)
{
#if FLASH_GPIO_NUM >= 0
    for (int i = 0; i < times; i++)
    {
        flashOn();
        delay(delayMs);
        flashOff();
        if (i < times - 1)
            delay(delayMs);
    }
#endif
}

void CameraModule::parseCmd(std::vector<String> &tokens, JsonDocument &_res_doc)
{
    int _tokenCount = tokens.size();

    if (_tokenCount > 1)
    {
        String subCmd = tokens[1];

        if (subCmd == "init")
        {
            if (init())
            {
                _res_doc["result"] = "ok";
                _res_doc["ms"] = "camera initialized";
            }
            else
            {
                _res_doc["result"] = "fail";
                _res_doc["ms"] = "camera init failed";
            }
        }
        else if (subCmd == "capture")
        {
            if (capture())
            {
                _res_doc["result"] = "ok";
                _res_doc["ms"] = "captured";
                _res_doc["size"] = (unsigned long)getImageSize();
            }
            else
            {
                _res_doc["result"] = "fail";
                _res_doc["ms"] = "capture failed";
            }
        }
        else if (subCmd == "status")
        {
            _res_doc["result"] = "ok";
            _res_doc["initialized"] = m_initialized;
            _res_doc["resolution"] = getResolutionName();
            _res_doc["psram"] = psramFound();
            if (psramFound())
            {
                _res_doc["psram_size"] = ESP.getPsramSize();
                _res_doc["psram_free"] = ESP.getFreePsram();
            }
        }
        else if (subCmd == "resolution")
        {
            if (_tokenCount > 2)
            {
                String res = tokens[2];
                if (setResolutionByName(res))
                {
                    _res_doc["result"] = "ok";
                    _res_doc["ms"] = "resolution set";
                    _res_doc["resolution"] = getResolutionName();
                }
                else
                {
                    _res_doc["result"] = "fail";
                    _res_doc["ms"] = "invalid resolution";
                    _res_doc["available"] = "QQVGA,QCIF,HQVGA,QVGA,CIF,VGA,SVGA,XGA,SXGA,UXGA";
                }
            }
            else
            {
                _res_doc["result"] = "ok";
                _res_doc["resolution"] = getResolutionName();
            }
        }
        else if (subCmd == "flash")
        {
            if (_tokenCount > 2)
            {
                String action = tokens[2];
                if (action == "on")
                {
                    flashOn();
                    _res_doc["result"] = "ok";
                    _res_doc["ms"] = "flash on";
                }
                else if (action == "off")
                {
                    flashOff();
                    _res_doc["result"] = "ok";
                    _res_doc["ms"] = "flash off";
                }
                else if (action == "blink")
                {
                    int times = (_tokenCount > 3) ? tokens[3].toInt() : 3;
                    flashBlink(times);
                    _res_doc["result"] = "ok";
                    _res_doc["ms"] = "flash blinked";
                }
                else
                {
                    _res_doc["result"] = "fail";
                    _res_doc["ms"] = "unknown flash command (on/off/blink)";
                }
            }
            else
            {
                _res_doc["result"] = "fail";
                _res_doc["ms"] = "need flash command (on/off/blink)";
            }
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
        _res_doc["ms"] = "need sub command (init/capture/status/resolution/flash)";
    }
}
