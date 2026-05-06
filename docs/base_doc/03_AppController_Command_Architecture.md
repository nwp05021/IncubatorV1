03_AppController_Command_Architecture.md

Version: 1.0
Status: Authoritative
목적:
본 문서는 시스템 전체의 상태 변경 아키텍처와
AppController 기반 Command 처리 구조를 정의한다.


1. 문서 목적
이 문서는 단순 Controller 문서가 아니다.
다음을 정의한다.
- 상태 변경 유일 통로- Command 처리 구조- Validation- State Mutation- Event Notification- Observer 구조
즉:
시스템의 중앙 제어실(Command Center)
을 정의하는 문서이다.

2. 핵심 철학
2.1 가장 중요한 원칙
모든 상태 변경은 AppController를 통과한다.

2.2 이유
이 구조가 되어야:


상태 꼬임 방지


Race 감소


Validation 중앙화


Recovery 단순화


Cloud/UI 충돌 제거


가 가능하다.

2.3 절대 금지
❌ UI → RuntimeState 직접 변경❌ Cloud → GPIO 직접 제어❌ Module → Settings 직접 저장❌ Device → 상태 변경

3. AppController 역할
3.1 핵심 책임
책임설명Command 처리외부 입력 수신Validation유효성 검사State Mutation상태 변경Persistence저장 요청Event Notify상태 변화 알림

3.2 비책임 영역
AppController는 다음을 수행하지 않는다.
금지이유GPIO 제어Device 책임UI DrawUI 책임Sensor ReadModule 책임MQTT 직접 처리Cloud 책임

4. Command Architecture
4.1 핵심 구조
Input    ↓Command    ↓Command Queue    ↓AppController    ↓Validation    ↓Mutation    ↓Event

4.2 Command의 목적
모든 입력을:
동일한 구조로 통일
한다.

4.3 입력 소스
Source변환EC11CommandAWS ShadowCommandBLECommandRecoveryCommandSchedulerCommand 가능

5. Command Queue
5.1 매우 중요한 구조
권장:
Queue 기반 처리

5.2 이유
동시에:


UI 입력


Cloud 입력


Recovery 입력


이 들어올 수 있기 때문.

5.3 핵심 목표
Single Threaded State Mutation
보장.

5.4 구조 예시
struct CommandQueue{    Command buffer[32];    uint8_t head;    uint8_t tail;};

5.5 Queue 정책
정책설명FIFO기본고정 크기필수OverflowDrop + Alarm동적 증가금지

6. Command Model
6.1 구조 예시
enum class CommandType{    StartBatch,    StopBatch,    UpdateSettings,    PatchPlanRow,    ToggleManualHeater,    ClearSafeMode};

6.2 Command Payload
struct Command{    CommandType type;    uint32_t timestamp;    CommandSource source;    union    {        StartBatchPayload startBatch;        PatchPlanPayload patchPlan;        SettingsPayload settings;    };};

6.3 Source 추적
매우 중요.
누가 상태를 변경했는가?
를 추적 가능해야 한다.

7. AppController Lifecycle
7.1 흐름
enqueue()    ↓process()    ↓validate()    ↓mutate()    ↓persist()    ↓notify()

7.2 Tick 기반
void AppController::tick(){    processQueue();}

8. Validation Architecture
8.1 가장 중요한 역할 중 하나
Validation 실패 시:
절대 상태 변경 금지

8.2 검증 항목
검증목적Temp Range위험 방지Day RangePlan 보호SafeMode 상태출력 보호Lockdown 상태전란 방지

8.3 예시
if (runtime.safeMode){    reject(CommandType::ToggleManualHeater);}

9. Mutation Architecture
9.1 핵심 원칙
Mutation은 AppController만 수행

9.2 직접 변경 금지
❌ UI direct mutation❌ Cloud direct mutation❌ Device mutation

9.3 Mutation 예시
runtime.manualHeater = true;settings.tempHysteresis = 0.5f;batch.active = true;

10. Persistence Architecture
10.1 목적
상태 변경 후 저장.

10.2 저장 정책
상태저장Settings즉시Batch즉시Plan즉시RuntimeState저장 안 함

10.3 Atomic Save
필수.
tmp → rename()
전략 권장.

11. Event Notification
11.1 목적
상태 변경 결과 전달.

11.2 구조
Mutation    ↓Event    ↓Observers

11.3 Event 예시
Event의미BatchStarted부화 시작AlarmRaised알람PlanUpdated정책 수정WifiConnected연결

11.4 Event 원칙
Event는 결과 통지
이다.

12. Observer Architecture
12.1 목적
UI/Cloud 동기화.

12.2 예시
PlanUpdated    ↓UI Refresh    ↓Cloud Sync

12.3 Polling 최소화
중요 이벤트는:
Observer/Event 기반
권장.

13. SafeMode Protection
13.1 목적
위험 출력 차단.

13.2 예시
명령SafeMode 시Heater ON거부Turner ON거부Fan PWM제한

13.3 핵심 원칙
SafeMode는출력 차단 우선

14. Manual Control Protection
14.1 위험 제어
Manual Mode는 위험 기능.

14.2 보호 전략
기능보호Heater ONLong HoldFactory ResetDouble ConfirmSafeMode ClearValidation

15. Cloud Integration Strategy
15.1 핵심 구조
AWS desired    ↓Command    ↓AppController

15.2 중요한 원칙
Cloud는:
특권 계층이 아니다.
즉:


로컬 UI


Cloud


BLE


모두 동일 규칙 적용.

16. UI Integration Strategy
16.1 구조
UI Action    ↓Command    ↓AppController

16.2 UI 직접 제어 금지
❌ UI → GPIO❌ UI → RuntimeState

17. Recovery Integration
17.1 Recovery도 Command 사용
권장:
Recovery 역시 Command 기반

17.2 이유
구조 통일.

17.3 예시
Recovery:    ClearAlarm    RestartWifi    RegeneratePlan

18. Command Priority (향후 확장)
18.1 우선순위 가능
향후 필요 시:
Priority예시CriticalSafeModeHighAlarmNormalUILowTelemetry

18.2 초기 버전
초기 버전은:
단순 FIFO
권장.

19. Debug Architecture
19.1 매우 중요
모든 Command는 로그 가능해야 한다.

19.2 예시
[CMD]Source=CloudType=PatchPlanResult=OK

19.3 장점


디버깅 용이


상태 추적 가능


장애 분석 가능



20. Embedded Product 철학
20.1 중요한 이유
좋은 제품은:
상태 변경 경로가 단순하다.

20.2 AppController 목표
AppController는:
“시스템의 교도소장”
이다.
모든 상태 변경을 통제한다.

21. 최종 구조
최종적으로 시스템은 다음 흐름을 가진다.
Input    ↓Command    ↓Queue    ↓AppController    ↓Validation    ↓Mutation    ↓Event    ↓UI / Cloud

22. 최종 원칙
모든 구현은 다음 질문을 통과해야 한다.
“이 상태 변경은AppController를 통과했는가?”
통과하지 않았다면 구조를 다시 설계한다.