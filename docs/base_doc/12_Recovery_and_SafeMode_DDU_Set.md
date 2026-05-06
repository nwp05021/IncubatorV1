# 12_Recovery_and_SafeMode_DDU_Set

> Version: 1.0
> Status: Authoritative

---

# 목적

본 문서는 Recovery/SafeMode 관련 구현 단위를 정의한다.

---

# 1. 핵심 철학

```text
복구보다 중요한 것은 안전 정지다.
```

---

# 2. 권장 DDU 목록

| DDU | 역할 |
|---|---|
| REC-001 | Reset Reason Detector |
| REC-002 | Boot Recovery |
| REC-003 | Plan Validation |
| REC-004 | Settings Validation |
| REC-005 | SafeMode Manager |
| REC-006 | Alarm Recovery |
| REC-007 | WDT Recovery |

---

# 3. SafeMode Manager

## 역할

- 출력 차단
- 상태 표시
- 복구 허용 여부 판단

---

## 차단 대상

```text
Heater
Humidifier
Turner
```

---

# 4. Boot Recovery

## 흐름

```text
Load
    ↓
Validate
    ↓
Restore
    ↓
SafeMode 판단
```

---

# 5. Recovery 원칙

```text
복구 실패 시
무리하게 동작하지 않는다.
```

---

# 6. 최종 원칙

```text
의심되면 안전 정지.
```
