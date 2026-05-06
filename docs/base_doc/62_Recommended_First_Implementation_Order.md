# 62_Recommended_First_Implementation_Order

> Version: 1.0
> Status: Practical Guide

---

# 목적

본 문서는 실제 구현 시작 순서를 정의한다.

---

# Phase-01 Runtime Foundation

## 구현 대상

```text
RuntimeState
AppSettings
Command
AppController
```

---

# Phase-02 Storage

## 구현 대상

```text
NVS Settings
SPIFFS Plan
Restore
```

---

# Phase-03 Sensor / Climate

## 구현 대상

```text
AHT20
SensorManager
ClimateModule
Alarm
```

---

# Phase-04 UI Foundation

## 구현 대상

```text
StatusBar
Home Screen
Navigation
Overlay
```

---

# Phase-05 Scheduler

## 구현 대상

```text
Batch
PlanTable
CurrentDay
Lockdown
```

---

# Phase-06 Cloud

## 구현 대상

```text
WiFi
MQTT
Telemetry
Shadow
```

---

# 권장 전략

```text
항상 작은 단위로 구현
```

---

# 금지

```text
❌ 전체 시스템 동시 구현
❌ UI 전체 자동 생성
```

---

# 최종 원칙

```text
실기기에서 매 단계 검증한다.
```
