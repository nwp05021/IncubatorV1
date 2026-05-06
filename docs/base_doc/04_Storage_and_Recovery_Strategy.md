04_Storage_and_Recovery_Strategy.md

Version: 1.0
Status: Authoritative
목적:
본 문서는 Incubator Firmware의 영구 저장, 부팅 복구, 장애 대응, SafeMode 전략을 정의한다.


1. 핵심 원칙
저장은 최소화하고,복구는 확실하게 하며,장애 시 출력 차단을 우선한다.

2. 저장 데이터 분류
데이터구조체저장 위치성격운영 설정AppSettingsNVS정적 설정부화 세션IncubationBatchNVS복구 필수일차별 계획IncubationPlanTableSPIFFS JSON대용량 정책실시간 상태RuntimeStateRAM휘발성UI 세션UiSessionStateNVS 선택UX 복원용

3. 저장소 역할
3.1 NVS
NVS는 작고 중요한 설정을 저장한다.
저장 대상:
AppSettingsIncubationBatchBootCountWiFi CredentialsRecovery Flags

3.2 SPIFFS
SPIFFS는 구조가 크거나 사람이 읽을 수 있는 데이터를 저장한다.
저장 대상:
IncubationPlanTableplan.jsondiagnostic log 선택

4. 저장 원칙
4.1 RuntimeState는 저장하지 않는다
RuntimeState는 현재 상태 스냅샷이다.
따라서 기본적으로 저장하지 않는다.
RuntimeState = RAM Only
예외적으로 복구가 꼭 필요한 일부 값만 별도 Snapshot으로 저장할 수 있다.

4.2 Settings / Batch / Plan은 AppController만 저장한다
저장 요청은 반드시 AppController를 통과한다.
금지:
UI → NVS 직접 저장 금지Cloud → SPIFFS 직접 저장 금지Module → 설정 직접 저장 금지

5. Schema Version 전략
모든 영구 데이터에는 version이 필요하다.
struct PersistentHeader{    uint32_t magic;    uint16_t schemaVersion;    uint16_t size;};
목적:


펌웨어 업데이트 대응


구조체 크기 변경 대응


손상 데이터 감지


기본값 복구



6. Atomic Save 전략
SPIFFS 저장은 반드시 원자적 저장을 사용한다.
plan.tmp 저장    ↓flush    ↓plan.bak 백업    ↓plan.json 교체
권장 순서:
1. /spiffs/plan.tmp 생성2. JSON serialize3. 파일 close4. 기존 plan.json → plan.bak5. plan.tmp → plan.json6. 검증 load
전원 차단 시 손상 가능성을 줄인다.

7. 부팅 복구 순서
Power On    ↓NVS Init    ↓BootCount 증가    ↓AppSettings Load    ↓IncubationBatch Load    ↓SPIFFS Mount    ↓Plan Load    ↓Plan Validation    ↓Device Init    ↓RuntimeState 초기화    ↓SafeMode 판단    ↓Main Loop 시작

8. 복구 정책
상황대응Settings 없음기본값 생성Settings 손상기본값 생성 + WarningBatch 없음비활성 BatchBatch 손상Batch 무효화Plan 없음Preset 재생성Plan 손상plan.bak 복구plan.bak도 손상Preset 재생성Sensor 실패SafeModeWDT 반복 리셋Boot SafeMode

9. Watchdog Recovery
9.1 목적
시스템이 멈췄을 때 자동 재부팅한다.

9.2 정책
Watchdog Timeout = 5초

9.3 WDT Reset 감지
부팅 시 reset reason을 확인한다.
WDT Reset 발생    ↓Recovery Flag 기록    ↓BootCount / WdtResetCount 증가

9.4 반복 리셋 보호
짧은 시간 안에 WDT Reset이 반복되면 SafeMode로 진입한다.
예:
10분 이내 WDT Reset 3회 이상    ↓Boot SafeMode

10. SafeMode 전략
10.1 SafeMode 목적
SafeMode는 시스템 보호 모드다.
가장 중요한 원칙:
출력 차단 우선

10.2 SafeMode 진입 조건
조건예시센서 장애AHT20 응답 없음Plan 오류현재 Day Row 없음저장소 오류Plan 복구 실패반복 WDT부팅 루프고온 치명 알람장시간 위험 온도설정 범위 오류비정상 목표값

10.3 SafeMode 동작
SafeMode에서는:
Heater OFFHumidifier OFFTurner OFFFan 정책 모드Buzzer/Alarm ON 가능UI SafeMode Overlay 표시Cloud reported에 safeMode=true 반영

11. SafeMode 해제
SafeMode 해제는 반드시 AppController Command로 처리한다.
ClearSafeMode Command    ↓Validation    ↓Sensor 정상 확인    ↓Plan 정상 확인    ↓출력 재개 허용
센서나 Plan이 여전히 비정상이면 해제 거부.

12. Plan Recovery
Plan 복구 순서:
1. plan.json load2. schema/version 검증3. rowCount 검증4. 현재 Batch totalDays와 비교5. 실패 시 plan.bak load6. 실패 시 Preset 재생성7. 재생성 실패 시 SafeMode

13. Settings Recovery
Settings 복구 순서:
1. NVS blob load2. size/version 검증3. value range 검증4. 실패 시 defaultSettings()5. NVS에 기본값 재저장

14. Batch Recovery
Batch는 부화 진행 복구의 핵심이다.
검증 항목:
active 여부species 유효성startEpoch 유효성totalDays 범위lockdownStartDay 범위batchId 존재 여부
Batch가 무효이면 부화 비활성 상태로 시작한다.

15. WiFi / Cloud Recovery
Cloud는 핵심 기능이 아니다.
따라서 WiFi 실패는 SafeMode 조건이 아니다.
WiFi Fail    ↓Local Mode 유지    ↓재시도
Cloud 장애로 환경 제어가 중단되면 안 된다.

16. 저장 주기 정책
데이터저장 시점Settings변경 즉시BatchStart/Stop 즉시Plan수정 즉시WiFiProvisioning 성공 즉시RuntimeState저장 안 함Diagnostic선택적 주기 저장

17. 저장 실패 처리
저장 실패 시:
1. Command 실패 반환2. Event 발행3. UI Warning 표시4. 필요 시 SafeMode 판단
Plan 저장 실패는 심각하다.
Settings 저장 실패는 Warning으로 시작하되 반복 시 SafeMode 검토.

18. Factory Reset
Factory Reset은 다음을 삭제한다.
NVS SettingsNVS BatchWiFi CredentialsSPIFFS PlanRecovery FlagsUI Session
Factory Reset 후:
기본 Settings 생성Batch inactivePlan empty재부팅 권장

19. Diagnostic 정보
최소 진단 정보:
BootCountLastResetReasonWdtResetCountLastErrorCodeLastSafeModeReasonLastStorageError
이 정보는 System Page와 Cloud reported에 표시 가능해야 한다.

20. 최종 원칙
모든 저장/복구 설계는 다음 질문을 통과해야 한다.
전원이 이 순간 꺼져도다음 부팅에서 안전하게 복구되는가?
그렇지 않으면 저장 전략을 다시 설계한다.