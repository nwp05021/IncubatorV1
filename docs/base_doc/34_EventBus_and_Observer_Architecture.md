# 34_EventBus_and_Observer_Architecture

> Version: 1.0
> Status: Authoritative

---

# 목적

본 문서는 EventBus 및 Observer 구조를 정의한다.

---

# 1. 핵심 철학

```text
이벤트는 상태 변화의 결과를 전달한다.
```

---

# 2. Event 목적

| 목적 | 설명 |
|---|---|
| UI 갱신 | 상태 변화 반영 |
| Cloud Sync | reported 업데이트 |
| Alarm 표시 | Overlay 표시 |
| Logging | 추적 가능성 |

---

# 3. Event 흐름

```text
Mutation
    ↓
Event Publish
    ↓
Observers
```

---

# 4. 권장 Event 목록

| Event | 설명 |
|---|---|
| BatchStarted | 부화 시작 |
| BatchCompleted | 부화 종료 |
| PlanUpdated | 정책 수정 |
| AlarmRaised | 알람 발생 |
| SafeModeEntered | SafeMode 진입 |

---

# 5. 금지

```text
❌ Event가 직접 상태 변경
❌ Event 내부 GPIO 제어
```

---

# 6. 최종 원칙

```text
Event는 결과 통지다.
```
