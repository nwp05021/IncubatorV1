
# DDU-CLOUD-002 — Telemetry Preview

핵심 흐름:

RuntimeState
    ↓
TelemetryBuilder
    ↓
JSON
    ↓
MQTT Publish

목표:

- reported JSON
- Dirty Sync
- Offline Queue
- Telemetry Rate Control
