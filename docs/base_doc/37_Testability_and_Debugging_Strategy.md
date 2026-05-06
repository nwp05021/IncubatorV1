# 37_Testability_and_Debugging_Strategy

> Version: 1.0
> Status: Authoritative

---

# 목적

본 문서는 테스트 가능성과 디버깅 전략을 정의한다.

---

# 1. 핵심 철학

```text
디버깅 가능한 구조가
좋은 구조다.
```

---

# 2. Testability 전략

| 전략 | 설명 |
|---|---|
| Interface 기반 | Mock 가능 |
| State 분리 | 검증 용이 |
| DI 구조 | 교체 가능 |
| Small DDU | 단위 테스트 가능 |

---

# 3. Debugging 전략

권장:

```text
상태 흐름 로그
```

---

# 4. 중요 로그

| 로그 | 설명 |
|---|---|
| Command | 상태 변경 |
| Alarm | 위험 |
| Recovery | 복구 |
| SafeMode | 진입 이유 |

---

# 5. 금지

```text
❌ printf 남발
❌ 의미 없는 로그
❌ Runtime blocking debug
```

---

# 6. 최종 원칙

```text
문제 원인 추적이 가능해야 한다.
```
