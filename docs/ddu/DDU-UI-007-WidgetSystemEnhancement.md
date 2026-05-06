# DDU-UI-007 Widget System Enhancement

## 1. Purpose

This DDU upgrades the Premium Home UI widget layer without changing the RuntimeState-centered rendering pipeline.

The goal is to move repeated card/output rendering logic out of `GraphicHomeRenderer` and into small, reusable, testable widget classes.

## 2. Architectural Position

```text
RuntimeState
    ↓
HomeUiModelBuilder
    ↓
HomeUiModel
    ↓
HomeUiDiff
    ↓
DirtyFlags
    ↓
GraphicHomeRenderer
    ↓
Widget Classes
    ↓
IDisplayDevice
    ↓
St7789DisplayDevice
```

## 3. Scope

Included:

- `MetricCardWidget`
- `OutputBarWidget`
- `StatusBarWidget` enhancement
- `ProgressWidget` enhancement
- `GraphicHomeRenderer` refactoring to delegate to widgets
- No new state ownership
- No blocking code
- No display driver dependency in widgets

Excluded:

- Screen Manager
- EC11 Input
- Animation timing
- Alarm Overlay redesign
- EventBus

## 4. Rules

### 4.1 RuntimeState Remains the Source of Truth

Widgets must not read sensors, devices, storage, cloud, or runtime internals directly.

Widgets may only render data already present in `HomeUiModel`.

### 4.2 Device Boundary

Widgets must draw only through `IDisplayDevice`.

Forbidden:

```cpp
#include <LovyanGFX.hpp>
```

inside widget classes.

### 4.3 No State Mutation

Widgets must not mutate:

- RuntimeState
- AppSettings
- AlarmState
- CommandQueue

### 4.4 Non-Blocking

Widgets must not call:

```cpp
delay(...)
vTaskDelay(...)
```

Rendering is synchronous but must remain small and deterministic.

## 5. Widget Responsibilities

### 5.1 MetricCardWidget

Reusable card renderer for major numeric values.

Used for:

- Temperature
- Humidity

Responsibilities:

- Draw card surface
- Draw label
- Draw numeric value
- Draw unit
- Draw status text

### 5.2 OutputBarWidget

Reusable output state renderer.

Responsibilities:

- Draw output strip
- Show actuator labels
- Highlight active outputs

### 5.3 StatusBarWidget

Responsibilities:

- Draw product title
- Show Wi-Fi status
- Show AWS/Cloud status

### 5.4 ProgressWidget

Responsibilities:

- Draw incubation progress label
- Calculate percentage from UiModel only
- Draw progress bar
- Show day information

## 6. Rendering Policy

When Sprite Double Buffer is available:

```text
Render full home screen into sprite
Push once to display
```

When Sprite Double Buffer is not available:

```text
DirtyFlags select affected widgets
Each widget redraws its own Rect
```

## 7. Validation Checklist

- Project builds without LovyanGFX include in widgets.
- Home screen remains visually equivalent or cleaner.
- Temperature and humidity cards are rendered by the same widget class.
- Dirty rendering fallback still works.
- Sprite rendering path still renders full frame.
- No RuntimeState mutation added.

## 8. Next DDU

After this DDU, the correct next step is:

```text
DDU-UI-008 Screen Manager Foundation
```
