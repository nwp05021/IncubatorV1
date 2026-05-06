# 15_Codex_Collaboration_Guide

> Version: 1.0
> Status: Authoritative

---

# 목적

본 문서는 Codex/Cursor와 안전하게 협업하는 규칙을 정의한다.

---

# 1. 핵심 철학

```text
Codex는 구현 도구다.
아키텍처는 사람이 소유한다.
```

---

# 2. 절대 금지

```text
❌ 전체 제품 생성
❌ 구조 설계 위임
❌ 대규모 자동 생성
```

---

# 3. 권장 방식

```text
작고 명확한 작업만 요청
```

---

# 4. 좋은 예시

```text
P0 Home Screen TemperatureCard 구현
- UiModel만 사용
- Dirty Render 적용
- RuntimeState 접근 금지
```

---

# 5. 나쁜 예시

```text
부화기 UI 전체 구현해줘
```

---

# 6. 항상 포함할 정보

| 항목 | 필요 |
|---|---|
| 파일 경로 | O |
| 입력 구조 | O |
| 출력 구조 | O |
| 금지 사항 | O |
| Acceptance Criteria | O |

---

# 7. 디버깅 전략

Codex에는:

```text
오류 메시지
현재 코드
기대 동작
```

만 제공.

---

# 8. 가장 중요한 규칙

```text
사람이 상태 흐름을 이해하지 못하면
구조를 다시 설계한다.
```

---

# 9. 최종 목표

```text
토큰 부족에도 유지 가능한 구조
```
