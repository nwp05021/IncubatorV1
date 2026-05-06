# 27_Firmware_Update_and_Versioning_Strategy

> Version: 1.0
> Status: Authoritative

---

# 목적

본 문서는 Firmware Version 및 향후 OTA 전략을 정의한다.

---

# 1. 핵심 철학

```text
업데이트는
복구 가능해야 한다.
```

---

# 2. Version 정책

권장:

```text
MAJOR.MINOR.PATCH
```

예:

```text
1.2.4
```

---

# 3. Version 포함 위치

| 위치 | 포함 |
|---|---|
| System Screen | O |
| Cloud Report | O |
| Boot Log | O |

---

# 4. Schema Version

Persistent Data는:

```text
Schema Version
```

반드시 포함.

---

# 5. OTA 전략 (향후)

권장:

```text
Dual Partition OTA
```

---

# 6. OTA 실패 정책

```text
Rollback 가능
```

해야 한다.

---

# 7. 금지

```text
❌ OTA 중 상태 손상
❌ 설정 유실
```

---

# 8. 최종 원칙

```text
업데이트보다
복구가 우선이다.
```
