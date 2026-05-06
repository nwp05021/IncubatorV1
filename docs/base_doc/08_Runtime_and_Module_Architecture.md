# 08_Runtime_and_Module_Architecture

> Version: 1.0
> Status: Authoritative

---

# 목적

본 문서는 Runtime Loop와 Module Layer 구조를 정의한다.

---

# 1. Runtime 철학

시스템은:

```text
Tick-Based
Non-Blocking
Single Main Loop
```

구조를 유지한다.

---

# 2. Main Loop 구조

```text
while(true)
{
    sensor.tick();
    scheduler.tick();
    climate.tick();
    turning.tick();

    appController.tick();

    ui.tick();
    cloud.tick();
}
```

---

# 3. Tick 원칙

```text
❌ delay()
❌ busy wait
❌ blocking network call
```

---

# 4. Module 역할

| Module | 역할 |
|---|---|
| SensorManager | 센싱 |
| Scheduler | Day 계산 |
| Climate | 환경 제어 |
| Turning | 전란 |
| Alarm | 알람 |
| Recovery | 복구 |

---

# 5. SensorManager

역할:

- 비동기 센싱
- RuntimeState 갱신
- 센서 상태 확인

금지:

- Heater 제어
- Alarm 발생

---

# 6. ClimateModule

역할:

- Hysteresis
- Output Control
- Alarm 판단

입력:

```text
RuntimeState
AppSettings
```

출력:

```text
GPIO State
```

---

# 7. Scheduler

역할:

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

# 8. RecoveryModule

역할:

- 오류 감지
- SafeMode 진입
- 복구 Command 발행

---

# 9. 최종 원칙

```text
Module은 상태를 계산하지만
시스템 정책의 최종 결정은 AppController가 수행한다.
```
