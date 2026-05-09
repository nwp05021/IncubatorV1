# DDU-01 Device Bring-up Implementation Notes

## Scope

This patch focuses on the first hardware-testable baseline:

1. `include/config/PinConfig.h` is now the single source of truth for device pins.
2. ST7789 display pin assignment is resolved from `PinConfig`.
3. EC11 rotary encoder + switch is implemented using ESP-IDF GPIO polling.
4. Shared ESP-IDF I2C bus wrapper is introduced for AHT20 and future I2C devices.
5. AHT20 mock implementation is replaced with a real ESP-IDF I2C implementation.
6. `main.cpp` is changed from animated mock data to a hardware bring-up loop:
   - display init
   - EC11 init
   - I2C init
   - AHT20 init/read every 2 seconds
   - 3 main pages by encoder rotation
   - long click menu entry/back
   - BLE provisioning QR placeholder screen

## Important Pin Source Rule

All hardware pin changes must be made only in:

```cpp
include/config/PinConfig.h
```

`src/product/config/PinConfig.h` is only a compatibility forwarding header.

## Test Order

1. Confirm display still boots in 320x240 landscape.
2. Rotate EC11: Main Page 0 → 1 → 2.
3. Long press EC11: enter/exit menu.
4. Confirm AHT20 changes `SEN!` to `AHT` in the header.
5. If AHT20 stays `SEN!`, check SDA/SCL wiring and pull-up status first.

## Known Remaining Work

- Full Korean font engine is not yet embedded as a dedicated compressed font asset.
- BLE provisioning service and QR payload generation are not wired yet; the QR page is a layout placeholder.
- Fan pin remains disabled until actual 12V fan wiring is confirmed.
