# 28_BLE_Provisioning_Architecture

> Version: 1.0
> Status: Authoritative

---

# 목적

본 문서는 BLE Provisioning 구조를 정의한다.

---

# 1. 핵심 철학

```text
Provisioning은
초기 설정 UX다.
```

---

# 2. 목표

사용자는:

```text
스마트폰 앱
    ↓
BLE
    ↓
WiFi 설정 완료
```

를 매우 쉽게 수행 가능해야 한다.

---

# 3. 권장 구조

```text
BLE App
    ↓
Provisioning Service
    ↓
Command
    ↓
AppController
```

---

# 4. 저장 흐름

```text
SSID/PASSWORD
    ↓
Validation
    ↓
NVS Save
    ↓
WiFi Reconnect
```

---

# 5. Provisioning 상태

| 상태 | 설명 |
|---|---|
| Idle | 대기 |
| Advertising | BLE 송출 |
| Receiving | 수신 |
| Saving | 저장 |
| Connecting | WiFi 연결 |
| Completed | 완료 |
| Failed | 실패 |

---

# 6. UX 원칙

```text
Provisioning은
“기술 기능”이 아니라
“사용자 경험”이다.
```

---

# 7. 금지

```text
❌ 복잡한 메뉴
❌ 긴 대기
❌ 실패 원인 미표시
```

---

# 8. 최종 원칙

```text
Provisioning 실패는
제품 첫인상을 망친다.
```
