# 10_UI_Component_DDU_Set

> Version: 1.0
> Status: Authoritative

---

# 목적

본 문서는 UI Component 구현을 위한 세부 DDU 분할 전략을 정의한다.

---

# 1. 핵심 원칙

```text
UI는 작은 Component 단위로 구현한다.
```

---

# 2. 권장 UI DDU 목록

| DDU | 역할 |
|---|---|
| UI-001 | StatusBar |
| UI-002 | TemperatureCard |
| UI-003 | HumidityCard |
| UI-004 | ProgressBar |
| UI-005 | StatusPill |
| UI-006 | AlarmOverlay |
| UI-007 | SafeModeOverlay |
| UI-008 | Toast |
| UI-009 | Dialog |
| UI-010 | FocusRenderer |

---

# 3. UI-001 StatusBar

## 역할

- 현재 시간
- Day
- WiFi
- AWS 상태

표시.

---

## 입력

```text
UiModel
```

---

## 금지

```text
❌ RuntimeState 직접 접근
❌ GPIO 접근
```

---

# 4. UI-002 TemperatureCard

## 역할

현재 온도 표시.

---

## 표시 항목

```text
Current Temp
Target Temp
Trend
```

---

## Acceptance Criteria

```text
AC-1
37.5° 정상 표시

AC-2
Dirty Render 적용

AC-3
단위 크기 축소
```

---

# 5. UI Overlay 원칙

Overlay는:

```text
반드시 현재 작업보다 우선해야 할 때만 사용
```

---

# 6. 최종 원칙

```text
Renderer는 그리기만 한다.
```
