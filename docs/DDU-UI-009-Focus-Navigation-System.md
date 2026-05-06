# DDU-UI-009 Focus Navigation System

## 1. Purpose

This DDU introduces a RuntimeState-centered UI focus navigation system for the Premium Incubator UI.

The goal is to make EC11 rotation move the current UI focus without allowing input code or widgets to mutate state directly.

```text
EC11 Device
    ↓
InputRuntimeTask
    ↓
CommandQueue
    ↓
AppController
    ↓
RuntimeState.focusedItem
    ↓
HomeUiModel
    ↓
HomeUiDiff
    ↓
Widget Renderer
```

## 2. Scope

Included:

- `UiFocusId`
- `RuntimeState.focusedItem`
- `UiRotateLeft / UiRotateRight` command handling
- Home UI model focus projection
- Focus dirty detection
- Focus highlight rendering for Home widgets

Excluded:

- Value editing
- Setting commit
- Long press action
- Screen navigation by click
- Menu hierarchy

These are intentionally deferred to later DDUs.

## 3. Architecture Rule

Input must never call UI widgets directly.

Allowed:

```text
Input → Command → AppController → RuntimeState → UiModel → Renderer
```

Forbidden:

```text
Input → Widget
Input → Renderer
Input → RuntimeState direct mutation
```

## 4. Focus Model

`UiFocusId` is defined in the domain layer because the currently focused item is runtime state.

```cpp
enum class UiFocusId
{
    None,
    Temperature,
    Humidity,
    EggTurn,
    Fan,
    Lighting,
    StartButton,
    StopButton
};
```

Current implementation uses this Home focus ring:

```text
Temperature → Humidity → Fan → StartButton → StopButton → Temperature
```

Reverse rotation walks the same ring backward.

## 5. RuntimeState Contract

`RuntimeState` owns the selected focus.

```cpp
UiFocusId focusedItem = UiFocusId::Temperature;
```

This preserves Single Source of Truth.

## 6. Command Contract

`CommandType::UiRotateLeft` and `CommandType::UiRotateRight` are consumed by `AppController`.

They do not edit target values yet.

They only mutate:

```cpp
runtime.focusedItem
```

## 7. Rendering Contract

Widgets receive focus state from `HomeUiModel`.

Widgets may render:

- accent border
- highlighted surface
- focused output label

Widgets must not own focus state.

## 8. Dirty Render Contract

`HomeDirtyFlags` adds:

```cpp
bool focusChanged;
```

Focus changes trigger redraw only for affected widgets.

## 9. Verification

Expected test result:

1. Device boots to Home screen.
2. EC11 clockwise rotation moves focus forward.
3. EC11 counter-clockwise rotation moves focus backward.
4. Temperature and Humidity cards show accent focus border.
5. Output bar shows focus for Fan / Start / Stop.
6. No blocking delay is introduced.
7. RuntimeScheduler remains active.

## 10. Next DDU

Next step:

```text
DDU-UI-010 Editable Field System
```

That DDU will add:

- focus select
- edit mode
- value adjustment
- cancel / commit separation
