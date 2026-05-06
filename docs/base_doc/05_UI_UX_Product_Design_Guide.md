1. 제품 UI 철학
   - 부화기는 계측기처럼 정확해야 하지만, 가전제품처럼 편안해야 한다.
   - 홈 화면은 “현재 상태를 1초 안에 이해”하는 것이 목표.
   - 메뉴는 깊지 않게, 위험 기능은 실수 방지.

2. 화면 구조
   - 320x240 landscape
   - Status Bar
   - Main Content
   - Bottom Action Bar
   - Overlay / Dialog

3. 5개 핵심 화면
   P0 Home
   P1 Progress
   P2 Manual
   P3 Plan Edit
   P4 System

4. 디자인 규칙
   - 숫자는 크고 선명하게
   - 색상은 의미 기반으로만 사용
   - 아이콘은 작고 일관되게
   - 한 화면에 정보 과밀 금지
   - 위험 동작은 확인 또는 Long Hold

5. UI 상태 머신
   Home
   Menu
   Edit
   Confirm
   Provisioning
   SafeMode
   AlarmOverlay

6. Codex 구현 규칙
   - Renderer는 그리기만 한다
   - UiModel만 읽는다
   - AppController 직접 호출 금지
   - 입력은 UiNavigator가 처리한다