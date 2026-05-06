# 36_Memory_and_Resource_Constraints

> Version: 1.0
> Status: Authoritative

---

# 목적

본 문서는 메모리 및 리소스 제약 전략을 정의한다.

---

# 1. 핵심 철학

```text
Embedded 시스템은 리소스가 제한적이다.
```

---

# 2. 메모리 전략

| 항목 | 전략 |
|---|---|
| UI Buffer | Static |
| JSON | Fixed Size |
| Queue | RingBuffer |
| Telemetry | Fixed Array |

---

# 3. 금지

```text
❌ 반복 new/delete
❌ 무제한 vector 증가
❌ 대형 stack allocation
```

---

# 4. 권장

```text
Static Allocation
Object Reuse
Dirty Render
```

---

# 5. CPU 전략

| 전략 | 설명 |
|---|---|
| Tick 기반 | O |
| Busy Wait 제거 | O |
| Render 최소화 | O |

---

# 6. 최종 원칙

```text
안정성이 기능보다 우선이다.
```
