# 11_Cloud_Integration_DDU_Set

> Version: 1.0
> Status: Authoritative

---

# 목적

본 문서는 AWS IoT Cloud 연동 구현을 위한 DDU 분할 전략을 정의한다.

---

# 1. 핵심 철학

```text
Cloud는 독립 Layer다.
```

---

# 2. 권장 DDU 목록

| DDU | 역할 |
|---|---|
| CLOUD-001 | WifiManager |
| CLOUD-002 | MQTT TLS Connect |
| CLOUD-003 | TelemetryBuilder |
| CLOUD-004 | CmdParser |
| CLOUD-005 | ShadowSync |
| CLOUD-006 | Offline Queue |
| CLOUD-007 | Cloud Recovery |

---

# 3. CLOUD-001 WifiManager

## 역할

- WiFi 연결
- Retry
- 상태 보고

---

## 금지

```text
❌ MQTT 처리
❌ RuntimeState 변경
```

---

# 4. CLOUD-003 TelemetryBuilder

## 역할

```text
RuntimeState
    ↓
JSON
```

---

# 5. CLOUD-004 CmdParser

## 역할

```text
JSON
    ↓
Command
```

---

# 6. 핵심 원칙

```text
Cloud는 Command만 생성한다.
```

---

# 7. 최종 원칙

```text
Cloud가 죽어도 제품은 살아야 한다.
```
