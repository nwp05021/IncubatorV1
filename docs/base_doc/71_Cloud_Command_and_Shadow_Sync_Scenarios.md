# 71_Cloud_Command_and_Shadow_Sync_Scenarios

> Version: 1.0
> Status: Scenario Guide

---

# 목적

본 문서는 Cloud Command 및 Shadow Sync 시나리오를 정의한다.

---

# 1. desired → Command

```text
AWS desired
    ↓
CmdParser
    ↓
Command
    ↓
AppController
```

---

# 2. reported 흐름

```text
RuntimeState
    ↓
TelemetryBuilder
    ↓
AWS reported
```

---

# 3. Plan 변경 예시

```text
Cloud Plan Patch
    ↓
Validation
    ↓
Plan Save
    ↓
reported Sync
```

---

# 4. SafeMode 예시

```text
Sensor Fail
    ↓
SafeMode
    ↓
reported.safeMode=true
```

---

# 5. 최종 원칙

```text
Cloud도 동일한 규칙을 따른다.
```
