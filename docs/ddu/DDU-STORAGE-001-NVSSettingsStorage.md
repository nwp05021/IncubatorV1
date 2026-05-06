
# DDU-STORAGE-001 — NVS Settings Storage

> Version: 1.0
> Status: Draft
> Target: ESP32-S3 + PlatformIO
> Dependency:
> - RuntimeState
> - AppSettings
> - Recovery Manager
>
> Estimated Time: 20~40 min

---

# 1. 목적

NVS 기반 영구 저장 구조 구현.

핵심 흐름:

```text
Command
    ↓
AppController
    ↓
StorageService
    ↓
NVS
```

목표:

- AppSettings 저장
- Settings Validation
- Schema Version
- Atomic Save
- Recovery 연계

---

# 2. 생성 파일

```text
product/storage/PersistentHeader.h

product/storage/SettingsStorage.h
product/storage/SettingsStorage.cpp
```

---

# 3. 핵심 철학

```text
저장은 최소화하고
복구는 확실하게
```

---

# 4. PersistentHeader

## 목적

저장 데이터 유효성 확인.

---

## PersistentHeader.h

```cpp
#pragma once

#include <stdint.h>

namespace incubator::storage
{
    struct PersistentHeader
    {
        uint32_t magic = 0x50494E43;

        uint16_t schemaVersion = 1;

        uint16_t size = 0;
    };
}
```

---

# 5. 저장 구조

## 저장 데이터

```text
PersistentHeader
AppSettings
```

---

## NVS Key

```text
settings_blob
```

---

# 6. SettingsStorage

## 역할

```text
Save
Load
Validate
Restore Default
```

---

## SettingsStorage.h

```cpp
#pragma once

#include "../domain/AppSettings.h"

namespace incubator::storage
{
    class SettingsStorage
    {
    public:
        bool begin();

        bool load(
            incubator::domain::AppSettings& settings);

        bool save(
            const incubator::domain::AppSettings& settings);

    private:
        bool validate(
            const incubator::domain::AppSettings& settings);

        void applyDefaults(
            incubator::domain::AppSettings& settings);
    };
}
```

---

# 7. Validation 정책

| 항목 | 범위 |
|---|---|
| tempHysteresis | 0.1 ~ 2.0 |
| humidityHysteresis | 1 ~ 10 |
| telemetryIntervalMs | >= 10000 |
| fanNormalPwm | 0 ~ 100 |

---

# 8. SettingsStorage.cpp

```cpp
#include "SettingsStorage.h"

namespace incubator::storage
{
    using namespace incubator::domain;

    bool SettingsStorage::begin()
    {
        return true;
    }

    bool SettingsStorage::load(
        AppSettings& settings)
    {
        // TODO:
        // NVS Read

        if (!validate(settings))
        {
            applyDefaults(settings);

            return false;
        }

        return true;
    }

    bool SettingsStorage::save(
        const AppSettings& settings)
    {
        if (!validate(settings))
        {
            return false;
        }

        // TODO:
        // NVS Write

        return true;
    }

    bool SettingsStorage::validate(
        const AppSettings& settings)
    {
        if (settings.tempHysteresis < 0.1f ||
            settings.tempHysteresis > 2.0f)
        {
            return false;
        }

        if (settings.humidityHysteresis < 1.0f ||
            settings.humidityHysteresis > 10.0f)
        {
            return false;
        }

        if (settings.telemetryIntervalMs < 10000)
        {
            return false;
        }

        return true;
    }

    void SettingsStorage::applyDefaults(
        AppSettings& settings)
    {
        settings = AppSettings();
    }
}
```

---

# 9. Atomic Save 전략

## 핵심 원칙

```text
부분 저장 방지
```

---

## 권장 흐름

```text
tmp write
    ↓
flush
    ↓
rename
```

---

# 10. Recovery 연계

## Validation 실패

```text
Load Fail
    ↓
Default Restore
    ↓
Warning Event
```

---

## 심각 오류

```text
반복 저장 실패
    ↓
SafeMode 가능
```

---

# 11. AppController 연결

## 핵심 원칙

```text
설정 저장은 AppController만 수행
```

---

## 금지

```text
❌ UI direct save

❌ Cloud direct save

❌ Module direct save
```

---

# 12. Main Flow

```text
UI Action
    ↓
Command
    ↓
AppController
    ↓
Settings Mutation
    ↓
SettingsStorage Save
```

---

# 13. 핵심 장점

## 1) 저장 중앙화

설정 저장 흐름 통일.

---

## 2) Schema Version 대응

Firmware 업데이트 대응 가능.

---

## 3) Recovery 단순화

```text
Load
 ↓
Validate
 ↓
Restore
```

구조 유지.

---

# 14. 금지 사항

```text
❌ RuntimeState 저장

❌ 반복 save loop

❌ UI direct NVS 접근

❌ Storage 내부 UI 처리
```

---

# 15. Acceptance Criteria

```text
AC-1
Settings Validation 정상 동작

AC-2
잘못된 값 Restore Default

AC-3
NVS Save 성공

AC-4
NVS Load 성공

AC-5
AppController 경유 저장 구조 유지
```

---

# 16. 다음 단계

다음 DDU:

```text
DDU-UI-001
Premium Home UI Foundation
```

다음 구현 예정:

- StatusBar
- TemperatureCard
- HumidityCard
- ProgressBar
- Dirty Render
- UiModel
