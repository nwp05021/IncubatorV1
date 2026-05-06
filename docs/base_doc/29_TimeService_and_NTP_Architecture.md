# 29_TimeService_and_NTP_Architecture

> Version: 1.0
> Status: Authoritative

---

# 목적

본 문서는 시간 동기화 및 RTC 전략을 정의한다.

---

# 1. 핵심 철학

```text
시간은 시스템 정책의 기준이다.
```

---

# 2. TimeService 역할

| 역할 | 설명 |
|---|---|
| NTP Sync | 인터넷 시간 |
| RTC 유지 | 오프라인 유지 |
| Epoch 제공 | Scheduler |
| Timestamp | Telemetry |

---

# 3. 권장 구조

```text
NTP
    ↓
RTC
    ↓
TimeService
    ↓
Scheduler/UI/Cloud
```

---

# 4. 동기화 전략

| 상황 | 전략 |
|---|---|
| WiFi 연결 | NTP Sync |
| Offline | RTC 유지 |
| RTC Invalid | Last Known Time |

---

# 5. 저장 전략

권장:

```text
Last Sync Epoch
```

NVS 저장.

---

# 6. 금지

```text
❌ Scheduler가 직접 NTP 접근
```

---

# 7. 최종 원칙

```text
시간 서비스는 중앙화한다.
```
