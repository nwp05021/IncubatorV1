# 13_Project_Folder_Structure_and_Naming

> Version: 1.0
> Status: Authoritative

---

# 목적

본 문서는 Incubator Firmware 프로젝트의
표준 폴더 구조와 네이밍 규칙을 정의한다.

---

# 1. 핵심 철학

```text
폴더 구조만 봐도
시스템 구조를 이해 가능해야 한다.
```

---

# 2. 권장 프로젝트 구조

```text
product/
├── app/
├── controller/
├── domain/
├── modules/
├── services/
├── storage/
├── ui/
├── cloud/
├── recovery/
└── devices/
```

---

# 3. app/

최상위 조립 영역.

포함:

```text
AppBootstrap
MainLoop
Dependency Wiring
```

---

# 4. controller/

포함:

```text
AppController
CommandQueue
Validators
```

---

# 5. domain/

포함:

```text
RuntimeState
AppSettings
Batch
PlanTable
Policies
```

---

# 6. modules/

포함:

```text
Sensor
Climate
Turning
Alarm
Scheduler
```

---

# 7. services/

공통 서비스.

예:

```text
TimeService
TelemetryService
NotificationService
```

---

# 8. ui/

포함:

```text
Renderer
UiModel
Navigation
Overlay
Theme
```

---

# 9. cloud/

포함:

```text
Wifi
MQTT
Shadow
Telemetry
```

---

# 10. recovery/

포함:

```text
SafeMode
Restore
WDT
Validation
```

---

# 11. devices/

하드웨어 접근 전용.

예:

```text
Aht20Device
RelayDevice
St7789Device
```

---

# 12. 네이밍 규칙

| 종류 | 규칙 |
|---|---|
| Class | PascalCase |
| Method | camelCase |
| Member | m_ prefix |
| Interface | I prefix 선택 |
| Enum | PascalCase |

---

# 13. namespace 전략

권장:

```cpp
namespace incubator::ui
namespace incubator::cloud
```

---

# 14. 최종 원칙

```text
구조가 곧 문서여야 한다.
```
