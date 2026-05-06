# 20_StateMachine_and_Navigation_Architecture

> Version: 1.0
> Status: Authoritative

---

# 목적

본 문서는 UI 상태 머신과 Navigation 구조를 정의한다.

---

# 1. 핵심 철학

```text
UI는 단순해야 한다.
```

---

# 2. 최대 Depth

```text
최대 2 Depth
```

---

# 3. 기본 구조

```text
Home
 ├─ Progress
 ├─ Manual
 ├─ PlanEdit
 └─ System
```

---

# 4. 입력 규칙

| 입력 | 의미 |
|---|---|
| Rotate | Move |
| Click | Enter |
| Hold | Back |

---

# 5. 위험 기능

| 기능 | 보호 |
|---|---|
| Heater Manual | Long Hold |
| Factory Reset | Confirm |
| SafeMode Clear | Validation |

---

# 6. Overlay 우선순위

```text
SafeMode
    > Alarm
        > Dialog
            > Toast
```

---

# 7. 최종 원칙

```text
사용자가 길을 잃으면 실패다.
```
