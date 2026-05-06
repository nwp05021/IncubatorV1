# DDU-APP-001 — AppSettings Standard Model

> Version: 1.0
> Status: Draft
> Dependency: Storage Strategy, AppSettings Reference
> Estimated Time: 20~30 min

## 목적

이 DDU 완료 후 다음이 가능해야 한다.

- 정적 운영 설정을 하나의 구조로 관리한다.
- 설정 변경은 Command → AppController 경로로만 수행한다.
- 설정값은 NVS에 저장하고, 부팅 시 검증 후 복구한다.

## 생성/수정 파일

```text
include/domain/AppSettings.h
src/app/AppController.cpp
```

## 핵심 구조

```text
Command(UpdateSettings)
    ↓
AppController Validation
    ↓
AppSettings Mutation
    ↓
NVS Save
```

## Validation 기준

| 항목 | 범위 |
|---|---|
| tempHysteresis | 0.1 ~ 2.0 |
| humidityHysteresis | 1.0 ~ 10.0 |
| telemetryIntervalMs | >= 10000 |
| turningDurationSec | 1 ~ 600 |

## 금지 사항

```text
❌ UI → NVS 직접 저장
❌ Cloud → Settings 직접 변경
❌ Module → Settings 저장
```

## Acceptance Criteria

AC-1. 손상/비정상 설정은 기본값으로 복구된다.

AC-2. 유효하지 않은 설정 변경 Command는 상태를 변경하지 않는다.

AC-3. 설정 변경 성공 시 즉시 NVS 저장을 수행한다.
