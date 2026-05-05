# DDU-009 — AWS IoT 연동 (Phase 2)
> **Document ID**: DDU-009  
> **Version**: 1.0  
> **상위 문서**: INC-IMPL-001 §13  
> **의존 DDU**: DDU-006 (AppController 필수)  
> **빌드 플래그**: `-DINCUBATOR_ENABLE_CLOUD`  
> **Codex 작업 시간 예상**: 30~40분

---

## 작업 목표

이 DDU 완료 후:
- WiFi 연결 실패 시에도 Phase 1 기능이 완전 보장된다
- 연결 성공 시 1분 주기로 Telemetry JSON을 AWS IoT에 발행한다
- AWS IoT에서 수신한 명령이 AppController::applyCommand()를 통해 처리된다
- Device Shadow로 현재 상태를 동기화한다

---

## 1. 빌드 조건

```cpp
// 모든 Cloud 코드는 이 ifdef로 감싼다
#ifdef INCUBATOR_ENABLE_CLOUD
    // ... cloud 코드 ...
#endif
```

`platformio.ini`에서 `-DINCUBATOR_ENABLE_CLOUD` 를 추가해야 Phase 2 활성화.  
Phase 1 빌드에는 이 코드가 완전히 제외된다.

---

## 2. 생성할 파일 목록

```
include/cloud/WifiManager.h
src/cloud/WifiManager.cpp

include/cloud/AwsIotClient.h
src/cloud/AwsIotClient.cpp

include/cloud/TelemetryBuilder.h
src/cloud/TelemetryBuilder.cpp

include/cloud/CmdParser.h
src/cloud/CmdParser.cpp
```

---

## 3. 아키텍처 개요

```
ESP32                          AWS IoT Core
  │                                 │
  ├── WiFiManager                   │
  │   └── WiFi.begin() 재시도       │
  │                                 │
  ├── AwsIotClient (MQTT TLS)       │
  │   ├── Publish (1분)  ─────────→ topic: incubator/{batchId}/telemetry
  │   │                                   └─→ IoT Rule → DynamoDB / Lambda
  │   ├── Subscribe ←───────────────── topic: incubator/{batchId}/cmd
  │   │   └── CmdParser → AppController::applyCommand()
  │   └── Shadow ──────────────────── $aws/things/{deviceId}/shadow/...
  │
  └── TelemetryBuilder
      └── RuntimeState → JSON 직렬화
```

---

## 4. WifiManager

```cpp
// include/cloud/WifiManager.h
#pragma once
#include <cstdint>

namespace incubator::cloud
{
    class WifiManager
    {
    public:
        static constexpr uint32_t kRetryIntervalMs = 30000U;  // 30초 재시도

        // SSID/Password는 NVS에서 읽거나 빌드 타임 상수로 설정
        // 개발 단계: AppConfig.h에 정의
        bool init(const char* ssid, const char* password);

        void tick(uint32_t nowMs);

        bool isConnected() const;
        const char* ipAddress() const;

    private:
        uint32_t m_lastRetryMs = 0;
        bool     m_connecting  = false;
    };
}

// src/cloud/WifiManager.cpp 구현 요점:
// init(): WiFi.mode(WIFI_STA); WiFi.begin(ssid, pw);
// tick(): WiFi.status() 확인, 연결 실패 시 kRetryIntervalMs 후 재시도
//         연결 성공 시 state.cloudConnected = true (참조 필요)
//         state.ipAddress = WiFi.localIP().toString()
```

---

## 5. AwsIotClient

### 5.1 헤더

```cpp
// include/cloud/AwsIotClient.h
#pragma once
#include <functional>
#include <cstdint>

namespace incubator::cloud
{
    using CmdCallback = std::function<void(const char* topic,
                                           const char* payload)>;

    class AwsIotClient
    {
    public:
        static constexpr uint32_t kReconnectIntervalMs = 15000U;
        static constexpr int      kMqttPort            = 8883;

        bool init(const char* endpoint,
                  const char* deviceId,
                  const char* rootCaPem,
                  const char* certPem,
                  const char* keyPem);

        void tick(uint32_t nowMs);

        bool publish(const char* topic, const char* json);
        bool isConnected() const { return m_connected; }

        void setCmdCallback(CmdCallback cb) { m_cmdCb = cb; }

    private:
        char     m_deviceId[32]     = {};
        bool     m_connected        = false;
        uint32_t m_lastReconnectMs  = 0;

        CmdCallback m_cmdCb;

        // PubSubClient (knolleary/PubSubClient) 인스턴스
        // TLS 설정: WiFiClientSecure + setCACert / setCertificate / setPrivateKey

        void reconnect();
        static void mqttCallback(const char* topic, uint8_t* payload, unsigned int len);
    };
}
```

### 5.2 인증서 로딩 방법

```cpp
// 방법 1 (개발 권장): platformio.ini embed_files
//   embed_files =
//     src/certs/aws-root-ca.pem
//     src/certs/certificate.pem.crt
//     src/certs/private.pem.key
//
// 코드에서 접근:
//   extern const uint8_t aws_root_ca_pem_start[]  asm("_binary_aws_root_ca_pem_start");
//   extern const uint8_t cert_pem_crt_start[]      asm("_binary_certificate_pem_crt_start");
//   extern const uint8_t private_pem_key_start[]   asm("_binary_private_pem_key_start");
//
// 방법 2 (운용): SPIFFS /certs/ 디렉토리에 저장
```

### 5.3 MQTT 토픽 구조 (필드명 고정)

```
Publish:
  incubator/{batchId}/telemetry    ← Telemetry JSON (1분 주기)

Subscribe:
  incubator/{batchId}/cmd          ← 원격 명령 수신
  $aws/things/{deviceId}/shadow/get/accepted    ← Shadow 응답
  $aws/things/{deviceId}/shadow/update/delta    ← Shadow 변경 통보
```

---

## 6. TelemetryBuilder

```cpp
// include/cloud/TelemetryBuilder.h
#pragma once
#include "domain/RuntimeState.h"
#include "domain/IncubationBatch.h"
#include <cstddef>

namespace incubator::cloud
{
    class TelemetryBuilder
    {
    public:
        // RuntimeState → Telemetry JSON 직렬화
        // buf: 출력 버퍼, bufSize: 버퍼 크기
        // 반환: 직렬화된 JSON 길이 (0 = 실패)
        static size_t build(const domain::RuntimeState& state,
                            const domain::IncubationBatch& batch,
                            char* buf, size_t bufSize);
    };
}
```

**출력 JSON 구조 (필드명 고정 — 서버와 공유)**:

```json
{
  "deviceId": "INC-001",
  "batchId":  "INC-001",
  "ts":       1746000000,
  "day":      7,
  "sensor": {
    "tempC":      37.8,
    "humidityPct":64.2,
    "tempOk":     true,
    "humiOk":     true
  },
  "actuator": {
    "heater":     true,
    "humidifier": false,
    "turner":     false,
    "fanDuty":    60
  },
  "target": {
    "tempC":       37.5,
    "humidityPct": 60.0
  },
  "alarm": {
    "temp": false,
    "humi": false
  },
  "progress": {
    "currentDay": 7,
    "totalDays":  21,
    "lockdown":   false
  }
}
```

> ⚠️ 이 JSON 필드명은 Lambda / DynamoDB / 모바일 앱과 공유됩니다.  
> **절대 변경 금지.** 변경 시 전체 서버 코드 영향.

---

## 7. CmdParser — 원격 명령 파싱

```cpp
// include/cloud/CmdParser.h
#pragma once
#include "app/AppController.h"

namespace incubator::cloud
{
    class CmdParser
    {
    public:
        // MQTT payload JSON → AppController::applyCommand()
        // 반환: 처리 성공 여부
        static bool parse(const char* json,
                          app::AppController& ctrl);
    };
}
```

**수신 명령 JSON 구조**:

```json
{
  "cmd": "PATCH_PLAN_ROW",
  "payload": {
    "day": 8,
    "targetTempC": 37.6,
    "targetHumidityPct": 58.0,
    "turningEnabled": true,
    "turningIntervalMin": 120
  }
}
```

**cmd 문자열 → Cmd 매핑**:

| JSON cmd 문자열 | AppController::Cmd |
|---|---|
| "START_BATCH" | Cmd::StartBatch |
| "STOP_BATCH" | Cmd::StopBatch |
| "PATCH_PLAN_ROW" | Cmd::PatchPlanRow |
| "RESET_PLAN" | Cmd::ResetPlan |
| "UPDATE_SETTINGS" | Cmd::UpdateSettings |
| "CLEAR_SAFE_MODE" | Cmd::ClearSafeMode |

---

## 8. main.cpp 수정 — Phase 2 추가

DDU-008 main.cpp의 지정 위치에 아래를 추가한다:

### 8.1 전역 객체 추가 (g 네임스페이스 내부)

```cpp
#ifdef INCUBATOR_ENABLE_CLOUD
    cloud::WifiManager   wifiMgr;
    cloud::AwsIotClient  awsClient;
#endif
```

### 8.2 globals.h 추가 (extern 선언)

```cpp
#ifdef INCUBATOR_ENABLE_CLOUD
    extern incubator::cloud::WifiManager   wifiMgr;
    extern incubator::cloud::AwsIotClient  awsClient;
#endif
```

### 8.3 setup() 추가

```cpp
#ifdef INCUBATOR_ENABLE_CLOUD
    // WiFi 시작
    g::wifiMgr.init(WIFI_SSID, WIFI_PASSWORD);   // AppConfig.h 에서 정의

    // AWS IoT 초기화 (인증서 embed_files 방식)
    extern const uint8_t aws_root_ca_pem_start[];
    extern const uint8_t cert_pem_crt_start[];
    extern const uint8_t private_pem_key_start[];

    g::awsClient.init(
        AWS_IOT_ENDPOINT,          // AppConfig.h 에서 정의
        INCUBATOR_DEVICE_ID,       // AppConfig.h 에서 정의
        (const char*)aws_root_ca_pem_start,
        (const char*)cert_pem_crt_start,
        (const char*)private_pem_key_start
    );

    // 원격 명령 수신 콜백 등록
    g::awsClient.setCmdCallback(
        [](const char* topic, const char* payload) {
            incubator::cloud::CmdParser::parse(payload, g::appCtrl);
        }
    );
#endif
```

### 8.4 loop() 추가

```cpp
#ifdef INCUBATOR_ENABLE_CLOUD
    g::wifiMgr.tick(now);
    g::awsClient.tick(now);
#endif
```

---

## 9. AppConfig.h 추가 항목 (Phase 2 활성화 시)

```cpp
// include/config/AppConfig.h 에 추가
#ifdef INCUBATOR_ENABLE_CLOUD
    #define WIFI_SSID          "your_ssid"
    #define WIFI_PASSWORD      "your_password"
    #define AWS_IOT_ENDPOINT   "xxxxxxxx.iot.ap-northeast-2.amazonaws.com"
    #define INCUBATOR_DEVICE_ID "INC-001"
#endif
```

> ⚠️ 프로덕션에서는 AppConfig.h에 하드코딩 금지.  
> NVS 또는 SPIFFS `/config.json`에서 읽도록 변경 권장.

---

## 완료 기준 (Acceptance Criteria)

| # | 항목 | 기준 |
|---|---|---|
| AC-1 | Phase 1 격리 | `INCUBATOR_ENABLE_CLOUD` 없이 빌드 성공 |
| AC-2 | WiFi 재연결 | 연결 실패 후 30초마다 자동 재시도 |
| AC-3 | 로컬 동작 | WiFi 연결 실패 시에도 히터/전란 제어 정상 |
| AC-4 | Telemetry | 1분 주기 JSON 발행, AWS IoT 콘솔에서 수신 확인 |
| AC-5 | 원격 명령 | PATCH_PLAN_ROW 수신 → plan 즉시 갱신 확인 |
| AC-6 | 단일 통로 | 원격 명령도 AppController::applyCommand() 경유 |
| AC-7 | JSON 필드명 | TelemetryBuilder 출력이 §6 JSON 구조와 일치 |
| AC-8 | 인증서 | embed_files 방식으로 flash에 포함, 외부 노출 없음 |
