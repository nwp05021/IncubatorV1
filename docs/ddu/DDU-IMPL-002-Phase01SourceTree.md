
# DDU-IMPL-002 — Phase-01 Source Tree

> Version: 1.0
> Status: Draft
> Target: ESP32-S3 + PlatformIO

---

# 1. 목적

실제 구현을 시작하기 위한 최소 컴파일 단위 Source Tree를 정의한다.

목표:

- 바로 구현 가능한 구조
- include 혼란 제거
- namespace 일관성
- RuntimeState 중심 구조

---

# 2. 권장 namespace

```cpp
namespace incubator
```

하위:

```cpp
incubator::domain
incubator::app
incubator::ui
incubator::cloud
```

---

# 3. 실제 Source Tree

```text
src/
 ├── main.cpp
 │
 ├── product/
 │   ├── app/
 │   │   ├── Command.h
 │   │   ├── CommandQueue.h
 │   │   ├── CommandQueue.cpp
 │   │   ├── AppController.h
 │   │   └── AppController.cpp
 │   │
 │   ├── domain/
 │   │   ├── RuntimeState.h
 │   │   ├── AppSettings.h
 │   │   ├── AlarmState.h
 │   │   └── RecoveryState.h
 │   │
 │   ├── modules/
 │   ├── devices/
 │   ├── ui/
 │   ├── cloud/
 │   └── storage/
 │
 └── platform/
```

---

# 4. main.cpp 원칙

```text
main.cpp는 wiring만 담당
```

금지:

```text
❌ business logic

❌ validation

❌ device policy
```

---

# 5. RuntimeState 규칙

## 핵심 원칙

```text
Single Source of Truth
```

---

## 저장 금지

```text
❌ RuntimeState persistent save
```

---

# 6. AppController 규칙

## 역할

```text
유일한 상태 변경 지점
```

---

## 흐름

```text
Command
    ↓
Validation
    ↓
RuntimeState Mutation
```

---

# 7. Device 규칙

```text
Hardware Access Only
```

예시:

```cpp
relay.on()
relay.off()
fan.setPwm()
```

---

# 8. Module 규칙

```text
Business Logic
```

예시:

```text
Climate
Turning
Fan
Alarm
```

---

# 9. UI 규칙

```text
RuntimeState Read Only
```

---

# 10. Cloud 규칙

```text
Cloud → Command only
```

---

# 11. 추천 Build 순서

```text
1. RuntimeState
2. Command
3. CommandQueue
4. AppController
5. main.cpp wiring
6. Sensor Pipeline
7. Climate
8. UI
9. Cloud
```

---

# 12. 최종 목표

```text
읽으면 구조가 보이는 펌웨어
```
