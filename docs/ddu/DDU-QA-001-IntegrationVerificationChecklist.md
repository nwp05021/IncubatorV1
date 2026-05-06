
# DDU-QA-001 — Integration Verification Checklist

> Version: 1.0
> Status: Draft
> Target: ESP32-S3 + PlatformIO

---

# 1. 목적

지금까지 작성된 Runtime / Module / UI / Cloud DDU를 실제 통합하기 전 검증 체크리스트를 정의한다.

---

# 2. 핵심 검증 흐름

```text
Build
    ↓
Boot
    ↓
Sensor
    ↓
RuntimeState
    ↓
Climate
    ↓
UI
    ↓
Cloud
    ↓
Recovery
```

---

# 3. Build Checklist

```text
[ ] PlatformIO build 성공
[ ] 모든 include path 정상
[ ] namespace 충돌 없음
[ ] 중복 class 없음
[ ] Arduino/ESP-IDF 혼용 문제 없음
```

---

# 4. RuntimeState Checklist

```text
[ ] RuntimeState 초기값 안전
[ ] safeMode 초기값 false
[ ] output 초기값 모두 OFF
[ ] sensorHealthy 초기 판단 명확
[ ] storageHealthy 초기 판단 명확
```

---

# 5. Command Checklist

```text
[ ] UI 입력은 Command로 변환
[ ] Cloud 입력은 Command로 변환
[ ] CommandQueue overflow 처리
[ ] AppController validation 동작
[ ] SafeMode 시 위험 command 거부
```

---

# 6. Module Checklist

```text
[ ] Sensor tick non-blocking
[ ] Climate tick 500ms 이하
[ ] Scheduler tick 10s 기준
[ ] Turning lockdown 차단
[ ] Fan safeMode 차단
[ ] Alarm delay 정상
[ ] Recovery safeMode 진입 정상
```

---

# 7. UI Checklist

```text
[ ] Home 화면 1초 이해 가능
[ ] 숫자 크기 충분
[ ] StatusBar 정상
[ ] SafeMode Overlay 최우선
[ ] Dirty Render 유지
[ ] Full redraw 남발 없음
```

---

# 8. Cloud Checklist

```text
[ ] WiFi 실패 시 제품 정상 동작
[ ] MQTT 실패 시 제품 정상 동작
[ ] desired는 Command로 변환
[ ] reported는 RuntimeState 기반
[ ] Cloud direct GPIO 없음
```

---

# 9. Storage Checklist

```text
[ ] Settings load 실패 시 default
[ ] Settings validation 정상
[ ] RuntimeState 저장 안 함
[ ] Plan 저장 atomic
[ ] Schema version 존재
```

---

# 10. SafeMode Checklist

```text
[ ] Sensor fail 시 SafeMode
[ ] Storage fail 시 SafeMode
[ ] SafeMode 시 Heater OFF
[ ] SafeMode 시 Humidifier OFF
[ ] SafeMode 시 Turner OFF
[ ] SafeMode overlay 표시
```

---

# 11. 최종 기준

```text
전원이 꺼져도 안전하게 복구되는가?
Cloud가 죽어도 제품이 동작하는가?
6개월 후에도 사람이 이해 가능한가?
```

---

# 12. 다음 단계

```text
DDU-RELEASE-001
Implementation Package Map
```
