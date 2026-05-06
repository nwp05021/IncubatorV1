# DDU-UI-008 Screen Manager Foundation

## 1. Purpose

This DDU defines the first production-safe screen management layer for the Premium Incubator UI.

The goal is not to implement a full menu system yet. The goal is to introduce a small, deterministic, non-blocking screen switch pipeline that preserves the existing data flow:

```text
RuntimeState
    ↓
UiModel
    ↓
Screen Manager
    ↓
Screen
    ↓
Renderer / Widget
    ↓
IDisplayDevice
```

## 2. Scope

Included:

- Screen identity model
- Screen transition request
- Screen Manager
- Home screen registration
- Placeholder production screens
- Runtime-safe invalidation on screen change
- Non-blocking render path

Not included:

- EC11 hardware driver
- Animation
- Overlay priority policy
- Menu editor
- Command mutation from UI

Those are handled by later DDUs.

## 3. Design Principles

- RuntimeState remains the single source of truth.
- ScreenManager does not own product state.
- Screen transition is command-like, explicit, and traceable.
- Screen change invalidates the target screen once.
- Each screen owns its own diff/snapshot policy.
- Display backend remains hidden behind IDisplayDevice.
- No delay is allowed.

## 4. Screen Set

```cpp
enum class ScreenId
{
    Home,
    Progress,
    Settings,
    Diagnostics,
    Alarm
};
```

The initial implementation provides:

- Home: existing GraphicHomeScreen
- Progress: placeholder industrial screen
- Settings: placeholder industrial screen
- Diagnostics: placeholder industrial screen
- Alarm: placeholder industrial screen

## 5. Screen Interface

Each screen must implement:

```cpp
class IScreen
{
public:
    virtual ~IScreen() = default;
    virtual void onEnter() = 0;
    virtual void onExit() = 0;
    virtual void render(const ScreenContext& context) = 0;
};
```

## 6. ScreenContext

ScreenContext is read-only for UI rendering.

```cpp
struct ScreenContext
{
    const HomeUiModel* home = nullptr;
    uint32_t nowMs = 0;
};
```

This keeps the current implementation small while allowing later expansion to RuntimeState-backed composite view models.

## 7. ScreenManager Contract

ScreenManager is responsible for:

- Registering screen pointers
- Tracking active screen
- Applying requested screen transitions
- Calling onExit / onEnter
- Rendering only the active screen

ScreenManager must not:

- Read hardware directly
- Mutate RuntimeState
- Perform recovery
- Block runtime execution

## 8. Transition Policy

A screen change is explicit:

```cpp
screenManager.request(ScreenId::Diagnostics);
screenManager.tick(context);
```

The transition is applied at the start of `tick()`. This makes it safe to request transitions from future input systems without immediate nested rendering side effects.

## 9. Verification

Minimum hardware verification:

1. Device boots to Home screen.
2. Every 4 seconds the active screen changes.
3. No blocking delay is used.
4. Home screen continues to render live values when active.
5. Placeholder screens clear and redraw predictably.
6. No crash occurs during repeated transitions.

## 10. Next DDU

The next suitable DDU is:

```text
DDU-UI-009 EC11 Input System Foundation
```

That DDU will connect rotary/button input to ScreenManager transition requests.
