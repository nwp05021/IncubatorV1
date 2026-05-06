# 52_Embedded_UI_Animation_Timing_Guide

> Version: 1.0
> Status: Authoritative

---

# 목적

본 문서는 Embedded UI Animation Timing 기준을 정의한다.

---

# 1. 핵심 철학

```text
애니메이션은
정보 전달을 방해하면 안 된다.
```

---

# 2. 권장 Timing

| Animation | 권장 시간 |
|---|---|
| Fade | 120~180ms |
| Highlight | 80~120ms |
| Page Transition | 150~220ms |
| Alarm Flash | 300~500ms |

---

# 3. 금지

```text
❌ 느린 Transition
❌ 과한 Bounce
❌ 반복 Motion
```

---

# 4. Alarm Motion

Alarm은:

```text
즉시 인지 가능
```

해야 한다.

---

# 5. 최종 원칙

```text
Motion은 부드럽지만 짧아야 한다.
```
