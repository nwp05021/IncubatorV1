00_Incubator_System_Architecture.md

Version: 1.0
Status: Authoritative
목적:
본 문서는 Incubator Firmware 전체 시스템의 최상위 아키텍처를 정의한다.
모든 DDU/코드/모듈은 본 문서를 기준으로 설계된다.


1. 시스템 목표
본 시스템은 단순 MCU 예제가 아니다.
목표는 다음과 같다.
상용 수준의Event-Driven,Self-Recovering,Cloud-ConnectedEmbedded Product
구축.

1.1 제품 목표
본 제품은:


자동 부화 제어


환경 유지


상태 복구


원격 관제


안전 제어


를 수행하는 독립형 임베디드 시스템이다.

1.2 핵심 특성
특성설명Real-Time주기 기반 환경 제어Non-Blockingdelay 없는 구조Self-Recovery장애 자동 복구Deterministic예측 가능한 동작State-Driven상태 중심 구조Offline FirstCloud 없어도 동작Centralized Control상태 변경 단일화

2. 시스템 핵심 철학
2.1 Single Source of Truth
가장 중요한 원칙:
RuntimeState가현재 시스템 상태의 유일한 진실이다.

2.2 Centralized State Mutation
상태 변경은 반드시:
AppController
를 통해서만 수행된다.

2.3 Read / Write 분리
Write:    AppControllerRead:    UI    Cloud    Renderer    Telemetry

2.4 Device Layer 원칙
Device는:
하드웨어 접근만 수행
한다.
금지:


정책 판단


상태 변경


알람 발생


Event Publish



2.5 Module Layer 원칙
Module은:


계산


상태 갱신


제어 판단


만 수행.

2.6 UI 원칙
UI는:
현재 상태를 보여주는 것
이 목적이다.
UI는 시스템 상태를 직접 변경하지 않는다.

3. 전체 시스템 구조
3.1 Layered Architecture
┌──────────────────────────────┐│ UI / Cloud                   │├──────────────────────────────┤│ AppController                │├──────────────────────────────┤│ Module Layer                 │├──────────────────────────────┤│ Domain / Policy              │├──────────────────────────────┤│ Storage Layer                │├──────────────────────────────┤│ Device Layer                 │└──────────────────────────────┘

3.2 계층 역할
Layer역할UI사용자 표시Cloud원격 연동AppController상태 변경Module환경 제어Domain비즈니스 모델Storage영속 저장Device하드웨어

4. Runtime Architecture
4.1 시스템 구조
SensorManager    ↓RuntimeState    ↑ClimateModuleScheduler    ↓RuntimeStateAppController    ↓RuntimeStateUI / Cloud    ↑RuntimeState

4.2 핵심 구조
모든 것이 RuntimeState를 중심으로 연결된다.

5. State Architecture
5.1 상태 종류
종류설명AppSettings정적 설정RuntimeState현재 상태IncubationBatch현재 부화 세션IncubationPlanTableDay별 정책

5.2 상태 책임
구조변경 가능 주체AppSettingsAppControllerRuntimeStateModulesBatchAppControllerPlanTableAppController

6. Command Architecture
6.1 핵심 원칙
모든 외부 입력은 Command가 된다.

6.2 입력 소스
입력변환EC11CommandAWS ShadowCommandBLE ProvisioningCommandRecoveryCommand

6.3 흐름
Input    ↓Command    ↓AppController    ↓Validation    ↓State Mutation    ↓Event Notify

6.4 직접 변경 금지
금지:
❌ UI → RuntimeState 직접 변경❌ Cloud → GPIO 직접 제어❌ Device → 상태 변경

7. Tick Architecture
7.1 시스템은 Tick 기반
모든 모듈은 Non-Blocking Tick 기반으로 동작한다.

7.2 Tick 주기
모듈주기Sensor2000msClimate500msTurning1000msScheduler10000msUI Render100msCloud Telemetry60000ms

7.3 금지 사항
❌ delay()❌ vTaskDelay()❌ blocking loop❌ busy wait

8. Boot Architecture
8.1 부팅 순서
Power On    ↓NVS Init    ↓Storage Restore    ↓Device Init    ↓SPIFFS Mount    ↓Plan Validation    ↓Recovery Check    ↓UI Boot Screen    ↓Loop Start

8.2 부팅 철학
실패해도 최대한 살아남는다.

9. Recovery Architecture
9.1 목표
장애 발생 시:
자동 복구 또는 안전 정지

9.2 장애 종류
장애대응Sensor FailSafeModeSPIFFS CorruptRestorePlan MissingRegenerateWatchdog ResetRecovery BootCloud DisconnectOffline Continue

9.3 SafeMode
SafeMode는:
출력 차단 우선
이다.

10. Storage Architecture
10.1 저장 전략
데이터저장 위치SettingsNVSBatchNVSRuntime SnapshotRAMPlanTableSPIFFS

10.2 저장 철학
자주 변하는 데이터는 RAM중요 설정은 NVS대용량 정책은 SPIFFS

11. Policy Architecture
11.1 정책 계층 역할
정책 계층은:
“무엇이 올바른가”
를 정의.

11.2 포함 요소
요소역할Preset조류별 정책DayResolver현재 Day 계산PlanGeneratorDay별 목표 생성

12. UI Architecture
12.1 UI 목적
상태를 빠르고 명확하게 보여준다.

12.2 UI 구조
RuntimeState    ↓UiViewModelBuilder    ↓UiModel    ↓Renderer

12.3 UI 원칙
항목원칙숫자크게색상의미 기반레이아웃단순애니메이션최소정보량낮게

13. Cloud Architecture
13.1 핵심 원칙
Cloud는 “추가 기능”이다.
Cloud 장애로 제품이 멈추면 안 된다.

13.2 Shadow 전략
desired → Commandreported ← RuntimeState

13.3 Offline First
WiFi 없음:
모든 핵심 기능 정상 동작

14. Event Architecture
14.1 이벤트 목적
중요 상태 변화 전달.

14.2 예시
이벤트설명AlarmRaised알람BatchCompleted부화 완료PlanUpdated정책 수정WifiConnected연결 성공

14.3 Event 원칙
Event는:
상태 변경 결과를 알리는 것
이지
상태를 직접 변경하는 것
이 아니다.

15. Concurrency Architecture
15.1 기본 전략
Single Main Loop 기반

15.2 이유
장점:


단순함


디버깅 용이


예측 가능


Race Condition 감소



15.3 향후 확장
필요 시:


UI Task


Cloud Task


분리 가능.
하지만 초기 구조는 단일 루프 유지.

16. Memory Architecture
16.1 핵심 원칙
동적 메모리 최소화

16.2 금지
❌ 반복 new/delete❌ 큰 STL 사용❌ 런타임 메모리 증가

16.3 권장
항목전략UIStatic BufferJSONFixed DocumentTelemetryRingBufferPlanFixed Array

17. Rendering Architecture
17.1 목표
UI 때문에 시스템이 느려지면 실패

17.2 전략
전략목적Dirty Renderredraw 최소화Cached Text성능 향상Bitmap Icon속도 향상Partial UpdateFlicker 감소

18. Embedded Product 철학
18.1 가장 중요한 목표
이 시스템은:
“ESP32 프로젝트”
처럼 보이면 안 된다.

18.2 목표 느낌
사용자는 다음처럼 느껴야 한다.
“완성된 상용 장비”

19. 최종 아키텍처 목표
최종적으로 시스템은 다음 형태를 가진다.
Command-DrivenState-CentricSelf-RecoveringCloud-SynchronizedPremium Embedded Product

20. 최종 원칙
모든 구현은 다음 질문을 통과해야 한다.
“이 구조가장기 유지보수 가능한가?”
가능하지 않다면 구조를 다시 설계한다.
