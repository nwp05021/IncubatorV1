01_Requirement_Definition.md

Version: 1.0
Status: Authoritative
목적:
본 문서는 Incubator Firmware 제품의 기능적/비기능적 요구사항을 정의한다.
모든 설계 및 구현은 본 문서를 기준으로 수행된다.

1. 제품 개요
1.1 제품 목적

본 제품은:

정밀 환경 제어 기반의
스마트 자동 부화 시스템

구축을 목표로 한다.

1.2 주요 기능
기능	설명
온도 제어	목표 온도 유지
습도 제어	목표 습도 유지
전란 제어	일정 기반 자동 전란
부화 스케줄	Day 기반 목표 변경
로컬 UI	EC11 + TFT 제어
Cloud 연동	AWS IoT
상태 복구	전원 복구 대응
장애 대응	SafeMode
1.3 제품 철학

제품은 다음 특성을 가져야 한다.

특성	목표
안정성	최우선
직관성	빠른 상태 이해
유지보수성	장기 운영 가능
확장성	센서/Cloud 확장
독립성	Cloud 없어도 동작
2. 기능 요구사항 (Functional Requirements)
2.1 환경 제어
FR-ENV-001

시스템은 현재 온도를 주기적으로 측정해야 한다.

FR-ENV-002

시스템은 현재 습도를 주기적으로 측정해야 한다.

FR-ENV-003

시스템은 목표 온도를 유지해야 한다.

FR-ENV-004

시스템은 목표 습도를 유지해야 한다.

FR-ENV-005

제어는 히스테리시스 기반이어야 한다.

FR-ENV-006

센서 실패 시 출력은 안전 방향으로 정지해야 한다.

2.2 스케줄 기반 부화 정책
FR-PLAN-001

시스템은 조류별 Preset을 제공해야 한다.

FR-PLAN-002

시스템은 Day 기반 목표값 변경을 지원해야 한다.

FR-PLAN-003

시스템은 Lockdown 구간을 지원해야 한다.

FR-PLAN-004

Lockdown 구간에서는 전란을 중단해야 한다.

FR-PLAN-005

사용자는 특정 Day 목표값을 수정할 수 있어야 한다.

FR-PLAN-006

수정된 Plan은 영구 저장되어야 한다.

2.3 전란 제어
FR-TURN-001

시스템은 설정된 간격마다 전란을 수행해야 한다.

FR-TURN-002

전란 시간은 설정 가능해야 한다.

FR-TURN-003

SafeMode 시 전란은 중단되어야 한다.

FR-TURN-004

수동 전란 Trigger를 지원해야 한다.

2.4 로컬 UI
FR-UI-001

시스템은 ST7789 기반 UI를 제공해야 한다.

FR-UI-002

시스템은 EC11 기반 입력을 지원해야 한다.

FR-UI-003

사용자는 현재 상태를 1초 이내에 이해 가능해야 한다.

FR-UI-004

UI는 현재 온도/습도를 크게 표시해야 한다.

FR-UI-005

UI는 현재 Day 진행 상태를 표시해야 한다.

FR-UI-006

UI는 Manual Control 화면을 제공해야 한다.

FR-UI-007

UI는 Plan Edit 기능을 제공해야 한다.

FR-UI-008

UI는 Alarm Overlay를 제공해야 한다.

FR-UI-009

UI는 SafeMode Overlay를 제공해야 한다.

2.5 Cloud 연동
FR-CLOUD-001

시스템은 AWS IoT MQTT 연동을 지원해야 한다.

FR-CLOUD-002

시스템은 Telemetry 발행을 지원해야 한다.

FR-CLOUD-003

시스템은 Device Shadow 동기화를 지원해야 한다.

FR-CLOUD-004

Cloud 명령은 Command 구조로 처리되어야 한다.

FR-CLOUD-005

WiFi 연결 실패 시에도 시스템은 정상 동작해야 한다.

2.6 상태 복구
FR-REC-001

시스템은 재부팅 후 이전 상태를 복원해야 한다.

FR-REC-002

Batch 정보는 영구 저장되어야 한다.

FR-REC-003

Plan 정보는 영구 저장되어야 한다.

FR-REC-004

설정값은 영구 저장되어야 한다.

FR-REC-005

Plan 손상 시 Preset 재생성이 가능해야 한다.

FR-REC-006

심각한 오류 시 SafeMode로 진입해야 한다.

2.7 알람
FR-ALARM-001

시스템은 고온 상태를 감지해야 한다.

FR-ALARM-002

시스템은 저온 상태를 감지해야 한다.

FR-ALARM-003

시스템은 고습 상태를 감지해야 한다.

FR-ALARM-004

시스템은 저습 상태를 감지해야 한다.

FR-ALARM-005

알람은 확인 시간 이후 활성화되어야 한다.

FR-ALARM-006

알람 발생 시 사용자에게 시각적으로 표시되어야 한다.

3. 비기능 요구사항 (Non-Functional Requirements)
3.1 성능
NFR-PERF-001

센서 Polling은 2초 이하 주기로 수행되어야 한다.

NFR-PERF-002

Climate Control Tick은 500ms 이하 간격이어야 한다.

NFR-PERF-003

UI Render는 100ms 이하 응답성을 가져야 한다.

NFR-PERF-004

UI는 Control Loop를 방해하면 안 된다.

3.2 안정성
NFR-REL-001

시스템은 Non-Blocking 구조여야 한다.

NFR-REL-002

시스템은 delay() 사용을 금지한다.

NFR-REL-003

Watchdog Recovery를 지원해야 한다.

NFR-REL-004

Cloud 장애는 핵심 기능에 영향을 주면 안 된다.

NFR-REL-005

센서 실패 시 안전 정지를 우선해야 한다.

3.3 유지보수성
NFR-MNT-001

모든 상태 변경은 AppController를 통해야 한다.

NFR-MNT-002

Device Layer는 비즈니스 로직을 가지면 안 된다.

NFR-MNT-003

UI는 RuntimeState를 직접 수정하면 안 된다.

NFR-MNT-004

Cloud는 GPIO를 직접 제어하면 안 된다.

NFR-MNT-005

모든 Layer는 책임이 분리되어야 한다.

3.4 메모리
NFR-MEM-001

반복적인 동적 메모리 할당을 금지한다.

NFR-MEM-002

UI는 Static Buffer 기반이어야 한다.

NFR-MEM-003

Telemetry Queue는 고정 크기여야 한다.

3.5 UX
NFR-UX-001

사용자는 현재 상태를 빠르게 이해 가능해야 한다.

NFR-UX-002

현재 온도/습도는 가장 우선적으로 표시되어야 한다.

NFR-UX-003

위험 동작은 Long Hold 기반이어야 한다.

NFR-UX-004

알람은 즉시 인지 가능해야 한다.

NFR-UX-005

UI는 “DIY 장치”처럼 보이면 안 된다.

4. 시스템 제약사항 (Constraints)
4.1 하드웨어
항목	내용
MCU	ESP32-S3
Display	ST7789 320x240
Input	EC11
Sensor	AHT20
Storage	NVS + SPIFFS
4.2 프레임워크
항목	내용
Framework	ESP-IDF + Arduino
Language	C++17
Build	PlatformIO
4.3 금지 사항
❌ delay()
❌ blocking while loop
❌ UI direct GPIO control
❌ Device business logic
❌ RuntimeState direct mutation from UI
5. 시스템 운영 모드
5.1 Normal Mode

정상 운영 상태.

5.2 Alarm Mode

경고 상태.

5.3 SafeMode

치명 오류 상태.

출력 우선 차단.

5.4 Manual Mode

사용자 강제 제어 상태.

6. 시스템 입력
6.1 로컬 입력
입력	설명
Encoder Rotate	이동
Encoder Click	선택
Encoder Hold	뒤로/승인
6.2 원격 입력
입력	설명
AWS Shadow Desired	설정 변경
MQTT Command	제어 명령
7. 시스템 출력
7.1 물리 출력
출력	설명
Heater	SSR
Humidifier	SSR
Turner	Relay
Fan	PWM
7.2 UI 출력
출력	설명
Temperature	현재값
Humidity	현재값
Progress	Day
Alarm	상태 표시
7.3 Cloud 출력
출력	설명
Telemetry	상태 발행
Shadow Reported	상태 동기화
8. Embedded Product 품질 기준
8.1 목표

제품은:

“ESP32 프로젝트”

처럼 보이면 안 된다.

8.2 목표 품질
항목	목표
UI	상용 수준
안정성	무정지 수준
Recovery	자동
유지보수	장기 가능
UX	직관적
9. 최종 요구사항 철학

가장 중요한 원칙:

기능 추가보다
안정성과 유지보수성이 우선이다.
10. 최종 목표

최종적으로 시스템은 다음을 만족해야 한다.

신뢰 가능한
Premium Embedded Product

수준의 품질.