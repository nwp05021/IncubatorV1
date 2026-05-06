# 39_Premium_Embedded_Product_Philosophy

> Version: 1.0
> Status: Authoritative

---

# 목적

본 문서는 Premium Embedded Product 철학을 정의한다.

---

# 1. 핵심 철학

```text
“동작한다”는
“좋은 제품”과 다르다.
```

---

# 2. 좋은 제품의 특징

| 특징 | 설명 |
|---|---|
| 안정감 | 신뢰 |
| 단순함 | 빠른 이해 |
| 복구 가능성 | 안전 |
| 직관성 | UX |
| 유지보수성 | 장기 운영 |

---

# 3. 나쁜 Embedded 제품

```text
❌ 상태 흐름 불명확
❌ UI가 직접 상태 수정
❌ Cloud가 직접 GPIO 제어
❌ Recovery 부재
```

---

# 4. 좋은 Embedded 제품

```text
Command
    ↓
AppController
    ↓
RuntimeState
    ↓
UI / Cloud
```

흐름이 명확하다.

---

# 5. UI 철학

좋은 MCU UI는:

```text
게임 UI
```

가 아니라

```text
산업 장비 UI
```

다.

---

# 6. 최종 목표

```text
ESP32 프로젝트처럼 보이지 않는 것
```

---

# 7. 최종 원칙

```text
사용자가 불안을 느끼면 실패다.
```
