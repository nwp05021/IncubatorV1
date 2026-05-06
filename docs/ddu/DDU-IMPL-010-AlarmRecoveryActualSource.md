
# DDU-IMPL-010 — Alarm & Recovery Actual Source

## 핵심 흐름

RuntimeState
    ↓
AlarmModule
    ↓
RecoveryManager
    ↓
SafeMode

## 목표

- High Temp Alarm
- Low Temp Alarm
- Alarm Delay
- SafeMode Trigger
- Output Protection

## 생성 파일

product/modules/alarm/AlarmModule.h
product/modules/alarm/AlarmModule.cpp

product/recovery/RecoveryManager.h
product/recovery/RecoveryManager.cpp

## 핵심 철학

위험 상태는 즉시 감지하고
불확실하면 출력 차단.

## Alarm 정책

- High Temp: target + 1.5°C
- Low Temp: target - 2.0°C
- Alarm Delay: 10 sec

## Recovery 정책

sensorHealthy == false
    ↓
safeMode = true

storageHealthy == false
    ↓
safeMode = true

## Acceptance Criteria

- Alarm 동작 정상
- SafeMode 진입 정상
- Heater OFF 정상
- Humidifier OFF 정상
- Non-Blocking 유지
