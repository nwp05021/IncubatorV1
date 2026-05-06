06_Cloud_Shadow_Architecture.md

Version: 1.0
Status: Authoritative
목적:
본 문서는 AWS IoT 기반 Cloud 연동 구조와
Device Shadow 동기화 전략을 정의한다.


1. 문서 목적
본 문서는 단순 MQTT 연결 문서가 아니다.
다음을 정의한다.
- Cloud Layer 역할- AWS IoT 구조- Device Shadow 전략- Telemetry 구조- Offline First 정책- Command 흐름- Cloud Recovery

2. 핵심 철학
2.1 가장 중요한 원칙
Cloud는 핵심 기능이 아니다.

2.2 의미
WiFi 또는 AWS 장애가 발생해도:


환경 제어


부화 스케줄


전란


UI


는 정상 동작해야 한다.

2.3 Offline First
Cloud 없이도완전한 독립 동작 가능
해야 한다.

3. Cloud Layer 역할
역할설명Telemetry Publish상태 발행Device Shadow Syncdesired/reportedRemote Command원격 명령Connection Recovery자동 재연결Time SyncNTP 보조

4. 전체 구조
RuntimeState    ↓TelemetryBuilder    ↓AwsIotClient    ↓AWS IoT CoreAWS desired    ↓CmdParser    ↓Command    ↓AppController

5. Layered Architecture
┌──────────────────────────────┐│ AWS IoT Core                 │├──────────────────────────────┤│ AwsIotClient                 │├──────────────────────────────┤│ TelemetryBuilder             ││ CmdParser                    │├──────────────────────────────┤│ AppController                │├──────────────────────────────┤│ RuntimeState                 │└──────────────────────────────┘

6. Cloud 구성 요소
구성역할WifiManagerWiFi 연결AwsIotClientMQTT/TLSTelemetryBuilderJSON 생성CmdParserJSON → CommandShadowSyncManagerreported sync

7. Device Shadow 전략
7.1 핵심 원칙
desired → Commandreported ← RuntimeState

7.2 매우 중요한 이유
이 구조가 되어야:


로컬 UI 충돌 방지


Cloud 직접 제어 방지


상태 일관성 유지


Single Source 유지


가능.

7.3 금지
❌ Cloud → GPIO 직접 제어❌ Cloud → RuntimeState 직접 변경

8. Shadow desired 흐름
AWS desired    ↓MQTT Subscribe    ↓CmdParser    ↓Command    ↓AppController    ↓Validation    ↓Mutation

9. Shadow reported 흐름
RuntimeState    ↓TelemetryBuilder    ↓reported JSON    ↓AWS Shadow

10. Telemetry Architecture
10.1 목적
현재 상태 전달.

10.2 포함 데이터
데이터예시Temp37.5Humi65HeatertrueDay7AlarmfalseSafeModefalse

10.3 Telemetry 철학
Telemetry는 “현재 상태”
이지
내부 구현 세부사항
이 아니다.

11. MQTT Topic 구조
11.1 Publish
incubator/{deviceId}/telemetry

11.2 Subscribe
incubator/{deviceId}/cmd

11.3 Shadow
$aws/things/{deviceId}/shadow/update$aws/things/{deviceId}/shadow/update/delta

12. JSON 구조
12.1 Telemetry 예시
{  "deviceId": "INC-001",  "day": 7,  "sensor": {    "tempC": 37.5,    "humidityPct": 65.0  },  "target": {    "tempC": 37.8,    "humidityPct": 60.0  },  "actuator": {    "heater": true,    "humidifier": false,    "turner": false  },  "safeMode": false}

12.2 Command 예시
{  "cmd": "PATCH_PLAN_ROW",  "payload": {    "day": 8,    "targetTempC": 37.6,    "targetHumidityPct": 58.0  }}

13. Cloud Command Architecture
13.1 매우 중요한 원칙
Cloud는 특권 계층이 아니다.

13.2 의미
Cloud도:


UI


BLE


Recovery


와 동일한 Command 규칙을 따른다.

13.3 흐름
Cloud Input    ↓Command    ↓AppController

14. Cloud Validation
14.1 목적
위험 제어 방지.

14.2 예시
명령검증Heater ONSafeMode 차단Plan PatchDay 범위SettingsTemp 범위

14.3 실패 시
Command Reject
후:


Warning Event


Cloud Error Response


UI Notification 가능



15. Connection Recovery
15.1 목표
자동 재연결.

15.2 흐름
Disconnect    ↓Retry Timer    ↓Reconnect    ↓Resubscribe

15.3 중요한 원칙
Cloud 장애로 Main Loop 정지 금지

16. Retry 정책
항목값WiFi Retry30초MQTT Retry15초TLS FailureBackoffDNS FailureRetry

17. Offline Queue (권장)
17.1 목적
WiFi 단절 시 Telemetry 손실 감소.

17.2 구조
Telemetry RingBuffer

17.3 정책
정책설명고정 크기필수FIFO기본Overflow오래된 데이터 Drop

18. Time Synchronization
18.1 NTP 역할
역할설명Batch Day 계산Epoch 기반Telemetry TimestampCloud 시간Recovery부팅 시간

18.2 오프라인 전략
RTC + Last NTP Sync
전략 사용.

19. Security Architecture
19.1 TLS 필수
MQTT는 TLS 사용.

19.2 인증서 전략
권장:
embed_files

19.3 금지
❌ Plain MQTT❌ Hardcoded Secret in Source❌ Debug Secret Print

20. ShadowSyncManager (권장)
20.1 목적
reported 관리 단순화.

20.2 역할
RuntimeState    ↓reported JSON 생성    ↓Dirty Sync

20.3 장점


불필요 Publish 감소


Cloud 비용 감소


트래픽 감소



21. Telemetry Rate 정책
항목권장기본 Telemetry60초Alarm 발생즉시SafeMode즉시Plan 변경즉시

22. Alarm / SafeMode Cloud 전략
22.1 Alarm
Alarm 발생 시:
Immediate Publish
권장.

22.2 SafeMode
SafeMode는 반드시:
reported.safeMode = true
동기화.

23. UI / Cloud 관계
23.1 중요한 원칙
UI와 Cloud는 동등한 Client
이다.

23.2 의미
둘 다:
Command → AppController
흐름을 사용.

24. Embedded Product 철학
24.1 중요한 목표
좋은 IoT 제품은:
Cloud가 없어도쓸 수 있어야 한다.

24.2 Cloud의 역할
Cloud는:
제품을 “확장”
하는 것이다.
제품 자체가 되어선 안 된다.

25. 최종 구조
최종적으로 Cloud 구조는 다음을 유지한다.
RuntimeState    ↓reporteddesired    ↓Command    ↓AppController

26. 최종 원칙
모든 Cloud 구현은 다음 질문을 통과해야 한다.
WiFi가 끊겨도제품은 정상 동작하는가?
정상 동작하지 않으면 구조를 다시 설계한다.