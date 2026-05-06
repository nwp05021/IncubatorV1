# 30_Telemetry_and_Data_Model_Architecture

> Version: 1.0
> Status: Authoritative

---

# 목적

본 문서는 Telemetry 데이터 구조와 발행 전략을 정의한다.

---

# 1. 핵심 철학

```text
Telemetry는 현재 상태를 전달한다.
```

---

# 2. 포함 데이터

| 데이터 | 설명 |
|---|---|
| Temp | 현재 온도 |
| Humi | 현재 습도 |
| Target | 목표값 |
| Outputs | 출력 상태 |
| Day | 현재 일차 |
| Alarm | 알람 |
| SafeMode | SafeMode |

---

# 3. JSON 예시

```json
{
  "tempC": 37.5,
  "humidityPct": 65.0,
  "heaterOn": true,
  "day": 7
}
```

---

# 4. 발행 정책

| 상황 | 발행 |
|---|---|
| Periodic | 60초 |
| Alarm | 즉시 |
| SafeMode | 즉시 |
| Plan Change | 즉시 |

---

# 5. Offline Queue

권장:

```text
RingBuffer Queue
```

---

# 6. 금지

```text
❌ 과도한 telemetry spam
❌ 내부 debug 값 남발
```

---

# 7. 최종 원칙

```text
Cloud 비용도 설계 대상이다.
```
