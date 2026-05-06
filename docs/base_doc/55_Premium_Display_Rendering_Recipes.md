# 55_Premium_Display_Rendering_Recipes

> Version: 1.0
> Status: Authoritative

---

# 목적

본 문서는 Premium Embedded Display 렌더링 패턴을 정의한다.

---

# 1. 핵심 철학

```text
렌더링은 제품의 완성도를 결정한다.
```

---

# 2. 권장 패턴

| 패턴 | 목적 |
|---|---|
| Dirty Render | 성능 |
| Cached Text | SPI 절감 |
| Icon Cache | Flicker 감소 |
| Partial Refresh | 부드러움 |

---

# 3. 숫자 표시

권장:

```text
Large + Bold + Center
```

---

# 4. ProgressBar

권장:

```text
Smooth Fill
Rounded Edge
Minimal Border
```

---

# 5. Alarm Overlay

권장:

```text
Strong Contrast
Minimal Text
Immediate Visibility
```

---

# 6. 금지

```text
❌ Full Screen Refresh Loop
❌ RGB Flash
❌ Tiny Dense Text
```

---

# 7. 최종 원칙

```text
사용자는 렌더링 품질을 즉시 느낀다.
```
