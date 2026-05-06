# 26_Diagnostics_and_System_Health_Architecture

> Version: 1.0
> Status: Authoritative

---

# 목적

본 문서는 시스템 진단 및 상태 모니터링 전략을 정의한다.

---

# 1. 핵심 철학

```text
문제가 생겼을 때
원인을 추적 가능해야 한다.
```

---

# 2. 최소 Diagnostic 정보

| 항목 | 설명 |
|---|---|
| BootCount | 부팅 횟수 |
| LastResetReason | 마지막 Reset |
| WdtResetCount | WDT 횟수 |
| LastSafeModeReason | 마지막 SafeMode |
| LastStorageError | 저장 오류 |

---

# 3. 상태 표시 위치

| 위치 | 표시 |
|---|---|
| System Screen | O |
| Cloud reported | O |
| Boot Log | O |

---

# 4. Health Monitor

권장:

```text
SystemHealthMonitor
```

---

# 5. 감시 대상

| 대상 | 설명 |
|---|---|
| Sensor Update | Timeout |
| Main Loop | Stall |
| Cloud | Reconnect |
| Storage | Write Fail |

---

# 6. 최종 원칙

```text
복구보다
진단 가능성이 더 중요할 때가 있다.
```
