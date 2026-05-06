
# DDU-RELEASE-001 — Implementation Package Map

> Version: 1.0
> Status: Draft
> Target: ESP32-S3 + PlatformIO

---

# 1. 목적

지금까지 정의된 모든 DDU를 실제 구현 가능한 Package 구조로 통합한다.

목표:

- 사람이 이해 가능한 구조
- RuntimeState 중심 구조
- Layer 명확화
- PlatformIO 실제 구현 기준
- Premium Embedded Product 수준 유지

---

# 2. 최상위 구조

```text
src/
 ├── main.cpp
 ├── product/
 │
 │   ├── app/
 │   ├── domain/
 │   ├── storage/
 │   ├── recovery/
 │   ├── diagnostics/
 │   ├── event/
 │   ├── performance/
 │   ├── time/
 │   ├── cloud/
 │   ├── modules/
 │   ├── devices/
 │   ├── ui/
 │   └── boot/
 │
 └── platform/
```

---

# 3. app/

## 역할

```text
State Mutation 중앙화
```

---

## 포함

```text
AppController
Command
CommandQueue
Validation
```

---

# 4. domain/

## 역할

```text
Single Source of Truth
```

---

## 포함

```text
RuntimeState
AppSettings
AlarmState
RecoveryState
PlanRow
```

---

# 5. modules/

## 역할

```text
Business Logic
```

---

## 포함

```text
ClimateModule
TurningModule
FanModule
```

---

# 6. devices/

## 역할

```text
Hardware Access Only
```

---

## 포함

```text
Aht20Device
RelayDevice
StepperDevice
PwmFanDevice
DisplayDevice
```

---

# 7. ui/

## 역할

```text
RuntimeState Read Only
```

---

## 포함

```text
Renderer
Overlay
Dialog
Navigator
ViewModel
Toast
```

---

# 8. cloud/

## 역할

```text
Offline First Cloud Layer
```

---

## 포함

```text
WifiManager
AwsIotClient
TelemetryBuilder
ShadowSync
CmdParser
```

---

# 9. boot/

## 역할

```text
Startup Validation
Safe Boot
```

---

## 포함

```text
BootManager
BootScreenRenderer
```

---

# 10. Runtime Tick 순서

```cpp
void loop()
{
    performance.beginLoop();

    const uint32_t now = millis();

    timeService.tick(now);

    diagnostics.tick(now);

    sensorManager.tick(now);

    scheduler.tick(now);

    climate.tick(now);

    turning.tick(now);

    fan.tick(now);

    alarm.tick(now);

    recovery.tick(now);

    appController.tick();

    eventBuilder.process(
        runtime,
        alarmState);

    toast.tick(now);

    ui.tick(now);

    wifi.tick(now);

    aws.tick(now);

    shadow.tick(now);

    performance.tick(now);

    performance.endLoop();
}
```

---

# 11. 절대 규칙

```text
RuntimeState direct mutation 금지
```

반드시:

```text
Command
    ↓
AppController
```

경유.

---

# 12. Device 규칙

```text
Device는 GPIO만 담당
```

금지:

```text
❌ 정책 판단

❌ Alarm 생성

❌ Recovery 처리
```

---

# 13. UI 규칙

```text
UI는 RuntimeState 읽기 전용
```

금지:

```text
❌ GPIO direct control

❌ RuntimeState direct mutation
```

---

# 14. Cloud 규칙

```text
Cloud는 특권 계층이 아니다.
```

반드시:

```text
Cloud
    ↓
Command
    ↓
Queue
    ↓
AppController
```

---

# 15. SafeMode 규칙

## 핵심 철학

```text
불확실하면 출력 차단
```

---

## 차단 대상

```text
Heater
Humidifier
Fan
Stepper
```

---

# 16. Memory 규칙

## 금지

```text
❌ Runtime new/delete 반복

❌ Dynamic vector growth

❌ Large temporary JSON

❌ Full redraw 반복
```

---

# 17. 최종 목표

```text
6개월 후에도 사람이 이해 가능한 구조
```

---

# 18. 다음 단계

```text
DDU-IMPL-001
Phase-01 Actual Source Implementation
```
