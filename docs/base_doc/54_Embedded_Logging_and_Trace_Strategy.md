# 54_Embedded_Logging_and_Trace_Strategy

> Version: 1.0
> Status: Authoritative

---

# 목적

본 문서는 Embedded Logging 및 Trace 전략을 정의한다.

---

# 1. 핵심 철학

```text
로그는 원인 추적 가능성을 제공해야 한다.
```

---

# 2. 권장 로그

| 로그 | 목적 |
|---|---|
| Command | 상태 변경 |
| Alarm | 위험 추적 |
| Recovery | 복구 흐름 |
| Cloud | 연결 상태 |

---

# 3. 로그 예시

```text
[CMD] StartBatch OK
[ALARM] HighTemp
[RECOVERY] Restore Plan
```

---

# 4. 금지

```text
❌ 의미 없는 spam log
❌ blocking printf
❌ frame 단위 render log
```

---

# 5. 최종 원칙

```text
로그는 디버깅 도구이지
UI가 아니다.
```
