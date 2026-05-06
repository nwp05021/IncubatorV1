# 31_Menu_and_Input_UX_Architecture

> Version: 1.0
> Status: Authoritative

---

# 목적

본 문서는 EC11 기반 메뉴 및 입력 UX 구조를 정의한다.

---

# 1. 핵심 철학

```text
입력은 빠르고 실수에 강해야 한다.
```

---

# 2. 입력 장치

```text
EC11 Rotary Encoder
+ Push Button
```

---

# 3. 입력 규칙

| 동작 | 의미 |
|---|---|
| Rotate | 이동 |
| Click | 선택 |
| Hold | 뒤로 |
| Long Hold | 위험 승인 |

---

# 4. 메뉴 구조

```text
Home
 ├─ Progress
 ├─ Manual
 ├─ Plan Edit
 └─ System
```

---

# 5. UX 원칙

| 원칙 | 설명 |
|---|---|
| Depth 최소화 | 길 잃음 방지 |
| Focus 강조 | 현재 위치 인지 |
| 큰 값 우선 | 빠른 이해 |
| 단순 Navigation | 오입력 감소 |

---

# 6. 금지

```text
❌ 복잡한 중첩 메뉴
❌ 작은 클릭 영역
❌ 숨겨진 조작
```

---

# 7. 최종 원칙

```text
사용자는 설명서 없이 조작 가능해야 한다.
```
