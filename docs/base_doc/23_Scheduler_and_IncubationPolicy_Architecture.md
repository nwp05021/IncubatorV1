# 23_Scheduler_and_IncubationPolicy_Architecture

> Version: 1.0
> Status: Authoritative

---

# 목적

본 문서는 부화 스케줄 및 정책 구조를 정의한다.

---

# 1. 핵심 철학

```text
현재 Day가
시스템 목표를 결정한다.
```

---

# 2. 핵심 흐름

```text
Batch
    ↓
Current Day
    ↓
Plan Row
    ↓
RuntimeState Target
```

---

# 3. Preset 정책

| Preset | 설명 |
|---|---|
| Chicken | 닭 |
| Duck | 오리 |
| Quail | 메추리 |

---

# 4. Plan Row 예시

```cpp
struct PlanRow
{
    float targetTempC;
    float targetHumidityPct;

    bool turningEnabled;

    uint16_t turningIntervalMin;
};
```

---

# 5. Lockdown

Lockdown 기간:

```text
전란 중단
습도 증가
```

---

# 6. Scheduler 역할

| 역할 | 설명 |
|---|---|
| Day 계산 | Epoch 기반 |
| Plan 적용 | RuntimeState 반영 |
| Lockdown 판단 | 상태 반영 |

---

# 7. 최종 원칙

```text
Scheduler는 정책을 적용하지만
상태 변경은 AppController가 수행한다.
```
