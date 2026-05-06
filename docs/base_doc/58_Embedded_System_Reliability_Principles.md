# 58_Embedded_System_Reliability_Principles

> Version: 1.0
> Status: Authoritative

---

# 목적

본 문서는 Embedded System Reliability 원칙을 정의한다.

---

# 1. 핵심 철학

```text
안정성은 기능보다 우선한다.
```

---

# 2. Reliability 목표

| 목표 | 설명 |
|---|---|
| 장시간 동작 | 무정지 지향 |
| 복구 가능성 | 자동 Recovery |
| 예측 가능성 | 안정적 상태 흐름 |
| Fail Safe | 위험 차단 |

---

# 3. 권장 전략

| 전략 | 설명 |
|---|---|
| Tick 기반 | Non-Blocking |
| Watchdog | Stall 복구 |
| SafeMode | 위험 보호 |
| Dirty Render | 성능 안정 |
| Interface 분리 | 유지보수 |

---

# 4. 금지

```text
❌ delay 기반 구조
❌ blocking network
❌ uncontrolled memory allocation
```

---

# 5. 최종 원칙

```text
불안정한 기능보다
안정적인 단순 기능이 낫다.
```
