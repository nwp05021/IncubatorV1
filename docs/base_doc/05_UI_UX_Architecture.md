05_UI_UX_Architecture.md

Version: 1.0
Status: Authoritative
목적:
본 문서는 Incubator Firmware의 전체 UI/UX 구조를 정의한다.
모든 Renderer/UI/DDU 구현은 본 문서를 기준으로 한다.


1. 문서 목적
본 문서는 단순 “화면 디자인 문서”가 아니다.
다음을 정의한다.
- UI Framework- UX Philosophy- Navigation- Rendering- Component System- Overlay- Input Model- Focus System- Embedded Premium UI 전략

2. UI 핵심 철학
2.1 가장 중요한 목표
사용자는1초 안에 현재 상태를 이해할 수 있어야 한다.

2.2 UI 역할
UI는:
현재 상태를 명확하게 보여주는 것
이 목적이다.

2.3 UI 금지 사항
❌ 화려한 게임 UI❌ RGB 남용❌ 정보 과밀❌ 작은 숫자 다량 표시❌ 디버그 화면 같은 UI

3. UI 스타일 방향
항목방향스타일Industrial Premium분위기차분함우선순위숫자 중심색상의미 기반애니메이션최소구조단순정보량낮게

4. Display Architecture
4.1 하드웨어
항목내용DisplayST7789Resolution320x240OrientationLandscape

4.2 좌표 기준
Origin = Left Top

4.3 Grid System
320 / 8 columns240 / 6 rows40px base grid

5. 전체 UI 구조
┌──────────────────────────────┐│ StatusBar                    │├──────────────────────────────┤│ TitleBar                     │├──────────────────────────────┤│                              ││ Main Content                 ││                              │├──────────────────────────────┤│ Bottom Action Bar            │└──────────────────────────────┘

6. UI Layer Architecture
EC11    ↓UiInputController    ↓UiNavigator    ↓UiStateMachine    ↓UiViewModelBuilder    ↓UiModel    ↓Renderer    ↓Display Driver

7. 책임 분리
계층역할UiInputController입력 해석UiNavigator화면 이동UiStateMachine현재 UI 상태UiViewModelBuilderRuntime → UiModelRendererDraw OnlyDisplay DriverTFT 출력

8. UI 상태 머신
8.1 핵심 상태
HomeProgressManualPlanEditSystemOverlaySafeMode

8.2 최대 Depth
최대 2 Depth
이상 금지.

9. 입력 UX
9.1 입력 장치
EC11 Rotary Encoder+ Push Button

9.2 입력 규칙
동작의미Rotate이동Click선택Long ClickBackLong Hold위험 승인

10. Focus System
10.1 목적
현재 조작 위치를 명확히 보여준다.

10.2 표현 방식
권장:
- Border- Reverse Color- Underline

10.3 금지
❌ 빠른 깜빡임❌ RGB Flash❌ 과한 애니메이션

11. Status Bar
11.1 역할
상태 인지 영역.

11.2 포함 정보
정보중요도Alarm높음Time높음Day중간WiFi중간Cloud낮음

11.3 예시
08:12      Day 7 / 21      WiFi AWS

12. Main Pages
Page역할P0 Home핵심 상태P1 Progress부화 진행P2 Manual수동 제어P3 Plan EditDay 설정P4 System엔지니어링 정보

13. P0 Home Screen
13.1 목적
제품의 얼굴.

13.2 가장 중요한 원칙
현재 온도/습도가UI의 중심이어야 한다.

13.3 표시 항목
현재 온도현재 습도목표값진행률장치 상태

13.4 금지 항목
❌ Heap❌ Debug Log❌ Raw Sensor❌ Boot Count

14. P1 Progress Screen
목적
현재 부화 진행 상태
표시.

표시 항목
항목설명Current Day현재 일차Total Days전체ProgressBar진행률Next Turning다음 전란Lockdown상태

15. P2 Manual Screen
목적
유지보수 및 디버깅.

위험 기능
기능보호Heater ONLong HoldResetConfirmFan Manual제한

표시
MANUAL MODE ACTIVE
명확히 표시.

16. P3 Plan Edit
목적
Day 기반 정책 수정.

수정 가능 항목
TempHumidityTurningInterval

UX 목표
전문가는 빠르게,초보자는 안전하게

17. P4 System
목적
엔지니어링 정보 제공.

표시 항목
항목중요도WiFi높음AWS높음Firmware중간Uptime중간IP낮음

18. Overlay Architecture
18.1 종류
Overlay목적Toast짧은 알림Dialog사용자 확인Alarm위험 경고SafeMode치명 오류

18.2 우선순위
SafeMode    > Alarm        > Dialog            > Toast

19. Alarm UX
19.1 목표
즉시 인지 가능

19.2 표시 예시
HIGH TEMP39.2°CCHECK HEATER

19.3 색상 정책
상태색상WarningYellowAlarmRedSafeModeDeep Red

20. Component Architecture
20.1 핵심 구조
UiModel    ↓Component Tree    ↓Render Queue    ↓Display

20.2 Component 종류
Component역할Value숫자Text텍스트Icon상태StatusPillON/OFFProgressBar진행률DialogOverlayGraph추세

21. Rendering Architecture
21.1 가장 중요한 원칙
안 그리는 것이더 중요하다.

21.2 Dirty Render
상태 변경 시만 redraw

21.3 Full Refresh 최소화
권장:
Dirty Region Render

22. Render Pipeline
RuntimeState    ↓UiModel Build    ↓Dirty Check    ↓Render Queue    ↓Renderer

23. Animation 정책
23.1 목표
부드러움
이지
화려함
이 아니다.

23.2 허용
효과허용FadeOSlideOSoft HighlightO

23.3 금지
❌ Bounce❌ RGB Flash❌ Particle❌ Physics Animation

24. Theme System
24.1 목적
제품 정체성 유지.

24.2 Theme 구조
struct UiTheme{    background;    surface;    primaryText;    secondaryText;    tempAccent;    humidityAccent;    success;    warning;    danger;};

25. Trend UX (권장)
목적
“살아있는 장비 느낌”

권장
Tiny TrendMini GraphSoft Refresh

중요한 점
과하면 싸구려가 된다.

26. Ambient UX
목적
고급 장비 느낌.

예시
효과설명Soft Glow상태 변화Gentle Highlight값 변경Smooth Progress진행률

핵심
거의 느껴지지 않을 정도
가 가장 고급스럽다.

27. Memory 정책
금지
❌ 반복 new/delete❌ 동적 증가❌ Runtime Font Load

권장
Static BufferFixed ArrayBitmap Cache

28. Embedded Premium UI 철학
좋은 MCU UI는:
앱처럼 화려한 UI
가 아니다.
진짜 좋은 MCU UI는:
안정적이고 신뢰감 있는 UI
이다.

29. 최종 목표
최종적으로 사용자는 다음처럼 느껴야 한다.
“누가 만든 DIY 장치”
가 아니라
“완성된 상용 장비”
같은 느낌.

30. 최종 원칙
모든 UI 구현은 다음 질문을 통과해야 한다.
이 요소가정말 사용자에게 필요한가?
불필요하면 제거한다.