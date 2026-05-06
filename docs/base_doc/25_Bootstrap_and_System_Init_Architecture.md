# 25_Bootstrap_and_System_Init_Architecture

> Version: 1.0
> Status: Authoritative

---

# 목적

본 문서는 시스템 부팅 및 초기화 전략을 정의한다.

---

# 1. 핵심 철학

```text
부팅 실패 시에도
최대한 안전하게 살아남는다.
```

---

# 2. 부팅 순서

```text
Power On
    ↓
NVS Init
    ↓
Settings Load
    ↓
SPIFFS Mount
    ↓
Plan Load
    ↓
Device Init
    ↓
RuntimeState Init
    ↓
Recovery Check
    ↓
UI Boot Screen
    ↓
Main Loop
```

---

# 3. AppBootstrap 역할

| 역할 | 설명 |
|---|---|
| Service Wiring | 객체 생성 |
| Dependency 연결 | 주입 |
| Recovery Start | 복구 |
| Main Loop 준비 | 실행 |

---

# 4. 초기화 실패 정책

| 실패 | 대응 |
|---|---|
| SPIFFS Fail | SafeMode |
| Sensor Fail | SafeMode |
| WiFi Fail | Offline Continue |

---

# 5. 금지

```text
❌ setup() 내부 대규모 로직
❌ 부팅 중 blocking retry
```

---

# 6. 최종 원칙

```text
부팅은 빠르고 단순해야 한다.
```
