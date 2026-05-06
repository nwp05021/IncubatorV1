# 67_UI_ViewModel_Architecture

> Version: 1.0
> Status: Authoritative

---

# 목적

본 문서는 UiViewModel 구조를 정의한다.

---

# 1. 핵심 철학

```text
UI는 RuntimeState를 직접 사용하지 않는다.
```

---

# 2. 구조

```text
RuntimeState
    ↓
UiViewModelBuilder
    ↓
UiModel
    ↓
Renderer
```

---

# 3. 장점

| 장점 | 설명 |
|---|---|
| UI 단순화 | O |
| Render 최적화 | O |
| 테스트 용이 | O |
| Layer 분리 | O |

---

# 4. 예시

```cpp
struct UiHomeModel
{
    float tempC;
    float humidityPct;

    bool heaterOn;

    uint16_t currentDay;
};
```

---

# 5. 금지

```text
❌ Renderer 내부 계산
❌ RuntimeState direct access
```

---

# 6. 최종 원칙

```text
UI는 표시 전용이다.
```
