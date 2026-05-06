# 14_Implementation_Roadmap

> Version: 1.0
> Status: Authoritative

---

# 목적

본 문서는 실제 구현 순서를 정의한다.

---

# Phase-01 Foundation

## 목표

아키텍처 고정.

---

## 완료 조건

```text
00~06 문서 완료
```
---

# Phase-02 Core Runtime

## 구현 대상

| 항목 | 설명 |
|---|---|
| RuntimeState | 상태 구조 |
| AppController | Command |
| Scheduler | Day 계산 |
| Climate | Hysteresis |
| Alarm | 상태 감시 |

---

# Phase-03 Storage & Recovery

## 구현 대상

```text
NVS
SPIFFS
SafeMode
Restore
```

---

# Phase-04 UI Foundation

## 구현 대상

```text
StatusBar
Home Screen
Overlay
Focus
Navigation
```

---

# Phase-05 Cloud

## 구현 대상

```text
WiFi
MQTT
Telemetry
Shadow
```

---

# Phase-06 Product Polish

## 구현 대상

```text
Animation
Trend
Diagnostics
Optimization
```

---

# 최종 목표

```text
상용 수준 Premium Embedded Product
```
