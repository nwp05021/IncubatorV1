# 42_UI_Screen_Detail_Specification

> Version: 1.0
> Status: Authoritative

---

# 목적

본 문서는 각 UI 화면의 상세 스펙을 정의한다.

---

# P0 Home

## 핵심 정보

```text
현재 온도
현재 습도
Target
진행 상태
```

---

# P1 Progress

## 핵심 정보

```text
Current Day
ProgressBar
Next Turning
Lockdown
```

---

# P2 Manual

## 핵심 정보

```text
Manual Outputs
Warning Banner
```

---

# P3 Plan Edit

## 핵심 정보

```text
Day
Temp
Humidity
Turning
```

---

# P4 System

## 핵심 정보

```text
WiFi
AWS
IP
FW Version
Uptime
```

---

# Overlay

| Overlay | 목적 |
|---|---|
| Alarm | 위험 |
| SafeMode | 치명 오류 |
| Toast | 짧은 알림 |

---

# 최종 원칙

```text
한 화면 = 하나의 목적
```
