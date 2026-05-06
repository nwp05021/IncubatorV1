# 66_UI_Renderer_Template_Set

> Version: 1.0
> Status: Implementation Template

---

# 목적

본 문서는 Renderer 구현 템플릿을 제공한다.

---

# 기본 Renderer

```cpp
class TemperatureCardRenderer
{
public:
    void render(DisplayDevice& display,
                const UiTemperatureCardModel& model);
};
```

---

# 입력 원칙

Renderer는:

```text
UiModel만 사용
```

---

# 금지

```text
❌ RuntimeState 접근
❌ GPIO 접근
❌ Business Logic
```

---

# Dirty Render 예시

```cpp
if (oldValue != newValue)
{
    redraw();
}
```

---

# 핵심 원칙

```text
Renderer는 그리기만 한다.
```
