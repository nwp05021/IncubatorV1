DDU Refactoring Strategy
Legacy DDU Reorganization & Implementation Workflow

Version: 1.0
Status: Authoritative
목적:
기존 DDU들을 새로운 상위 아키텍처 기준에 맞게 재정렬하고,
Codex/Cursor 의존도를 낮춘 “사람 중심 구현 워크플로우”를 정의한다.

1. 왜 DDU 리팩토링이 필요한가?

기존 DDU는 다음 특징이 있었다.

기능 단위 중심

예:

SensorManager 구현
PlanGenerator 구현
AwsClient 구현

이 방식은 초기 속도는 빠르지만:

구조 파편화
상태 흐름 분산
Codex 의존 증가
유지보수 난이도 증가

문제가 발생한다.

2. 새로운 DDU 방향

새 방향은:

아키텍처 중심 + 상태 중심 + 제품 중심

이다.

3. 새로운 문서 계층 구조
3.1 최상위 문서
문서	역할
00_System_Architecture	시스템 헌법
01_Requirement_Definition	요구사항
02_Domain_Model	상태 구조
03_AppController	상태 변경
04_Storage_Recovery	저장/복구
05_UI_UX	UI Framework
06_Cloud_Shadow	Cloud 구조
3.2 하위 DDU

상위 문서를 기반으로:

DDU = 구현 단위

로 축소.

즉:

상위 문서
    ↓
DDU
    ↓
코드

구조.

4. 가장 중요한 변화

기존:

Codex가 구조를 만들었다

새 방식:

사람이 구조를 만든다
Codex는 구현만 한다
5. Codex 역할 재정의
5.1 이제 Codex는:
역할	허용
Renderer 구현	O
Driver 구현	O
Debug	O
Refactoring	O
Boilerplate	O
5.2 금지
역할	이유
전체 구조 설계	위험
상태 흐름 설계	위험
아키텍처 결정	위험
대규모 자동 생성	토큰 폭주
6. 새로운 구현 Workflow
6.1 권장 흐름
1. 상위 아키텍처 문서 작성
    ↓
2. 작은 DDU 작성
    ↓
3. 사람이 구조 검토
    ↓
4. Codex 구현 요청
    ↓
5. 컴파일
    ↓
6. 실기기 테스트
    ↓
7. 문서 반영
6.2 가장 중요한 점
문서가 코드보다 우선
7. DDU 크기 정책
7.1 매우 중요

DDU는 작아야 한다.

7.2 권장 단위

좋은 예:

P0 Home Renderer
Plan Row Serializer
AHT20 Async State Machine
Shadow Delta Parser
7.3 금지
❌ 전체 UI 구현
❌ 전체 Cloud 구현
❌ 전체 제품 생성
8. DDU 작성 규칙

모든 DDU는 반드시 포함:

항목	설명
목적	왜 필요한가
생성 파일	정확한 파일
의존성	상위 구조
입력	무엇을 받는가
출력	무엇을 변경하는가
금지 사항	절대 금지
Acceptance Criteria	완료 기준
9. 상태 흐름 우선 원칙

DDU보다 먼저 정의해야 하는 것:

상태 흐름
9.1 예시

잘못된 접근:

UI부터 구현

올바른 접근:

RuntimeState 정의
    ↓
UiModel 정의
    ↓
Renderer 구현
10. UI DDU 리팩토링 방향

기존:

UI 전체 구현

새 방향:

DDU	역할
UI-001	StatusBar
UI-002	Home Value Card
UI-003	ProgressBar
UI-004	Alarm Overlay
UI-005	Focus Renderer
11. Cloud DDU 리팩토링 방향

기존:

AWS 전체 연결

새 방향:

DDU	역할
CLOUD-001	WifiManager
CLOUD-002	MQTT Connect
CLOUD-003	TelemetryBuilder
CLOUD-004	Shadow Delta Parser
CLOUD-005	Offline Queue
12. Module DDU 리팩토링 방향

기존:

Climate 전체 구현

새 방향:

DDU	역할
MOD-001	Temp Hysteresis
MOD-002	Humi Hysteresis
MOD-003	Alarm Confirm Timer
MOD-004	Turner Scheduler
13. 가장 중요한 설계 원칙
13.1 이해 가능한 구조

가장 중요:

사람이 읽고 이해 가능한 구조
13.2 이유

Codex는:

토큰 제한 존재
문맥 손실 존재
구조 붕괴 가능성 존재

따라서:

사람이 구조를 통제

해야 한다.

14. 상태 기반 설계 원칙
14.1 핵심
상태 흐름이 곧 시스템이다.
14.2 좋은 구조 예시
Command
    ↓
AppController
    ↓
RuntimeState
    ↓
UI / Cloud

이 흐름이 명확해야 한다.

15. “작은 구현 단위” 전략
15.1 매우 중요

Codex에는 항상:

작고 명확한 작업

만 맡긴다.

15.2 좋은 요청 예시
P0 Home Temperature Card Renderer 구현
- UiModel만 사용
- RuntimeState 접근 금지
- Dirty Render 사용
15.3 나쁜 요청 예시
부화기 UI 전체 구현해줘
16. Acceptance Criteria 중요성

모든 DDU는 완료 조건이 있어야 한다.

예:

AC-1:
37.5° 정상 출력

AC-2:
Dirty Render 동작

AC-3:
Full redraw 없음
17. 실기기 검증 우선

중요:

컴파일 성공 ≠ 완료
17.1 반드시 확인
항목	확인
Flicker	O
Encoder UX	O
Recovery	O
Boot Restore	O
Alarm Overlay	O
18. 유지보수 전략
18.1 가장 중요한 질문
6개월 후 내가 이해 가능한가?
18.2 목표
목표	설명
직관성	구조 단순
추적 가능성	상태 흐름 명확
독립성	Layer 분리
확장성	Sensor/Cloud 교체 가능
19. 추천 실제 개발 순서
Phase 1 — Foundation
00 Architecture
01 Requirements
02 Domain
03 AppController
04 Storage
Phase 2 — Core Runtime
Sensor
Scheduler
Climate
Turning
Phase 3 — UI
StatusBar
Home
Progress
Overlay
Navigation
Phase 4 — Cloud
WiFi
MQTT
Telemetry
Shadow
Offline Queue
Phase 5 — Recovery
SafeMode
WDT
Restore
Diagnostics
20. 최종 철학

가장 중요한 원칙:

Codex는 구현 도구일 뿐,
아키텍처는 사람이 소유한다.
21. 최종 목표

최종 목표는 다음이다.

토큰 부족에도
사람이 완전히 이해 가능한
Premium Embedded Product