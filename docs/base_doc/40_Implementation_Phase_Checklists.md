# 40_Implementation_Phase_Checklists

> Version: 1.0
> Status: Authoritative

---

# 목적

본 문서는 구현 단계별 완료 기준(Checklist)을 정의한다.

---

# Phase-01 Architecture

## 완료 기준

- 상위 문서 완료
- 상태 흐름 정의 완료
- Layer 책임 확정
- RuntimeState 확정

---

# Phase-02 Runtime

## 완료 기준

- Main Loop 정상
- Tick 기반 동작
- delay 제거
- Sensor Poll 정상

---

# Phase-03 Recovery

## 완료 기준

- SafeMode 진입
- Plan Restore
- Settings Restore
- WDT Recovery

---

# Phase-04 UI

## 완료 기준

- Flicker 최소화
- Overlay 정상
- Focus 명확
- Navigation 직관적

---

# Phase-05 Cloud

## 완료 기준

- Offline 동작
- MQTT Reconnect
- Shadow Sync
- Telemetry Publish

---

# 최종 기준

```text
실기기 장시간 테스트 통과
```
