# 24_Climate_Control_Architecture

> Version: 1.0
> Status: Authoritative

---

# 목적

본 문서는 온습도 제어 구조를 정의한다.

---

# 1. 핵심 철학

```text
제어는 단순하고 예측 가능해야 한다.
```

---

# 2. 권장 전략

초기 버전:

```text
Hysteresis
```

권장.

---

# 3. 제어 흐름

```text
Sensor Value
    ↓
RuntimeState
    ↓
ClimateModule
    ↓
Output State
    ↓
GPIO
```

---

# 4. Hysteresis 예시

```text
Target = 37.8
Hysteresis = 0.3

ON  < 37.5
OFF > 38.1
```

---

# 5. Humidity 전략

권장:

```text
단순 Threshold + Interval
```

---

# 6. Alarm 조건

| Alarm | 조건 |
|---|---|
| HighTemp | 상한 초과 |
| LowTemp | 하한 미만 |
| HighHumidity | 상한 초과 |

---

# 7. SafeMode 연계

센서 실패 시:

```text
출력 차단
```

우선.

---

# 8. 금지

```text
❌ 복잡한 PID 남용
❌ Blocking 제어
❌ Delay 기반 제어
```

---

# 9. 최종 원칙

```text
예측 가능한 제어가
가장 안전하다.
```
