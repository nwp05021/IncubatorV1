
# DDU-ALARM-001 — Alarm Pipeline Preview

핵심 흐름:

RuntimeState
    ↓
AlarmDetector
    ↓
AlarmState
    ↓
UI Overlay
    ↓
Cloud Publish

목표:

- 즉시 인지 가능한 Alarm
- Alarm Delay
- SafeMode 연계
- RuntimeState 기반 판단
