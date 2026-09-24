# Technology Stack

**Analysis Date:** 2026-09-24

## Languages

**Primary:**
- C++17 (`-std=gnu++17`) - ESP32 firmware, board drivers, services, LVGL UI, and the platform-independent Pomodoro engine in `platformio.ini` and `src/`.

**Secondary:**
- C++17 - Native Unity tests compile only `src/core/*`, as configured by `build_src_filter`, with tests in `test/native/test_pomodoro_engine/test_main.cpp` and `platformio.ini`.
- YAML - GitHub Actions build definition in `.github/workflows/build.yml`.
- Just recipe syntax and Bash - Developer command wrappers in `justfile`.

## Runtime

**Environment:**
- Arduino on ESP32-S3, supplied by PlatformIO's pinned `espressif32@6.8.1` platform, for the `esp32-s3-devkitc-1` board profile in `platformio.ini`.
- Native host executable for engine tests through PlatformIO's `native` platform; that platform is not version-pinned in `platformio.ini`.

**Package Manager:**
- PlatformIO Core; CI pins `platformio==6.1.18` via pip in `.github/workflows/build.yml`, while local prerequisites do not pin the CLI in `README.md`.
- PlatformIO Package Manager installs declared platform and library dependencies through `pio pkg install` in `justfile`.
- Lockfile: missing; reproducibility relies on explicit pins in `platformio.ini` and the CI PlatformIO pin in `.github/workflows/build.yml`.

## Frameworks

**Core:**
- Arduino framework for ESP32 - Firmware runtime, GPIO/PWM, I²C, Wi-Fi, time, serial diagnostics, and Preferences/NVS, selected in `platformio.ini` and used throughout `src/main.cpp`, `src/board/`, and `src/services/`.
- LVGL `8.4.0` - Touch UI, display/input registration, and widgets, pinned in `platformio.ini`, configured in `include/lv_conf.h`, and used in `src/ui/app_ui.cpp`.
- GFX Library for Arduino `1.4.7` - NV3041A display output over ESP32 QSPI, pinned in `platformio.ini` and instantiated in `src/board/display_driver.h`.

**Testing:**
- Unity (version resolved by PlatformIO, not explicitly pinned) - Native unit tests selected by `test_framework = unity` in `platformio.ini` and implemented in `test/native/test_pomodoro_engine/test_main.cpp`.

**Build/Dev:**
- PlatformIO Core `6.1.18` in CI - Dependency installation, native tests, ESP32 compilation, upload, cleaning, and serial monitoring in `.github/workflows/build.yml` and `justfile`.
- Python `3.12` on `ubuntu-latest` - CI host used to install and run PlatformIO in `.github/workflows/build.yml`.
- GitHub Actions `actions/checkout@v7` and `actions/setup-python@v7` - CI setup actions in `.github/workflows/build.yml`.
- `just` (version not pinned) - Optional local task runner wrapping PlatformIO commands in `justfile` and documented in `README.md`.
- Amp orb setup pins PlatformIO `6.1.18`, `just` `1.42.4`, and `prek` `0.5.3`, then installs PlatformIO project packages in `.agents/setup`.

## Key Dependencies

**Critical:**
- `moononournation/GFX Library for Arduino@1.4.7` - Drives the NV3041A display via `Arduino_ESP32QSPI` and `Arduino_NV3041A` in `src/board/display_driver.h` and `platformio.ini`.
- `lvgl/lvgl@8.4.0` - Renders the clock/Pomodoro UI and accepts GT911 pointer input in `src/ui/app_ui.cpp`, `src/board/display_driver.cpp`, and `src/board/touch_driver.cpp`.
- Arduino-ESP32 bundled APIs - `WiFi`, `Preferences`, `Wire`, `esp_timer`, and libc time integration are used in `src/services/clock_service.cpp`, `src/services/session_store.cpp`, `src/services/settings_store.h`, `src/board/touch_driver.h`, and `src/main.cpp`.

**Infrastructure:**
- No server-side infrastructure dependency exists; the artifact is standalone firmware, and accounts, cloud sync, analytics, calendars, phone apps, and Token Pulse integration are explicitly excluded in `README.md`.

## Configuration

**Environment:**
- Local Wi-Fi credentials are compile-time macros `WIFI_SSID` and `WIFI_PASSWORD` in ignored `include/wifi_config.h`, created from `include/wifi_config.example.h` by `just setup` in `justfile`.
- Firmware still builds without credentials because `src/services/clock_service.cpp` uses `__has_include("wifi_config.h")` and empty fallback values; the file must not be committed according to `AGENTS.md`.
- LVGL uses RGB565 (`LV_COLOR_DEPTH 16`), a 48 KiB LVGL heap, Arduino `millis()` ticks, selected Montserrat fonts, and disabled LVGL logging in `include/lv_conf.h`.

**Build:**
- `platformio.ini` defines default firmware environment `jc4827w543` and native environment `native`; `just check` runs the same native-test-then-firmware-build sequence as `.github/workflows/build.yml` via `justfile`.
- Firmware configuration is 4 MB flash, `huge_app.csv`, QIO/OPI memory, OPI PSRAM, 921600-baud upload, 115200-baud monitor, USB CDC on boot, and `BOARD_HAS_PSRAM` in `platformio.ini`.
- The target listing expects 4 MB flash and 8 MB PSRAM, but physical-board verification remains outstanding in `docs/hardware.md` and `README.md`.

## Platform Requirements

**Development:**
- PlatformIO Core and optionally `just` are required; a USB-connected Guition JC4827W543 is required only for upload and physical verification, per `README.md` and `justfile`.
- Fresh Amp orbs provision the command-line tools and declared PlatformIO packages through the idempotent `.agents/setup` script.
- Automated checks are `pio test -e native` and `pio run -e jc4827w543`; there is no C++ lint, formatter, or typecheck stage according to `AGENTS.md` and `.github/workflows/build.yml`.

**Production:**
- Guition JC4827W543 with expected ESP32-S3-WROOM-1-N4R8, 480×272 NV3041A QSPI display, GT911 I²C touch controller, 4 MB flash, and 8 MB PSRAM in `docs/hardware.md` and `src/board/board_config.h`.
- Display QSPI uses GPIO 45/47/21/48/40/39, backlight PWM uses GPIO 1, and GT911 I²C/reset/interrupt use GPIO 8/4/38/3 in `src/board/board_config.h`; this profile is explicitly not the unrelated parallel-RGB map described in `docs/hardware.md`.
- Display, touch, Wi-Fi, brightness, and sound behavior are not yet verified on physical hardware; required checks are recorded in `docs/acceptance.md` and `README.md`.

---

*Stack analysis: 2026-09-24*
