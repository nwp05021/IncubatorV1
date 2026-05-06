# 19_UI_Rendering_Performance_Strategy

> Version: 1.0
> Status: Authoritative

---

# 목적

본 문서는 TFT UI 렌더링 성능 전략을 정의한다.

---

# 1. 핵심 철학

```text
그리기보다
안 그리는 것이 더 중요하다.
```

---

# 2. 핵심 목표

| 목표 | 설명 |
|---|---|
| Flicker 최소화 | UX 향상 |
| CPU 사용 감소 | 제어 안정 |
| SPI Traffic 감소 | 성능 향상 |
| Full Refresh 최소화 | 고급감 |

---

# 3. Dirty Render

권장:

```text
Dirty Region Render
```

---

# 4. Render Pipeline

```text
RuntimeState
    ↓
UiModel Build
    ↓
Dirty Check
    ↓
Render Queue
    ↓
Renderer
```

---

# 5. Full Refresh 금지

```text
❌ 매 프레임 전체 redraw
```

---

# 6. Cached Text

권장:

```text
변하지 않는 문자열 캐시
```

예:

- Title
- Labels
- Icons

---

# 7. Bitmap 전략

권장:

```text
Status Icon Bitmap Cache
```

---

# 8. Animation 정책

```text
부드럽지만 거의 느껴지지 않을 정도
```

---

# 9. 최종 원칙

```text
UI 때문에 제어 루프가 흔들리면 실패다.
```
