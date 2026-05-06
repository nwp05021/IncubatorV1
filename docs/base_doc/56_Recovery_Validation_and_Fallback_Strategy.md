# 56_Recovery_Validation_and_Fallback_Strategy

> Version: 1.0
> Status: Authoritative

---

# 목적

본 문서는 Recovery Validation 및 Fallback 전략을 정의한다.

---

# 1. 핵심 철학

```text
복구 실패 시 무리하게 진행하지 않는다.
```

---

# 2. Validation 흐름

```text
Load
    ↓
Schema Check
    ↓
Range Check
    ↓
Fallback
```

---

# 3. Fallback 전략

| 실패 | 대응 |
|---|---|
| Plan Corrupt | plan.bak |
| plan.bak Fail | regenerate |
| Settings Corrupt | default |
| Batch Invalid | inactive |

---

# 4. SafeMode 조건

| 조건 | 설명 |
|---|---|
| Sensor Fail | 위험 |
| Restore Fail | 불확실 |
| WDT Loop | 불안정 |

---

# 5. 최종 원칙

```text
의심되면 SafeMode.
```
