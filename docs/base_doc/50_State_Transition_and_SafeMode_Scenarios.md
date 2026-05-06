# 50_State_Transition_and_SafeMode_Scenarios

> Version: 1.0
> Status: Authoritative

---

# 목적

본 문서는 주요 상태 전환 및 SafeMode 시나리오를 정의한다.

---

# 1. 핵심 철학

```text
상태 전환은 예측 가능해야 한다.
```

---

# 2. 주요 상태

| 상태 | 설명 |
|---|---|
| Idle | 대기 |
| Incubating | 부화 중 |
| Lockdown | 전란 중단 |
| Alarm | 경고 |
| SafeMode | 보호 모드 |

---

# 3. SafeMode 진입 예시

```text
Sensor Fail
    ↓
Validation Fail
    ↓
SafeMode
```

---

# 4. Alarm 흐름

```text
HighTemp
    ↓
Alarm Overlay
    ↓
Cloud Publish
```

---

# 5. 복구 흐름

```text
Recovery Check
    ↓
Validation
    ↓
Resume
```

---

# 6. 최종 원칙

```text
위험 시 출력 차단 우선.
```
