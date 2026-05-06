# DDU-CMD-001 — Command / CommandQueue

> Version: 1.0
> Status: Draft
> Dependency: AppController Command Architecture
> Estimated Time: 30~40 min

## 목적

이 DDU 완료 후 다음이 가능해야 한다.

- UI, Cloud, BLE, Recovery 입력을 동일한 Command 구조로 통일한다.
- CommandQueue를 통해 Single Threaded State Mutation을 보장한다.
- Queue Overflow 시 동적 확장 없이 안전하게 Reject한다.

## 생성 파일

```text
include/app/Command.h
include/app/CommandQueue.h
src/app/CommandQueue.cpp
```

## 핵심 구조

```text
Input Source
    ↓
Command
    ↓
CommandQueue FIFO
    ↓
AppController::tick()
    ↓
Validation
    ↓
Mutation
```

## Queue 정책

- 고정 크기 Ring Buffer
- FIFO
- Overflow 시 false 반환
- 동적 메모리 할당 없음

## 금지 사항

```text
❌ std::vector 기반 Runtime 확장
❌ Queue 내부 동적 할당
❌ Command가 직접 GPIO 제어
```

## Acceptance Criteria

AC-1. CommandQueue는 push/pop/overflow 동작이 명확하다.

AC-2. AppController는 Tick마다 최대 1개 Command만 처리한다.

AC-3. SafeMode 중 위험 출력 Command는 Reject된다.
