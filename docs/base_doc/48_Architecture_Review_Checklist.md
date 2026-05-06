# 48_Architecture_Review_Checklist

> Version: 1.0
> Status: Authoritative

---

# 목적

본 문서는 아키텍처 리뷰 체크리스트를 정의한다.

---

# 1. 상태 흐름

| 질문 | 확인 |
|---|---|
| RuntimeState Owner 명확한가 | O |
| UI direct mutation 없는가 | O |
| Cloud direct GPIO 없는가 | O |

---

# 2. Recovery

| 질문 | 확인 |
|---|---|
| Power Loss 복구 가능한가 | O |
| SafeMode 존재하는가 | O |
| WDT 대응 가능한가 | O |

---

# 3. UI

| 질문 | 확인 |
|---|---|
| 1초 안에 상태 이해 가능한가 | O |
| Overlay 우선순위 명확한가 | O |
| Flicker 없는가 | O |

---

# 4. Cloud

| 질문 | 확인 |
|---|---|
| Offline 동작 가능한가 | O |
| desired → Command 구조인가 | O |

---

# 5. 코드 구조

| 질문 | 확인 |
|---|---|
| Layer 분리 명확한가 | O |
| Interface 존재하는가 | O |
| 작은 DDU 구조인가 | O |

---

# 6. 최종 원칙

```text
사람이 이해 못 하면 실패다.
```
