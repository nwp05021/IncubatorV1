# DDU-PROV-001—BLE Provisioning + QR 화면 + 로컬 모드 Fallback
> **Document ID**: DDU-PROV-001  
> **Version**: 1.0 | **Status**: Authoritative  
> **상위 문서**: 22_Incubator_DetailDesign.md (Incubator-DETAIL-001)  
> **Phase**: Phase 2 (`-D Incubator_ENABLE_PROVISIONING`)  
> **대상 AI**: Cursor  
> **목적**: ESP-IDF BLE Provisioning 구현 + QR 코드 화면 + 타임아웃 후 로컬 모드 진입까지  
>           단일 DDU 범위로 Cursor에게 완전한 구현 지시서를 제공한다.
>
> ⚠️ **Cursor 필독 — 최우선 규칙**
> - FwCore 계층 분리 절대 준수: Device는 하드웨어만, Module은 상태/Event만,
>   Controller(Policy)는 정책 조합만 담당한다.
> - `ProvisioningService`는 FwCore `IModule` 기반 **상태 머신 + non-blocking** 구조.
>   blocking retry loop 절대 금지.
> - WiFi 인증정보 NVS 저장/로드는 `AppSettings` 경유. Device 직접 접근 금지.
> - Event 없는 silent failure 금지 — 모든 상태 전환은 EventBus로 발행.
> - Tick 내 malloc/new/vTaskDelay 금지.
> - `incubator::` 네임스페이스 전용, FwCore 예약 sourceId(1~99) 사용 금지.

---

## 목차

1. [범위 및 전제 조건](#1-범위-및-전제-조건)
2. [동작 요건 확정](#2-동작-요건-확정)
3. [상태 머신 설계](#3-상태-머신-설계)
4. [AppSettings WiFi 필드](#4-appsettings-wifi-필드)
5. [ProductEventCode 추가](#5-producteventcode-추가)
6. [ProvisioningManager 구현 명세](#6-provisioningmanager-구현-명세)
7. [UiModel Provisioning 필드](#7-uimodel-provisioning-필드)
8. [QR 화면 렌더러 명세 (ProvisioningRenderer)](#8-qr-화면-렌더러-명세-provisioningrenderer)
9. [UiController 연동](#9-uicontroller-연동)
10. [main.cpp / RegisterServices 연동](#10-maincpp--registerservices-연동)
11. [sdkconfig 필수 항목](#11-sdkconfig-필수-항목)
12. [파일 목록 및 책임 요약](#12-파일-목록-및-책임-요약)
13. [구현 순서 (Cursor 작업 순서)](#13-구현-순서-cursor-작업-순서)
14. [완료 기준 (Acceptance Criteria)](#14-완료-기준-acceptance-criteria)

---

## 1. 범위 및 전제 조건

### 1.1 이 DDU의 범위

| 항목 | 포함 | 비고 |
|---|---|---|
| AppSettings WiFi 필드 추가 | ✅ | §4 |
| ProductEventCode Provisioning 추가 | ✅ | §5 |
| ProvisioningManager (상태 머신 + BLE) | ✅ | §6 — 핵심 |
| QR 코드 화면 (ProvisioningRenderer) | ✅ | §8 |
| UiModel Provisioning 필드 추가 | ✅ | §7 |
| UiController 연동 (Event 수신 + 페이지 전환) | ✅ | §9 |
| RegisterServices / main.cpp 연동 | ✅ | §10 |
| CloudClient / AWS IoT 연결 | ❌ | 별도 DDU |
| WiFi 재연결 재시도 로직 | ❌ | 별도 DDU |
| BLE 스택 상세 (esp_ble_gap 등) | ✅ | §6.5에서 핵심 API 명시 |

### 1.2 전제 — 이미 구현된 것

```
✅ AppSettings              (include/incubator/domain/model/AppSettings.h)
✅ LocalPlanRepository      (WiFi 필드 추가 전 버전)
✅ UiModel                  (include/incubator/ui/UiModel.h — Provisioning 필드 미포함)
✅ UiController             (src/ui/UiController.cpp)
✅ MainUiRenderer           (src/ui/MainUiRenderer.cpp)
✅ St7789DisplayDevice      (drawUtf8Text, fillRect 등 확장 API 포함)
✅ ProductEventCodes.h      (10000번대, Provisioning 항목 없음)
✅ ProductIds.h             (Infra_Provisioning = 6001U 정의됨)
✅ RegisterServices.cpp     (#if Incubator_ENABLE_PROVISIONING 블록 stub 존재)
```

---

## 2. 동작 요건 확정

### 2.1 시나리오 — 메뉴에서 수동 Provisioning 요청

```
MENU 4: WiFi 정보 리셋 → 버튼 짧게 누름

MENU 5: BLE Provisioning → 버튼 짧게 누름
  │
  ├─ [UiController] → ProvisioningManager.requestProvisioning()
  │
  ├─ [ProvisioningManager] → 상태: Idle/LocalMode → Advertising
  │              → BLE 광고 시작
  │              → QR 코드 화면 오버레이 표시
  │              → 남은 시간 카운트다운 (30초)
  │
  ├─ [30초 후 타임아웃]
  │              → 상태: → LocalMode
  │              → BLE 광고 중단
  │              → Main 페이지로 복귀
  │
  └─ [성공 시] → 인증정보 저장 → WiFi 연결 시도 → LocalMode 유지
                 (CloudClient가 WiFi 연결 후 역할 인계)
```

**메뉴 수동 타임아웃: 30초** (`kMenuProvisioningTimeoutMs = 30000U`)

### 2.2 핵심 원칙

```
✅ WiFi가 없어도 부화기는 항상 정상 작동한다.
✅ Provisioning 타임아웃은 부화 제어(온도/습도/전란)를 중단시키지 않는다.
✅ Provisioning 화면은 오버레이 — MainUiRenderer의 일반 렌더링을 대체하는 별도 렌더러.
✅ BLE 스택은 Provisioning 완료/타임아웃 후 즉시 해제 (메모리 반환).
✅ 인증정보는 NVS에만 저장 (AppSettings 경유).
```

---

## 3. 상태 머신 설계

### 3.1 ProvisioningState 열거형

```cpp
// include/incubator/infra/ProvisioningState.h
namespace incubator::infra
{
    enum class ProvisioningState : uint8_t
    {
        Idle        = 0,  // 초기 상태, BLE 비활성
        Advertising = 1,  // BLE 광고 중, QR 화면 표시, 타임아웃 카운트 진행
        Connecting  = 2,  // 스마트폰 연결됨, 인증정보 전송 대기
        Saving      = 3,  // 인증정보 NVS 저장 중 (단일 Tick)
        Succeeded   = 4,  // 저장 완료 → WiFi 연결 시도 준비
        TimedOut    = 5,  // 타임아웃 → LocalMode 전환 트리거
        LocalMode   = 6,  // WiFi 없이 로컬 운전 중 (안정 상태)
        Failed      = 7,  // BLE 초기화 실패 등 비정상
    };
}
```

### 3.2 상태 전이 다이어그램

```
                  startProvisioning()
  ┌─────────────────────────────────────────┐
  │                                         ▼
[Idle] ──────────────────────────────► [Advertising]
  ▲                                    │   │   │
  │                                    │   │   │ timeout
  │                          연결됨    │   │   ▼
  │                                    │   │ [TimedOut] ──► [LocalMode]
  │                                    ▼   │                    │
  │                              [Connecting]                   │
  │                                    │                        │
  │                             인증정보 수신                   │
  │                                    ▼                        │
  │                              [Saving]                       │
  │                                    │                        │
  │                             저장 완료                       │
  │                                    ▼                        │
  └───────────────────────────── [Succeeded] ──► [LocalMode]◄──┘
                                                      ▲
                                    requestProvisioning()
                                    (LocalMode → Advertising)
```

### 3.3 각 상태의 Tick 행동

| 상태 | onTick() 행동 |
|---|---|
| `Idle` | 아무 것도 안 함. `startProvisioning()` 호출 대기. |
| `Advertising` | 매 Tick: 남은 시간 = `timeoutMs - (now - enterTime)` 계산. UiModel 갱신. 타임아웃 시 → `TimedOut`. |
| `Connecting` | BLE 연결 콜백 대기. 별도 타임아웃 없음 (Advertising 타임아웃 승계). |
| `Saving` | NVS 저장 1회 시도 → 성공: `Succeeded`, 실패: `Failed` |
| `Succeeded` | 다음 Tick에 `LocalMode`로 전환. Event 발행. |
| `TimedOut` | BLE 광고 중단 → `LocalMode` 전환. Event 발행. |
| `LocalMode` | 안정 상태. `requestProvisioning()` 콜백 대기. |
| `Failed` | Error Event 발행 후 `LocalMode`로 전환. |

---

## 4. AppSettings WiFi 필드

### 4.1 수정 파일

`include/incubator/domain/model/AppSettings.h`

### 4.2 추가 필드

```cpp
// AppSettings 구조체에 아래 필드 추가
struct AppSettings
{
    // ... 기존 필드 유지 ...

    // ── WiFi 인증정보 (Provisioning 저장값) ─────────────────
    char wifiSsid[64];      // NVS 키: "wifi_ssid"  (빈 문자열 = 미설정)
    char wifiPassword[64];  // NVS 키: "wifi_pass"
    bool wifiConfigured;    // wifiSsid가 유효한지 빠른 확인용

    // ── Provisioning 정책 ────────────────────────────────────
    uint32_t bootProvisioningTimeoutMs;   // 기본: 60000 (60초)
    uint32_t menuProvisioningTimeoutMs;   // 기본: 30000 (30초)
};
```

### 4.3 기본값

```cpp
static AppSettings defaultSettings()
{
    AppSettings s{};
    // ... 기존 기본값 ...
    memset(s.wifiSsid,     0, sizeof(s.wifiSsid));
    memset(s.wifiPassword, 0, sizeof(s.wifiPassword));
    s.wifiConfigured               = false;
    s.bootProvisioningTimeoutMs    = 60000U;
    s.menuProvisioningTimeoutMs    = 30000U;
    return s;
}
```

### 4.4 NVS 저장 규칙

AppSettings는 NVS `"incubator"` namespace에 binary blob `"app_settings"` 키로 저장된다.  
필드 추가 후 구조체 크기가 변경되므로 **로드 실패 시 기본값으로 초기화**하는 기존 로직이 자동 처리한다.

---

## 5. ProductEventCode 추가

### 5.1 수정 파일

`include/incubator/config/ProductEventCodes.h`

### 5.2 추가 항목

```cpp
// ProductEventCode enum에 아래 항목 추가 (기존 Sync 영역 10200번대 이후)
// Provisioning (10500~10599)
ProvisioningStarted    = 10500,  // BLE 광고 시작 (Info)
ProvisioningSucceeded  = 10501,  // WiFi 인증정보 저장 완료 (Info)
ProvisioningTimedOut   = 10502,  // 타임아웃 → LocalMode (Info)
ProvisioningFailed     = 10503,  // BLE 초기화 실패 (Error)
ProvisioningCancelled  = 10504,  // 사용자 취소 (Info)
WifiCredentialsSaved   = 10505,  // NVS 저장 완료 (Info)
LocalModeEntered       = 10506,  // 로컬 모드 진입 (Info)
```

---

## 6. ProvisioningManager 구현 명세

### 6.1 파일 위치

```
include/incubator/infra/ProvisioningManager.h
src/infra/ProvisioningManager.cpp
```

### 6.2 계층 분류

- **레이어**: Infrastructure Service (FwCore IModule 구현)
- **역할**: BLE Provisioning 상태 머신, WiFi 인증정보 수신/저장, 타임아웃 관리
- **금지**: Device 직접 제어, UI 직접 렌더, Recovery 판단

### 6.3 클래스 선언

```cpp
// include/incubator/infra/ProvisioningManager.h
#pragma once
#include <incubator/infra/ProvisioningState.h>
#include <incubator/domain/model/AppSettings.h>
#include <incubator/domain/repository/IIncubationPlanRepository.h>
#include <cstdint>

// ESP-IDF Provisioning (빌드 플래그로 제어)
#ifdef Incubator_ENABLE_PROVISIONING
  #include <wifi_provisioning/manager.h>
  #include <wifi_provisioning/scheme_ble.h>
#endif

namespace incubator::infra
{
    class ProvisioningManager
    {
    public:
        // ── 타임아웃 상수 ────────────────────────────────────────
        static constexpr uint32_t kBootTimeoutMs = 60000U;  // 60초
        static constexpr uint32_t kMenuTimeoutMs = 30000U;  // 30초

        ProvisioningManager(
            fwcore::core::IEventBus&                           eventBus,
            fwcore::hal::IClock&                               clock,
            incubator::domain::model::AppSettings&             settings,
            incubator::domain::repository::IIncubationPlanRepository& repo);

        // ── IModule 구현 ─────────────────────────────────────────
        void setup();
        void tick();
        void shutdown();

        // ── 외부 트리거 API ──────────────────────────────────────
        // main.cpp: 부팅 시 WiFi 미설정이면 호출
        void startBootProvisioning();

        // UiController: 메뉴에서 "WiFi 설정" 선택 시 호출
        void requestMenuProvisioning();

        // UiController: 사용자 취소 (버튼 길게 누름)
        void cancelProvisioning();

        // ── 상태 조회 ────────────────────────────────────────────
        ProvisioningState getState()         const { return m_state; }
        uint32_t          getRemainingMs()   const;  // 남은 타임아웃 ms
        bool              isAdvertising()    const;
        bool              isLocalMode()      const;
        const char*       getServiceKey()    const { return m_serviceKey; }
        // BLE 서비스 이름 (QR 코드 생성에 사용)
        const char*       getDeviceName()    const { return m_deviceName; }

    private:
        // ── 상태 머신 ─────────────────────────────────────────────
        void transitionTo(ProvisioningState next);
        void onEnterAdvertising();
        void onEnterLocalMode();
        void onEnterSucceeded();
        void onEnterFailed(const char* reason);

        void tickAdvertising(uint32_t now);
        void tickConnecting(uint32_t now);
        void tickSaving();

        // ── BLE Provisioning ESP-IDF API ──────────────────────────
        bool initBleProvisioning();
        void deinitBleProvisioning();
        void startAdvertising();
        void stopAdvertising();

        // ── WiFi 인증정보 저장 ────────────────────────────────────
        bool saveCredentials(const char* ssid, const char* password);

        // ── Event 발행 ────────────────────────────────────────────
        void publishEvent(fwcore::core::EventCode code,
                          fwcore::core::Severity severity,
                          const char* message);

        // ── 멤버 변수 ─────────────────────────────────────────────
        fwcore::core::IEventBus&          m_eventBus;
        fwcore::hal::IClock&              m_clock;
        domain::model::AppSettings&       m_settings;
        domain::repository::IIncubationPlanRepository& m_repo;

        ProvisioningState m_state         = ProvisioningState::Idle;
        uint32_t          m_enterTimeMs   = 0U;
        uint32_t          m_timeoutMs     = kBootTimeoutMs;
        bool              m_bleInited     = false;

        // 수신된 인증정보 (임시 보관, Saving 상태에서 NVS 저장 후 소거)
        char m_pendingSsid[64]     = {};
        char m_pendingPassword[64] = {};

        // BLE 서비스 식별 (QR 코드 URL에 포함)
        char m_deviceName[32]  = {};  // "INCUBATOR_XXXX" 형식
        char m_serviceKey[32]  = {};  // Pop (proof-of-possession) 키

        // ESP-IDF Provisioning 이벤트 핸들러 (static)
        static void provisioningEventHandler(void* arg,
                                             esp_event_base_t eventBase,
                                             int32_t eventId,
                                             void* eventData);
        // WiFi 이벤트 핸들러
        static void wifiEventHandler(void* arg,
                                     esp_event_base_t eventBase,
                                     int32_t eventId,
                                     void* eventData);
        // 이벤트 핸들러에서 설정되는 플래그 (ISR-safe)
        volatile bool m_credentialReceived = false;
        volatile bool m_wifiConnected      = false;
    };
}
```

### 6.4 tick() 구현 가이드

```cpp
void ProvisioningManager::tick()
{
    uint32_t now = m_clock.now();

    switch (m_state) {
    case ProvisioningState::Idle:
        // 아무 것도 안 함. startBootProvisioning() / requestMenuProvisioning() 대기.
        break;

    case ProvisioningState::Advertising:
        tickAdvertising(now);
        break;

    case ProvisioningState::Connecting:
        tickConnecting(now);
        break;

    case ProvisioningState::Saving:
        tickSaving();
        break;

    case ProvisioningState::Succeeded:
        // 다음 Tick에 LocalMode로 전환
        transitionTo(ProvisioningState::LocalMode);
        break;

    case ProvisioningState::TimedOut:
        // BLE 해제 후 LocalMode
        deinitBleProvisioning();
        transitionTo(ProvisioningState::LocalMode);
        break;

    case ProvisioningState::LocalMode:
        // 안정 상태 — 아무 것도 안 함
        break;

    case ProvisioningState::Failed:
        // 이미 Event 발행됨 → LocalMode로 전환
        transitionTo(ProvisioningState::LocalMode);
        break;
    }
}

void ProvisioningManager::tickAdvertising(uint32_t now)
{
    // 1. 인증정보 수신 플래그 확인 (이벤트 핸들러에서 설정)
    if (m_credentialReceived) {
        m_credentialReceived = false;
        transitionTo(ProvisioningState::Connecting);
        return;
    }

    // 2. 타임아웃 체크
    uint32_t elapsed = now - m_enterTimeMs;
    if (elapsed >= m_timeoutMs) {
        transitionTo(ProvisioningState::TimedOut);
        return;
    }

    // 3. UiModel 남은 시간은 UiController가 getRemainingMs()로 읽어감
    //    (ProvisioningManager는 UiModel 직접 접근 금지)
}

void ProvisioningManager::tickConnecting(uint32_t now)
{
    // Advertising 타임아웃 승계 (진입 시간 기준 동일 타임아웃)
    uint32_t elapsed = now - m_enterTimeMs;
    if (elapsed >= m_timeoutMs) {
        transitionTo(ProvisioningState::TimedOut);
        return;
    }

    // WiFi 연결 성공 플래그 → Saving으로
    if (m_wifiConnected) {
        m_wifiConnected = false;
        transitionTo(ProvisioningState::Saving);
    }
}

void ProvisioningManager::tickSaving()
{
    // 단일 Tick에서 NVS 저장 시도
    if (saveCredentials(m_pendingSsid, m_pendingPassword)) {
        // 저장 성공 → 임시 버퍼 소거
        memset(m_pendingSsid,     0, sizeof(m_pendingSsid));
        memset(m_pendingPassword, 0, sizeof(m_pendingPassword));
        transitionTo(ProvisioningState::Succeeded);
    } else {
        transitionTo(ProvisioningState::Failed);
    }
}
```

### 6.5 ESP-IDF BLE Provisioning API 사용

```cpp
bool ProvisioningManager::initBleProvisioning()
{
    // 1. WiFi 초기화 (이미 되어 있으면 스킵)
    esp_err_t ret = esp_wifi_init(nullptr);
    // (실제로는 nvs_flash_init, esp_netif_init 순서가 main에서 선행)

    // 2. Provisioning Manager 초기화 (BLE scheme)
    wifi_prov_mgr_config_t config = {
        .scheme                = wifi_prov_scheme_ble,
        .scheme_event_handler  = WIFI_PROV_SCHEME_BLE_EVENT_HANDLER_FREE_BTDM,
        .app_event_handler     = {
            .event_cb  = provisioningEventHandler,
            .user_data = this,
        },
    };
    if (wifi_prov_mgr_init(config) != ESP_OK) return false;

    // 3. 이미 프로비저닝된 적 있는지 확인
    bool provisioned = false;
    wifi_prov_mgr_is_provisioned(&provisioned);
    // 이 DDU에서는 항상 새로 시작 (기존 인증정보 있어도 재프로비저닝 허용)

    // 4. BLE 디바이스 이름 설정 ("INCUBATOR_" + MAC 4자리)
    uint8_t mac[6];
    esp_wifi_get_mac(WIFI_IF_STA, mac);
    snprintf(m_deviceName, sizeof(m_deviceName),
             "INCUBATOR_%02X%02X", mac[4], mac[5]);

    // 5. PoP (proof-of-possession) 키 — 고정값 또는 NVS 저장값
    snprintf(m_serviceKey, sizeof(m_serviceKey), "incubator");

    m_bleInited = true;
    return true;
}

void ProvisioningManager::startAdvertising()
{
    if (!m_bleInited) return;

    // BLE UUID 128bit 커스텀 (선택)
    uint8_t custom_service_uuid[16] = {
        0x56, 0x6e, 0x61, 0x73, 0x6b, 0x75, 0x74, 0x69,
        0x69, 0x6e, 0x63, 0x75, 0x62, 0x61, 0x74, 0x01,
    };
    wifi_prov_scheme_ble_set_service_uuid(custom_service_uuid);

    wifi_prov_mgr_start_provisioning(
        WIFI_PROV_SECURITY_1,
        m_serviceKey,   // PoP
        m_deviceName,   // Service name
        nullptr         // Service key (unused for BLE)
    );
}

void ProvisioningManager::deinitBleProvisioning()
{
    if (m_bleInited) {
        wifi_prov_mgr_stop_provisioning();
        wifi_prov_mgr_deinit();
        m_bleInited = false;
    }
}

// ESP-IDF 이벤트 핸들러
void ProvisioningManager::provisioningEventHandler(
    void* arg, esp_event_base_t eventBase,
    int32_t eventId, void* eventData)
{
    auto* self = static_cast<ProvisioningManager*>(arg);

    if (eventBase == WIFI_PROV_EVENT) {
        switch (eventId) {
        case WIFI_PROV_CRED_RECV: {
            // 인증정보 수신 — ISR-safe 플래그만 설정
            auto* cred = static_cast<wifi_sta_config_t*>(eventData);
            strncpy(self->m_pendingSsid,
                    reinterpret_cast<const char*>(cred->ssid),
                    sizeof(self->m_pendingSsid) - 1);
            strncpy(self->m_pendingPassword,
                    reinterpret_cast<const char*>(cred->password),
                    sizeof(self->m_pendingPassword) - 1);
            self->m_credentialReceived = true;
            break;
        }
        case WIFI_PROV_END:
            // Provisioning 종료 이벤트 (정상/취소 모두)
            break;
        default:
            break;
        }
    }
}
```

### 6.6 saveCredentials() 구현

```cpp
bool ProvisioningManager::saveCredentials(const char* ssid, const char* pass)
{
    strncpy(m_settings.wifiSsid,     ssid, sizeof(m_settings.wifiSsid) - 1);
    strncpy(m_settings.wifiPassword, pass, sizeof(m_settings.wifiPassword) - 1);
    m_settings.wifiConfigured = (strlen(ssid) > 0);

    // Repository 경유 NVS 저장
    return m_repo.saveSettings(m_settings);
}
```

### 6.7 getRemainingMs() 구현

```cpp
uint32_t ProvisioningManager::getRemainingMs() const
{
    if (m_state != ProvisioningState::Advertising &&
        m_state != ProvisioningState::Connecting)
    {
        return 0U;
    }
    uint32_t elapsed = m_clock.now() - m_enterTimeMs;
    return (elapsed < m_timeoutMs) ? (m_timeoutMs - elapsed) : 0U;
}
```

---

## 7. UiModel Provisioning 필드

### 7.1 수정 파일

`include/incubator/ui/UiModel.h`

### 7.2 추가 필드

```cpp
// UiModel 구조체에 추가
struct UiModel
{
    // ... 기존 필드 유지 ...

    // ── Provisioning 관련 ────────────────────────────────────
    bool     provisioningActive;       // QR 화면 표시 여부
    uint32_t provisioningRemainingMs;  // 남은 타임아웃 (ms)
    bool     wifiConfigured;           // WiFi 인증정보 저장 여부
    char     provDeviceName[32];       // BLE 디바이스 이름 (QR URL에 사용)
    char     provServiceKey[32];       // PoP 키 (QR URL에 사용)
};
```

### 7.3 UiModel::zero() 추가 초기화

```cpp
static UiModel zero()
{
    UiModel m{};
    // ... 기존 초기화 ...
    m.provisioningActive      = false;
    m.provisioningRemainingMs = 0U;
    m.wifiConfigured          = false;
    memset(m.provDeviceName, 0, sizeof(m.provDeviceName));
    memset(m.provServiceKey, 0, sizeof(m.provServiceKey));
    return m;
}
```

---

## 8. QR 화면 렌더러 명세 (ProvisioningRenderer)

### 8.1 파일 위치

```
include/incubator/ui/ProvisioningRenderer.h
src/ui/ProvisioningRenderer.cpp
```

### 8.2 책임

- `UiModel.provisioningActive == true`일 때만 렌더 (MainUiRenderer에서 호출)
- QR 코드를 **직접 픽셀로 그린다** (라이브러리 없이, qrcodegen 경량 C 라이브러리 사용)
- QR URL 형식: `https://espressif.github.io/esp-jumpstart/qrcode.html?data={"ver":"v1","name":"INCUBATOR_XXXX","pop":"incubator","transport":"ble"}`
- 남은 시간 카운트다운 표시

### 8.3 qrcodegen 라이브러리 통합

```
components/qrcodegen/
├── qrcodegen.h   ← Nayuki QR Code generator C 라이브러리 헤더
└── qrcodegen.c   ← 구현 (MIT 라이센스, ~500줄)

출처: https://github.com/nayuki/QR-Code-generator/tree/master/c
platformio.ini: lib_deps에 추가 또는 components에 직접 포함
```

### 8.4 클래스 선언

```cpp
// include/incubator/ui/ProvisioningRenderer.h
#pragma once
#include <incubator/ui/UiModel.h>
#include <cstdint>

// qrcodegen (Nayuki, MIT)
extern "C" {
  #include "qrcodegen.h"
}

namespace incubator::ui
{
    class ProvisioningRenderer
    {
    public:
        explicit ProvisioningRenderer(const UiModel& model);

        // MainUiRenderer::render() 에서 provisioningActive 시 호출
        void render(DisplayDevice& device);

    private:
        const UiModel& m_model;

        void renderQrCode(DisplayDevice& d);
        void renderCountdown(DisplayDevice& d);
        void renderInstructions(DisplayDevice& d);

        // QR 코드 픽셀 버퍼 (static — Tick 내 malloc 금지)
        static uint8_t s_qrBuffer[qrcodegen_BUFFER_LEN_MAX];
        static uint8_t s_tempBuffer[qrcodegen_BUFFER_LEN_MAX];

        // 마지막으로 생성된 QR URL (캐시)
        static char    s_lastUrl[256];
        static bool    s_qrReady;

        void buildQrUrl(char* buf, size_t len) const;
        bool generateQr(const char* url);
        void drawQrPixels(DisplayDevice& d,
                          int originX, int originY, int pixelSize);
    };
}
```

### 8.5 화면 레이아웃 (QR 화면, 320×240)

```
┌──────────────────────────────────────────────────────────────┐  y=0
│  Header                                                      │
├──────────────────────────────────────────────────────────────┤
│                                                              │
│    ┌─────────────────┐    스마트폰으로 QR 코드를             │
│    │                 │    스캔하여 WiFi를                    │
│    │   QR CODE       │    설정하세요                         │
│    │   (128×128px)   │                                       │
│    │                 │    앱: ESP BLE Provisioning           │
│    │                 │                                       │
│    └─────────────────┘    기기명: INCUBATOR_A1B2             │
│          x=5, y=50        키:     incubator                  │
│                                                              │
├──────────────────────────────────────────────────────────────┤
│  [연결 대기 중...]                                   00:45    │ 
└──────────────────────────────────────────────────────────────┘
```

**픽셀 좌표**:

| 요소 | x | y | 크기/비고 |
|---|---|---|---|
| 헤더 "WiFi 설정" | 5 | 8 | textSize=1 |
| 남은시간 "MM:SS" | 165 | 8 | textSize=2, kAccentTemp |
| 취소 안내 | 240 | 8 | textSize=1, kTextDim |
| QR 코드 영역 | 5 | 50 | 128×128, pixelSize=4 (32×32 모듈) |
| 안내문 "스마트폰으로..." | 145 | 60 | textSize=1, kText |
| 기기명 | 145 | 120 | textSize=1, kTextDim |
| PoP 키 | 145 | 140 | textSize=1, kTextDim |
| Footer "연결 대기 중..." | 5 | 218 | textSize=1, kTextDim |
| 카운트다운 숫자 | 270 | 210 | textSize=2, kAccentTemp |

### 8.6 render() 구현 가이드

```cpp
void ProvisioningRenderer::render(IDisplayDevice& device)
{
    auto& tft = static_cast<incubator::devices::St7789DisplayDevice&>(device);

    device.beginFrame();
    device.clear();

    // ── Header ───────────────────────────────────────────────
    tft.fillRect(0, 0, 320, 30, Color::kHeader);
    tft.setTextColor(Color::kText);
    tft.setTextSize(1);
    tft.drawUtf8Text(5, 8, "WiFi 설정", Color::kText);

    // 남은 시간 MM:SS
    uint32_t remSec = m_model.provisioningRemainingMs / 1000U;
    char timeBuf[8];
    snprintf(timeBuf, sizeof(timeBuf), "%02lu:%02lu",
             remSec / 60, remSec % 60);
    tft.setTextSize(2);
    tft.drawUtf8Text(165, 5, timeBuf, Color::kAccentTemp);

    tft.setTextSize(1);
    tft.drawUtf8Text(248, 8, "취소:길게", Color::kTextDim);

    // ── QR 코드 ───────────────────────────────────────────────
    renderQrCode(device);

    // ── 안내 텍스트 ───────────────────────────────────────────
    renderInstructions(device);

    // ── Footer ────────────────────────────────────────────────
    tft.fillRect(0, 200, 320, 40, Color::kFooter);
    tft.drawUtf8Text(5, 218, "연결 대기 중...", Color::kTextDim);

    // 카운트다운 숫자 (초)
    char secBuf[4];
    snprintf(secBuf, sizeof(secBuf), "%lu", remSec % 60);
    tft.setTextSize(2);
    tft.drawUtf8Text(270, 210, secBuf, Color::kAccentTemp);

    device.endFrame();
}
```

### 8.7 QR 코드 생성 및 픽셀 렌더

```cpp
void ProvisioningRenderer::buildQrUrl(char* buf, size_t len) const
{
    // Espressif ESP-IDF Provisioning QR URL 표준 형식
    snprintf(buf, len,
        "{\"ver\":\"v1\",\"name\":\"%s\",\"pop\":\"%s\",\"transport\":\"ble\"}",
        m_model.provDeviceName,
        m_model.provServiceKey);
}

bool ProvisioningRenderer::generateQr(const char* url)
{
    return qrcodegen_encodeText(
        url,
        s_tempBuffer,
        s_qrBuffer,
        qrcodegen_Ecc_MEDIUM,
        qrcodegen_VERSION_MIN,
        qrcodegen_VERSION_MAX,
        qrcodegen_Mask_AUTO,
        true  // boostEcl
    );
}

void ProvisioningRenderer::drawQrPixels(IDisplayDevice& d,
                                         int ox, int oy, int pixelSize)
{
    auto& tft = static_cast<incubator::devices::St7789DisplayDevice&>(d);
    int size = qrcodegen_getSize(s_qrBuffer);

    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            uint16_t color = qrcodegen_getModule(s_qrBuffer, x, y)
                             ? Color::kBlack : Color::kWhite;
            tft.fillRect(ox + x * pixelSize,
                         oy + y * pixelSize,
                         pixelSize, pixelSize, color);
        }
    }
}
```

> **QR 생성 시점**: URL이 변경될 때만 재생성 (`s_lastUrl` 캐시 비교).  
> 매 Tick마다 재생성 금지 — Tick 내 연산 과부하.

---

## 9. UiController 연동

### 9.1 수정 파일

`include/incubator/ui/UiController.h` + `src/ui/UiController.cpp`

### 9.2 ProvisioningManager 주입

```cpp
// UiController 생성자에 ProvisioningManager 추가
UiController(
    ...
    incubator::devices::Ec11InputDevice&         ec11,
    incubator::domain::model::RuntimeState&      runtimeState,
    UiModel&                                     uiModel,
    incubator::infra::ProvisioningManager&       provMgr);   // ← 추가
```

### 9.3 onEvent() 추가 처리

```cpp
// UiController::onEvent() 에 추가
case ProductEventCode::ProvisioningStarted:
    m_uiModel.provisioningActive = true;
    // activePage 강제 변경 없음 — ProvisioningRenderer가 오버레이로 처리
    break;

case ProductEventCode::ProvisioningTimedOut:
case ProductEventCode::ProvisioningSucceeded:
case ProductEventCode::ProvisioningCancelled:
    m_uiModel.provisioningActive = false;
    m_uiModel.provisioningRemainingMs = 0U;
    break;

case ProductEventCode::WifiCredentialsSaved:
    m_uiModel.wifiConfigured = true;
    break;
```

### 9.4 syncUiModel() 추가

```cpp
void UiController::syncUiModel()
{
    // ... 기존 동기화 ...

    // Provisioning 남은 시간 갱신 (매 Tick)
    if (m_uiModel.provisioningActive) {
        m_uiModel.provisioningRemainingMs = m_provMgr.getRemainingMs();
        strncpy(m_uiModel.provDeviceName,
                m_provMgr.getDeviceName(),
                sizeof(m_uiModel.provDeviceName) - 1);
        strncpy(m_uiModel.provServiceKey,
                m_provMgr.getServiceKey(),
                sizeof(m_uiModel.provServiceKey) - 1);
    }
}
```

### 9.5 PAGE 5 "WiFi 설정" 항목 처리

```cpp
// PAGE 5 (System) 에서 "WiFi 설정" 선택 후 버튼 짧게
// UiController::handleButtonPress() 내에서:

if (m_uiModel.activePage == 4 /* PAGE 5 */ && isWifiSettingSelected()) {
    m_provMgr.requestMenuProvisioning();
    // ProvisioningManager가 상태 전환 후 Event 발행 → onEvent()에서 처리
}
```

### 9.6 취소 처리 (버튼 길게 → QR 화면에서)

```cpp
// UiController::handleButtonLongPress() 내에서:
if (m_uiModel.provisioningActive) {
    m_provMgr.cancelProvisioning();
    // ProvisioningCancelled Event → onEvent() → provisioningActive=false
    return;
}
```

---

## 10. main.cpp / RegisterServices 연동

### 10.1 main.cpp 수정 — 부팅 시 자동 Provisioning

```cpp
// setup() 내, kernel.setup() 이전에:

#ifdef Incubator_ENABLE_PROVISIONING
    // ESP-IDF WiFi/BT 스택 기본 초기화 (한 번만)
    nvs_flash_init();                   // NVS 초기화 (이미 DeviceRegistry에서 했으면 스킵)
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    // WiFi 미설정 시 자동 Provisioning 시작
    if (!g_settings.wifiConfigured) {
        g_provisioningMgr.startBootProvisioning();
        // 부화 제어는 계속 정상 작동 — Tick 루프에서 병렬 처리
    }
#endif
```

### 10.2 전역 객체 선언 (main.cpp 또는 별도 globals)

```cpp
// static 수명 객체들
static incubator::domain::model::AppSettings    g_settings;
static incubator::infra::ProvisioningManager    g_provisioningMgr(
    g_eventBus,
    g_clock,
    g_settings,
    g_repository);
static incubator::ui::ProvisioningRenderer      g_provRenderer(g_uiModel);
```

---

## 11. sdkconfig 필수 항목

```ini
# BLE Provisioning 필수
CONFIG_BT_ENABLED=y
CONFIG_BT_BLUEDROID_ENABLED=y
CONFIG_BT_BLE_ENABLED=y
CONFIG_ESP_WIFI_ENABLED=y

# NVS (이미 설정됨)
CONFIG_NVS_ENCRYPTION=n

# Event Loop (Provisioning 이벤트 핸들러용)
CONFIG_ESP_EVENT_POST_FROM_ISR=y

# 메모리 (BLE + WiFi 동시 사용 시 필요)
CONFIG_ESP_MAIN_TASK_STACK_SIZE=8192

# Provisioning Manager 로그 레벨
CONFIG_LOG_DEFAULT_LEVEL_DEBUG=y
```

### 11.1 platformio.ini 빌드 플래그 추가

```ini
[env:esp32-s3-devkitc-1-provisioning]
extends           = env:esp32-s3-devkitc-1
build_flags       =
    ${env:esp32-s3-devkitc-1.build_flags}
    -D Incubator_ENABLE_PROVISIONING

lib_deps          =
    ; qrcodegen (Nayuki MIT)
    ; components/qrcodegen 에 직접 포함하거나 아래 경로 사용
```

---

## 12. 파일 목록 및 책임 요약

### 12.1 신규 생성 파일

| 파일 | 레이어 | 책임 |
|---|---|---|
| `include/incubator/infra/ProvisioningState.h` | Infra | 상태 열거형 정의 |
| `include/incubator/infra/ProvisioningManager.h` | Infra | 상태 머신 + BLE API 선언 |
| `src/infra/ProvisioningManager.cpp` | Infra | BLE 광고/수신/저장, 타임아웃, Event 발행 |
| `include/incubator/ui/ProvisioningRenderer.h` | UI | QR 화면 렌더러 선언 |
| `src/ui/ProvisioningRenderer.cpp` | UI | QR 생성(qrcodegen), 픽셀 렌더, 카운트다운 |
| `components/qrcodegen/qrcodegen.h` | External | Nayuki QR Code generator (MIT) |
| `components/qrcodegen/qrcodegen.c` | External | QR Code generator 구현 |

### 12.2 수정 파일

| 파일 | 수정 내용 |
|---|---|
| `include/incubator/domain/model/AppSettings.h` | WiFi 필드 4개 추가 (`wifiSsid`, `wifiPassword`, `wifiConfigured`, 타임아웃) |
| `include/incubator/config/ProductEventCodes.h` | Provisioning 7개 Event 추가 (10500번대) |
| `include/incubator/ui/UiModel.h` | Provisioning 5개 필드 추가 |
| `include/incubator/ui/UiController.h` | `ProvisioningManager&` 생성자 인자 추가 |
| `src/ui/UiController.cpp` | `onEvent()` Provisioning Event 처리, `syncUiModel()` 추가, PAGE 5 버튼 처리 |
| `src/ui/MainUiRenderer.cpp` | `render()` 에서 `provisioningActive` 시 ProvisioningRenderer 위임 |
| `src/composition/RegisterServices.cpp` | `#ifdef` 블록에 `AddModule(g_provisioningMgr)` 추가 |
| `src/main.cpp` | `g_provisioningMgr` 객체 추가, WiFi 스택 초기화, `startBootProvisioning()` 조건부 호출 |
| `sdkconfig.defaults` | BLE 관련 CONFIG 추가 |

### 12.3 의존 관계 요약

```
ProvisioningManager
  ← depends on: IEventBus, IClock, AppSettings, IIncubationPlanRepository
  ← depends on: wifi_provisioning (ESP-IDF)
  ← used by: UiController (requestMenuProvisioning, cancelProvisioning, getRemainingMs)
  ← used by: main.cpp (startBootProvisioning)
  ← used by: FwCore Tick (IModule::tick())

ProvisioningRenderer
  ← depends on: UiModel (read-only), St7789DisplayDevice (cast), qrcodegen
  ← used by: MainUiRenderer.render() (provisioningActive 시 위임)

AppSettings (수정됨)
  ← used by: ProvisioningManager (saveCredentials)
  ← used by: main.cpp (wifiConfigured 확인)
```

---

## 13. 구현 순서 

```
STEP 1. 데이터 모델 수정
        AppSettings.h: WiFi 필드 추가, defaultSettings() 갱신
        ProductEventCodes.h: 10500번대 추가
        UiModel.h: Provisioning 필드 추가, zero() 갱신

STEP 2. ProvisioningState.h 생성
        상태 열거형 8개 정의

STEP 3. qrcodegen 라이브러리 통합
        components/qrcodegen/ 에 nayuki qrcodegen.h + qrcodegen.c 배치
        CMakeLists.txt 또는 platformio lib_deps 등록

STEP 4. ProvisioningManager.h + ProvisioningManager.cpp 구현
        4-1. 클래스 선언 완성
        4-2. initBleProvisioning() / deinitBleProvisioning()
        4-3. startAdvertising() / stopAdvertising()
        4-4. provisioningEventHandler() (static)
        4-5. tick() 상태 머신 (tickAdvertising, tickConnecting, tickSaving)
        4-6. saveCredentials()
        4-7. transitionTo() + publishEvent()
        4-8. 검증: Serial 로그에서 상태 전이 확인

STEP 5. ProvisioningRenderer.h + ProvisioningRenderer.cpp 구현
        5-1. buildQrUrl() + generateQr()
        5-2. drawQrPixels()
        5-3. renderCountdown() + renderInstructions()
        5-4. render() 전체 조립
        5-5. 검증: QR 화면 TFT 출력 + 스마트폰 스캔 확인

STEP 6. UiController 수정
        6-1. ProvisioningManager& 생성자 추가
        6-2. onEvent() Provisioning 케이스 추가
        6-3. syncUiModel() Provisioning 갱신 추가
        6-4. PAGE 5 버튼 처리 (requestMenuProvisioning)
        6-5. LongPress 취소 처리

STEP 7. MainUiRenderer 수정
        render() 에 provisioningActive 분기 추가:
        if (m_model.provisioningActive) {
            g_provRenderer.render(device); return;
        }

STEP 8. RegisterServices.cpp + main.cpp 연동
        객체 생성, WiFi 스택 초기화, startBootProvisioning() 조건부 호출
        sdkconfig.defaults BLE 항목 추가

STEP 9. 통합 검증 (§14 AC 체크)
```

---

## 14. 완료 기준 (Acceptance Criteria)

### AC-1. 부팅 시 자동 Provisioning

| 검증 항목 | 기준 |
|---|---|
| WiFi 미설정 부팅 | TFT에 QR 화면 + 카운트다운 자동 표시 |
| 60초 타임아웃 | QR 화면 사라지고 Main 화면(PAGE 1) 복귀 |
| 타임아웃 후 부화 제어 | 온도·습도 센서 읽기 및 릴레이 제어 정상 작동 |
| 타임아웃 Event | Serial TraceLog에 `ProvisioningTimedOut` Event 확인 |
| WiFi 설정 있는 부팅 | QR 화면 표시 안 함 — Main 화면 바로 진입 |

### AC-2. 메뉴 수동 Provisioning

| 검증 항목 | 기준 |
|---|---|
| PAGE 5 "WiFi 설정" 선택 후 버튼 | QR 화면 오버레이 표시 |
| 30초 타임아웃 | QR 화면 사라지고 PAGE 5 복귀 |
| 버튼 길게 누름 (QR 화면 중) | QR 화면 즉시 닫힘, ProvisioningCancelled Event |
| 카운트다운 | 1초 간격으로 남은 시간 갱신 표시 |

### AC-3. BLE Provisioning 성공

| 검증 항목 | 기준 |
|---|---|
| QR 코드 스캔 | ESP BLE Provisioning 앱으로 스캔 → 연결 가능 |
| WiFi 인증정보 전송 | 앱에서 SSID/PW 입력 후 전송 → NVS 저장 확인 |
| WifiCredentialsSaved Event | Serial TraceLog 확인 |
| 재부팅 후 | `wifiConfigured == true` → QR 화면 미표시 |

### AC-4. 아키텍처 준수

| 검증 항목 | 기준 |
|---|---|
| ProvisioningManager | UiModel 직접 접근 없음 |
| ProvisioningRenderer | RuntimeState 직접 접근 없음 |
| tick() 내 금지 패턴 | malloc/new/vTaskDelay 없음 |
| Event 없는 상태 전환 | 없음 — 모든 전환은 EventBus 발행 확인 |
| 부화 제어 격리 | Provisioning 진행 중에도 온도/습도/전란 정상 작동 확인 |

---

## Implementation Update (2026-05-06)

- Boot-time QR display is disabled. The device boots to Main even when WiFi is not configured.
- BLE Provisioning starts only from the `BLE 설정` menu item.
- QR payload uses the ESP BLE Provisioning raw JSON format:
  `{"ver":"v1","name":"INCUBATOR_XXXXXX","pop":"XXXXXXXX","transport":"ble"}`
- The QR renderer prioritizes scan reliability and uses the largest practical QR area on the 320x240 screen.

## Implementation Note (2026-05-05)

현재 V1 구현은 Arduino-ESP32 `WiFiProv`와 ESP-IDF `wifi_provisioning` 컴포넌트를 사용한다.
BLE 활성화를 위해 `sdkconfig.defaults`에 Bluedroid/BLE 설정을 추가했으며,
QR 출력은 LovyanGFX의 `qrcode()` 렌더러를 사용한다.

- 부팅 시 `AppSettings.wifiConfigured == false`이면 60초 BLE Provisioning 화면을 표시한다.
- 메뉴의 `BLE 설정` 항목은 기존 provisioning 정보를 reset하고 30초 QR 화면을 표시한다.
- 성공 시 `wifiConfigured`와 SSID를 `AppSettings` NVS blob에 저장한다.
- 긴 클릭은 Provisioning을 취소하고 로컬 모드로 돌아간다.
