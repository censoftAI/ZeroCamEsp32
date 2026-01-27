# ESP32-S3 CAM Uploader

ESP32-S3 기반 카메라 보드를 지원하는 이미지 업로드 펌웨어입니다.

## 지원 보드

| 환경 이름 | 보드 | 설명 |
|----------|------|------|
| `xiao_esp32s3_sense` | **Seeed XIAO ESP32S3 Sense** | Seeed Studio 제품, 8MB Flash + 8MB PSRAM |
| `esp32s3cam` | ESP32-S3 WROOM CAM (N8R8) | Freenove, 디바이스마트 등 8MB Flash + 8MB PSRAM |
| `esp32s3cam_n16r8` | ESP32-S3 WROOM CAM (N16R8) | 16MB Flash + 8MB PSRAM |
| `esp32cam` | ESP32-CAM AI-Thinker | 기존 ESP32-CAM 보드 |
| `esp32wrover_cam` | ESP32-WROVER CAM | Freenove ESP32-Wrover |
| `esp32s3eye` | ESP32-S3-EYE | Espressif 개발 보드 |

## 빌드 방법

### PlatformIO CLI

```bash
# XIAO ESP32S3 Sense 빌드 (Seeed Studio)
pio run -e xiao_esp32s3_sense

# ESP32-S3 WROOM CAM 빌드
pio run -e esp32s3cam

# ESP32-CAM AI-Thinker 빌드
pio run -e esp32cam

# 업로드
pio run -e xiao_esp32s3_sense -t upload
```

### VS Code + PlatformIO Extension

1. VS Code에서 프로젝트 폴더 열기
2. PlatformIO 아이콘 클릭
3. 원하는 환경 선택 후 Build/Upload

## 핀 배치

### Seeed XIAO ESP32S3 Sense

| 기능 | GPIO |
|------|------|
| XCLK | 10 |
| SIOD (SDA) | 40 |
| SIOC (SCL) | 39 |
| D0-D7 | 15, 17, 18, 16, 14, 12, 11, 48 |
| VSYNC | 38 |
| HREF | 47 |
| PCLK | 13 |
| 내장 LED | 21 |

### ESP32-S3 WROOM CAM (Freenove)

| 기능 | GPIO |
|------|------|
| XCLK | 15 |
| SIOD (SDA) | 4 |
| SIOC (SCL) | 5 |
| D0-D7 | 11, 9, 8, 10, 12, 18, 17, 16 |
| VSYNC | 6 |
| HREF | 7 |
| PCLK | 13 |
| 내장 LED | 2 |
| WS2812 LED | 48 |

### ESP32-CAM AI-Thinker

| 기능 | GPIO |
|------|------|
| PWDN | 32 |
| XCLK | 0 |
| SIOD (SDA) | 26 |
| SIOC (SCL) | 27 |
| D0-D7 | 5, 18, 19, 21, 36, 39, 34, 35 |
| VSYNC | 25 |
| HREF | 23 |
| PCLK | 22 |
| Flash LED | 4 |
| 내장 LED | 33 |

## 시리얼 명령어

### 기본 명령어

```
about       - 시스템 정보
reboot      - 재부팅
heap        - 메모리 정보
help        - 도움말
```

### 카메라 명령어

```
camera init              - 카메라 초기화
camera capture           - 이미지 캡처
camera status            - 카메라 상태
camera resolution <name> - 해상도 설정 (QQVGA~UXGA)
camera flash on/off/blink - 플래시 제어
```

### WiFi 명령어

```
wifi set ssid <name>     - SSID 설정
wifi set password <pass> - 비밀번호 설정
wifi connect             - 연결
wifi disconnect          - 연결 해제
wifi status              - 상태 확인
wifi scan                - 네트워크 스캔
```

### 서버 명령어

```
server set url <url>     - 서버 URL 설정
server set path <path>   - 업로드 경로 설정
server set token <token> - 인증 토큰 설정
server status            - 상태 확인
```

### 업로드 명령어

```
upload [filename]        - 캡처 후 업로드
saveall                  - 모든 설정 저장
autoconnect              - 저장된 설정으로 자동 연결
```

### 설정 명령어

```
config load              - 설정 로드
config save              - 설정 저장
config dump              - 설정 출력
config clear             - 설정 초기화
config set <key> <value> - 값 설정
config get <key>         - 값 조회
```

## 설정 키

| 키 | 설명 |
|----|------|
| `wifi_ssid` | WiFi SSID |
| `wifi_pass` | WiFi 비밀번호 |
| `server_url` | 서버 URL (예: http://192.168.1.100:8080) |
| `server_path` | 업로드 경로 (예: /api/v1/camera/upload) |
| `auth_token` | 인증 토큰 |
| `device_id` | 디바이스 ID |
| `resolution` | 해상도 (VGA, SVGA, XGA 등) |
| `auto_connect` | 자동 WiFi 연결 (0/1) |
| `auto_upload` | 자동 업로드 (0/1) |
| `upload_interval` | 업로드 간격 (초) |
| `use_flash` | 플래시 사용 (0/1) |

## 예제 사용법

```bash
# WiFi 설정 및 연결
wifi set ssid MyNetwork
wifi set password MyPassword
wifi connect
saveall

# 서버 설정
server set url http://192.168.1.100:8080
server set path /api/upload
saveall

# 수동 업로드
upload

# 자동 업로드 설정 (60초 간격)
config set auto_upload 1
config set upload_interval 60
```

## 주의사항

1. **XIAO ESP32S3 Sense 사용 시**:
   - Arduino IDE에서 PSRAM 옵션을 반드시 활성화하세요
   - SD카드와 Round Display를 함께 사용할 경우 J3 점퍼를 자르거나 소프트웨어 설정이 필요합니다
   - 참고: https://wiki.seeedstudio.com/xiao_esp32s3_camera_usage/

2. ESP32-S3의 경우 PSRAM이 Octal SPI 모드로 동작합니다. `platformio.ini`의 `board_build.arduino.memory_type = qio_opi` 설정이 필수입니다.

3. ESP32-S3 WROOM CAM의 Flash LED는 WS2812 RGB LED (GPIO 48)입니다. 일반 GPIO로는 제어할 수 없으며, NeoPixel 라이브러리가 필요합니다.

4. 카메라 초기화 실패 시 PSRAM 활성화 여부를 확인하세요.

## 라이선스

MIT License
