# 45_LongTerm_Maintenance_and_Refactoring_Guide

> Version: 1.0
> Status: Strategic

---

# 목적

본 문서는 장기 유지보수 및 리팩토링 전략을 정의한다.

---

# 1. 핵심 철학

```text
좋은 구조는
시간이 지나도 무너지지 않는다.
```

---

# 2. 유지보수 전략

| 전략 | 설명 |
|---|---|
| 작은 DDU | 영향 최소화 |
| 상태 분리 | 추적 가능 |
| Layer 분리 | 독립성 |
| Interface 기반 | 교체 가능 |

---

# 3. 리팩토링 기준

다음 상황이면 리팩토링 검토:

- RuntimeState Owner 불명확
- UI direct mutation 발생
- Cloud direct GPIO 발생
- Global 남발
- Recovery 흐름 복잡화

---

# 4. 좋은 리팩토링

```text
단순화
명확화
책임 분리
```

---

# 5. 나쁜 리팩토링

```text
❌ 구조 변경만 큰 작업
❌ 전체 재작성
❌ 상태 흐름 파괴
```

---

# 6. 최종 원칙

```text
이해 가능성이 가장 중요하다.
```
