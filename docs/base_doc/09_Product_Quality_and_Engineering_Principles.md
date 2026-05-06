# 09_Product_Quality_and_Engineering_Principles

> Version: 1.0
> Status: Authoritative

---

# 목적

본 문서는 Premium Embedded Product 수준의 품질 기준을 정의한다.

---

# 1. 핵심 철학

```text
“동작하는 것”과
“제품”은 다르다.
```

---

# 2. 제품 품질 기준

| 항목 | 목표 |
|---|---|
| UI | 상용 수준 |
| Recovery | 자동 |
| 유지보수 | 장기 가능 |
| 안정성 | 무정지 지향 |
| UX | 직관적 |
| 구조 | 사람이 이해 가능 |

---

# 3. 가장 중요한 기준

```text
6개월 후에도
내가 이해 가능해야 한다.
```

---

# 4. Embedded 제품 원칙

## 좋은 제품

- 상태 흐름이 명확
- Recovery가 단순
- UI가 차분
- Cloud 의존 없음

---

## 나쁜 제품

- Global 남발
- 상태 Owner 불명확
- UI가 직접 상태 수정
- Cloud가 직접 GPIO 제어

---

# 5. Premium UI 기준

Premium UI는:

```text
화려함
```
이 아니라

```text
신뢰감
```
이다.

---

# 6. Cloud 기준

Cloud는:

```text
제품을 확장하는 기능
```

이지 제품 자체가 아니다.

---

# 7. Recovery 기준

전원이 꺼져도:

```text
안전하게 복구 가능
```

해야 한다.

---

# 8. 최종 목표

최종적으로 사용자에게:

```text
“완성된 상용 장비”
```

같은 느낌을 제공해야 한다.
