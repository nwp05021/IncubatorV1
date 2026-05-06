# 60_Final_Architecture_Principles

> Version: 1.0
> Status: Final

---

# 목적

본 문서는 전체 아키텍처의 최종 핵심 원칙을 요약한다.

---

# 1. 핵심 철학

```text
사람이 완전히 이해 가능한
Premium Embedded Product
```

구축.

---

# 2. 최종 구조

```text
Command
    ↓
AppController
    ↓
RuntimeState
    ↓
UI / Cloud
```

---

# 3. 상태 원칙

```text
Single Source of Truth
```

---

# 4. UI 원칙

```text
차분하고
직관적이며
신뢰감 있는 UI
```

---

# 5. Cloud 원칙

```text
Offline First
```

---

# 6. Recovery 원칙

```text
의심되면 SafeMode
```

---

# 7. Codex 협업 원칙

```text
Codex는 구현 도구일 뿐,
아키텍처는 사람이 소유한다.
```

---

# 8. 최종 목표

```text
누구에게 보여줘도
부끄럽지 않은 제품
```

---

# 9. 최종 원칙

```text
복잡함보다
명확함을 선택한다.
```
