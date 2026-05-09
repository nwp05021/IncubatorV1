# DDU-01 UI Runtime Implementation Notes

## Version

0.3.2

## Scope

This patch wires the first DDU-01 UI runtime layer after Display, EC11 and AHT20 bring-up.

## Implemented

- `Ddu01UiRuntime`
- Home0 / Home1 / Home2 navigation by EC11 rotation
- Long click from Home pages to Main Menu
- Long click from Menu/sub-screens to Back
- MENU 0~7 shell screens
- BLE Provisioning screen placeholder with large QR area
- WiFi Reset / Reboot confirmation pattern
- Factory Reset 10-second hold UI shell
- RuntimeState read-only UI rendering
- Sensor values written only to RuntimeState in `main.cpp`

## Architecture rule

- UI does not mutate `RuntimeState`.
- UI does not touch GPIO.
- Output control is not implemented in UI.
- Manual Test screen is shell-only until Command wiring is added.

## Acceptance Criteria

- AHT20 values appear on Home0.
- EC11 rotate changes Home0 / Home1 / Home2.
- Long click enters Menu.
- Menu rotate changes selected item.
- Long click returns to previous level.
- MENU 5 opens BLE Provisioning QR placeholder.

## Next DDU

Recommended next patch:

1. CommandQueue wiring for ManualControl and WiFi reset.
2. Korean font engine / VLW font loading.
3. Real BLE Provisioning service + real QR payload.
4. Fan PWM device wiring after hardware connection.
