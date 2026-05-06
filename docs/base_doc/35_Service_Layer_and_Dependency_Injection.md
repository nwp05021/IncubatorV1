# 35_Service_Layer_and_Dependency_Injection

> Version: 1.0
> Status: Authoritative

---

# 목적

본 문서는 Service Layer 및 Dependency Injection 전략을 정의한다.

---

# 1. 핵심 철학

```text
객체 생성과 사용을 분리한다.
```

---

# 2. Service Layer 역할

| 역할 | 설명 |
|---|---|
| 공통 기능 제공 | O |
| 객체 공유 | O |
| 의존성 연결 | O |

---

# 3. 권장 서비스

| 서비스 | 설명 |
|---|---|
| TimeService | 시간 |
| TelemetryService | 발행 |
| NotificationService | 알림 |
| StorageService | 저장 |

---

# 4. DI 구조

```text
AppBootstrap
    ↓
ServiceCollection
    ↓
Modules
```

---

# 5. 금지

```text
❌ Global Singleton 남발
❌ Module 내부 직접 생성
```

---

# 6. 최종 원칙

```text
생성보다 연결이 중요하다.
```
