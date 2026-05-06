# DDU-RUNTIME-001 — RuntimeState Standard Model

> Version: 1.0
> Status: Draft
> Dependency: System Architecture, Domain Model, RuntimeState Reference
> Estimated Time: 20~30 min

## 목적

이 DDU 완료 후 다음이 가능해야 한다.

- 현재 온도/습도/목표값/출력/SafeMode/Alarm/Batch 상태를 하나의 구조에서 읽는다.
- UI, Cloud, Alarm, Debug가 동일한 RuntimeState를 기준으로 동작한다.
- RuntimeState는 RAM 전용 Read Model로 유지한다.

## 생성/수정 파일

```text
include/domain/RuntimeState.h
```

## 핵심 구조

```text
Sensor / Scheduler / Climate / Recovery
    ↓
RuntimeState
    ↓
UI / Cloud / Alarm / Debug
```

## 상태 소유권

- RuntimeState는 현재 상태의 Single Source of Truth다.
- UI/Cloud/Renderer는 읽기만 한다.
- 실제 상태 변경은 Module 또는 AppController 정책을 통해서만 수행한다.

## 금지 사항

```text
❌ UI가 RuntimeState 직접 수정
❌ Cloud가 RuntimeState 직접 수정
❌ Device가 RuntimeState 접근
❌ RuntimeState를 NVS에 전체 저장
```

## Acceptance Criteria

AC-1. `RuntimeState::zero()`로 안전 초기화 가능.

AC-2. Home UI에 필요한 현재값/목표값/출력상태/SafeMode/Alarm/Batch 정보가 모두 존재.

AC-3. Cloud Telemetry reported 생성에 필요한 값이 모두 존재.
