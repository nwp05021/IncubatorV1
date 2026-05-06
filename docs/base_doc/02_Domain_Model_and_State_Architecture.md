02_Domain_Model_and_State_Architecture.md

Version: 1.0
Status: Authoritative
목적:
본 문서는 시스템 전체의 Domain Model 및 상태(State) 아키텍처를 정의한다.
모든 상태 흐름과 데이터 구조는 본 문서를 기준으로 설계된다.


1. 문서 목적
이 문서는 단순 구조체 정의 문서가 아니다.
다음을 정의한다.
- 시스템 상태 구조- 상태 소유권- 상태 변경 규칙- 데이터 흐름- Single Source of Truth- Runtime Architecture

2. 핵심 철학
2.1 Single Source of Truth
가장 중요한 원칙:
RuntimeState는현재 시스템 상태의 유일한 진실이다.

2.2 상태 분리
시스템 상태는 반드시 다음처럼 분리한다.
상태 종류목적Settings정적 운영 설정RuntimeState실시간 상태Batch현재 부화 세션PlanDay별 정책

2.3 왜 중요한가?
이 분리가 되어야:


상태 꼬임 방지


복구 단순화


Cloud 동기화 단순화


UI 안정화


유지보수 향상


이 가능하다.

3. 전체 상태 구조
3.1 구조 개요
AppSettings    ↓Climate LogicIncubationPlanTable    ↓SchedulerIncubationBatch    ↓Current DayRuntimeState    ↑Everything

3.2 핵심 구조
모든 읽기는 RuntimeState모든 쓰기는 AppController

4. 상태 종류 정의
4.1 AppSettings
역할
정적 운영 설정.

특성
특성설명변경 빈도낮음저장 위치NVS수정 주체AppController재부팅 유지O

포함 예시
tempHysteresishumidityHysteresisalarmEnabledtelemetryIntervalturningDuration

4.2 RuntimeState
역할
현재 시스템의 실시간 상태.

가장 중요한 구조
시스템 현재 상태의 중심

특성
특성설명변경 빈도매우 높음저장 위치RAM읽기 주체UI/Cloud갱신 주체Modules

포함 예시
currentTempCcurrentHumidityPctheaterOnhumidifierOncurrentDaysafeModealarmActive

4.3 IncubationBatch
역할
현재 진행 중인 부화 세션.

특성
특성설명변경 빈도낮음저장 위치NVS재부팅 유지O

포함 예시
speciesstartEpochtotalDayslockdownStartDaybatchId

4.4 IncubationPlanTable
역할
Day 기반 목표 정책.

특성
특성설명변경 빈도낮음저장 위치SPIFFS구조Array/Table

포함 예시
Day 1:  Temp 37.8  Humi 55Day 19:  Temp 37.2  Humi 65

5. RuntimeState Architecture
5.1 가장 중요한 구조
RuntimeState는Read Model이다.

5.2 RuntimeState 목적
목적설명UI 표시현재 상태Cloud ReportreportedTelemetry발행Alarm 판단현재값 기반Debug상태 확인

5.3 RuntimeState 변경 규칙
허용
주체허용ModuleOSchedulerORecoveryO

금지
주체금지 이유UI상태 꼬임Renderer책임 위반Cloud Direct보안 문제DeviceLayer 위반

6. 상태 변경 흐름
6.1 핵심 흐름
Input    ↓Command    ↓AppController    ↓Validation    ↓State Mutation    ↓RuntimeState Update    ↓Event Notify

6.2 중요한 원칙
상태 변경은 반드시 중앙 집중화

7. Command Model
7.1 목적
모든 외부 입력을 통일.

7.2 구조
struct Command{    CommandType type;    uint32_t timestamp;    union payload;};

7.3 입력 소스
입력변환EC11CommandAWSCommandBLECommandRecoveryCommand

8. Scheduler Data Flow
8.1 흐름
Batch    ↓Current Day    ↓Plan Row    ↓RuntimeState Targets

8.2 결과
RuntimeState에는 항상:
현재 적용 중인 목표값
이 존재해야 한다.

9. Climate Control Data Flow
9.1 흐름
Sensor Value    ↓RuntimeState    ↓ClimateModule    ↓Output State    ↓GPIO

9.2 핵심 원칙
Device는 단순 출력만

10. UI Data Flow
10.1 구조
RuntimeState    ↓UiViewModelBuilder    ↓UiModel    ↓Renderer

10.2 중요한 원칙
UI는:
RuntimeState 직접 수정 금지

11. Cloud Data Flow
11.1 Reported
RuntimeState    ↓TelemetryBuilder    ↓AWS reported

11.2 Desired
AWS desired    ↓Command    ↓AppController

12. State Ownership
12.1 핵심 원칙
모든 상태는 Owner가 명확해야 한다.

12.2 Ownership Table
상태OwnerAppSettingsAppControllerBatchAppControllerPlanTableAppControllerRuntimeStateModules

13. State Persistence
13.1 저장 전략
상태저장 위치SettingsNVSBatchNVSPlanSPIFFSRuntimeRAM

13.2 저장 철학
실시간 상태는 RAM복구 필요 상태는 영구 저장

14. Event Architecture
14.1 목적
상태 변화 전달.

14.2 예시
Event의미AlarmRaised알람 발생BatchStarted부화 시작BatchCompleted부화 완료PlanUpdated정책 변경

14.3 Event 원칙
Event는 결과 통지
이지
상태 변경 명령
이 아니다.

15. Validation Architecture
15.1 목적
잘못된 상태 진입 방지.

15.2 예시
검증이유Day RangePlan 오류 방지Temp Range위험 방지Humi Range센서 이상 방지

15.3 Validation 위치
모든 Validation은 AppController

16. Recovery State Strategy
16.1 부팅 시 복구
NVS Restore    ↓Plan Load    ↓Validation    ↓SafeMode 판단

16.2 실패 전략
실패대응Plan MissingRegenerateCorrupt SettingsDefaultsInvalid BatchIgnoreSensor FailSafeMode

17. SafeMode State
17.1 목적
치명 오류 보호.

17.2 SafeMode 활성 조건
조건예시Sensor FailAHT20 응답 없음Plan InvalidDay Row 없음Critical Alarm장시간 고온

17.3 SafeMode 행동
출력 차단 우선

18. Memory Strategy
18.1 중요한 원칙
상태 구조는 가능한 고정 크기

18.2 금지
❌ dynamic growth❌ uncontrolled allocation❌ runtime vector expansion

19. Embedded Product 관점
19.1 중요한 이유
좋은 상태 구조는:


버그 감소


Recovery 단순화


UI 안정화


Cloud 단순화


를 만든다.

19.2 가장 중요한 목표
상태 흐름이 인간이 이해 가능한 수준이어야 한다.

20. 최종 상태 철학
최종적으로 시스템은:
Command-DrivenState-CentricSingle Source of Truth
구조를 유지해야 한다.

21. 최종 원칙
모든 구현은 다음 질문을 통과해야 한다.
“이 상태는누가 소유하는가?”
Owner가 불명확하면 구조를 다시 설계한다.