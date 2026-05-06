DDU-UI-MASTER-001
Incubator Premium UI/UX Design Guide

Version: 1.0
Status: Authoritative
목적:
“상용 고급 부화기 UI”의 기준을 정의한다.
본 문서는 모든 UI 구현의 최상위 기준이며, Renderer/UI 코드보다 우선한다.

1. UI 설계 철학 (Design Philosophy)
1.1 제품의 정체성

이 제품은 단순 DIY 장치가 아니다.

다음 성격을 동시에 가진다.

성격	의미
계측 장비	정확하고 신뢰 가능해야 함
가전 제품	직관적이고 편안해야 함
IoT 장치	연결 상태를 명확히 보여야 함
생명 유지 장치	위험 상태를 즉시 인지 가능해야 함
1.2 핵심 UI 목표

UI의 목적은 “예쁘게 보이는 것”이 아니다.

가장 중요한 목표:

현재 상태를 1초 안에 이해할 수 있어야 한다.

즉 사용자는 전원을 켠 직후 다음을 즉시 알아야 한다.

현재 온도
현재 습도
정상 동작 여부
현재 부화 진행 상황
위험 상태 존재 여부
1.3 디자인 방향

UI 방향은 다음을 따른다.

항목	방향
스타일	Industrial Premium
분위기	차분하고 신뢰감
애니메이션	최소한, 부드럽게
색상	기능 기반 색상만 사용
정보 밀도	낮게
글꼴	큰 숫자 중심
계층 구조	단순
조작 깊이	최대 2 Depth
2. 화면 시스템 구조
2.1 전체 구조
┌──────────────────────────────┐
│ Status Bar                   │
├──────────────────────────────┤
│                              │
│ Main Content                 │
│                              │
├──────────────────────────────┤
│ Bottom Action Bar            │
└──────────────────────────────┘
2.2 영역 정의
영역	역할
Status Bar	시간 / 네트워크 / 알람
Main Content	핵심 정보
Bottom Bar	상태 아이콘 / 힌트
Overlay	알람 / SafeMode / 확인창
2.3 해상도 기준
static constexpr int SCREEN_W = 320;
static constexpr int SCREEN_H = 240;

Landscape 기준.

3. 색상 시스템
3.1 핵심 원칙

색상은 “의미”를 위해서만 사용한다.

장식용 컬러 사용 금지.

3.2 Primary Palette
역할	색상
Background	Pure Black
Surface	Dark Gray
Primary Text	White
Secondary Text	Gray
Divider	Deep Gray
3.3 Semantic Colors
의미	색상
Temperature	Orange
Humidity	Cyan
Active	Green
Warning	Yellow
Alarm	Red
SafeMode	Deep Red
3.4 금지 사항
❌ 무지개 RGB 사용 금지
❌ 과도한 Gradient 금지
❌ 번쩍이는 효과 금지
❌ 게임 UI 스타일 금지
4. Typography
4.1 UI 핵심 원칙

이 제품의 핵심은 숫자이다.

따라서:

숫자가 UI의 얼굴이다.
4.2 폰트 우선순위
요소	우선순위
현재 온도	최상
현재 습도	최상
알람 상태	높음
목표값	중간
시스템 정보	낮음
4.3 숫자 스타일

현재값은 가능한 크게 표시한다.

예:

37.5°
65%

온도와 습도는 서로 시각적 균형 유지.

5. 입력 UX 원칙
5.1 입력 장치
EC11 Rotary Encoder
+ Push Button
5.2 조작 철학

사용자는 설명서를 보지 않아도 이해 가능해야 한다.

5.3 입력 규칙
동작	의미
Rotate	이동
Click	선택
Long Click	뒤로 / 홈
Long Hold	위험 작업 승인
5.4 위험 작업

다음은 반드시 Long Hold 필요.

Factory Reset
Batch Stop
SafeMode Clear
Manual Heater ON
6. 화면 구성
6.1 Screen Map
Page	이름	역할
P0	Home	핵심 상태
P1	Progress	진행 정보
P2	Manual	수동 제어
P3	Plan Edit	Day 계획 수정
P4	System	시스템 상태
7. P0 — Home Screen
7.1 목적

제품의 얼굴.

사용자가 가장 오래 보는 화면.

7.2 핵심 목표
1초 안에 현재 상태 파악
7.3 레이아웃
┌──────────────────────────────────────────┐
│ 08:12      Day 7 / 21        WiFi Cloud │
├──────────────────────────────────────────┤
│                                          │
│      37.5°               65%             │
│                                          │
│    Target 37.8°      Target 60%          │
│                                          │
│                                          │
├──────────────────────────────────────────┤
│ HTR  HUM  TURN  FAN       [====      ]   │
└──────────────────────────────────────────┘
7.4 디자인 원칙
현재값 우선

현재값이 UI의 70% 이상을 차지해야 한다.

불필요 정보 제거

Home에는 다음 금지.

❌ Boot Count
❌ IP Address
❌ Heap Memory
❌ Debug Text
❌ Raw Sensor State
7.5 아이콘 규칙

아이콘은 작고 단순하게.

텍스트보다 앞서면 안 된다.

8. P1 — Progress Screen

목적:

“부화가 얼마나 진행되었는가?”

표시:

진행률 바
현재 Day
남은 일수
다음 전란 시간
Lockdown 예정 정보
9. P2 — Manual Screen
9.1 목적

디버깅 및 유지보수.

평상시 자주 사용하는 화면이 아니다.

9.2 원칙

위험 동작은 명확히 표시.

예:

MANUAL MODE ACTIVE

노란색 표시.

9.3 수동 제어 항목
항목	설명
Heater	ON/OFF
Humidifier	ON/OFF
Turner	Trigger
Fan	Duty
10. P3 — Plan Edit
10.1 목표

전문 사용자가 Day별 정책 수정 가능.

10.2 표시 항목
Day 8
Temp      37.6°
Humidity  58%
Turning   ON
Interval  120m
10.3 편집 UX
Rotate → 값 변경
Click → 다음 필드
Long Click → 저장 후 복귀
11. P4 — System
11.1 목적

엔지니어링/점검 정보.

11.2 표시 항목
Uptime
WiFi 상태
AWS 상태
Firmware Version
IP Address
SafeMode 여부
12. Overlay System
12.1 Overlay 종류
종류	목적
Alarm Overlay	경고
SafeMode Overlay	치명 오류
Confirm Dialog	사용자 확인
Provisioning Overlay	BLE/WiFi 설정
12.2 Overlay 원칙

Overlay는 화면 전체를 장악해야 한다.

배경은 어둡게.

13. 애니메이션 정책
13.1 목표

“고급스럽게 느껴질 정도만”

13.2 허용
효과	허용 여부
Fade	O
Slide	O
Blink	제한적
Bounce	X
Zoom	X
14. 성능 원칙
14.1 절대 규칙
UI 때문에 제어가 느려지면 실패다.
14.2 제한
항목	기준
Render Interval	100ms
Full Redraw	최소화
Dynamic Allocation	금지
ISR Rendering	금지
15. UI 아키텍처
15.1 구조
RuntimeState
    ↓
UiViewModelBuilder
    ↓
UiModel
    ↓
Renderer
15.2 역할 분리
구성	역할
UiInputController	입력 처리
UiNavigator	페이지 상태
UiViewModelBuilder	상태 변환
MainUiRenderer	Draw Only
15.3 Renderer 규칙
Renderer는 절대:
- AppController 접근 금지
- RuntimeState 수정 금지
- 비즈니스 로직 금지
16. 고급 UI 구현 전략
16.1 가장 중요한 원칙
좋은 UI는 “많이 보여주는 UI”가 아니다.

좋은 UI는:

“중요한 것만 보여주는 UI”

이다.

16.2 MCU UI에서 특히 중요한 것
항목	중요 이유
큰 숫자	멀리서도 확인
검은 배경	저가 TFT 품질 향상
단순 레이아웃	성능 유지
적은 색상	고급감 증가
안정적 프레임	신뢰감
17. 최종 목표

이 제품 UI의 목표는 다음이다.

DIY 장치처럼 보이지 않는 것.

즉:

“상용 의료/계측 장비 수준의 신뢰감”

을 주는 UI를 목표로 한다.