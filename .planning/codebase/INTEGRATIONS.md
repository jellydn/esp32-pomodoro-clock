# External Integrations

**Analysis Date:** 2026-09-24

## APIs & External Services

**Network time:**
- Public NTP is the only runtime Internet service: `pool.ntp.org`, `time.cloudflare.com`, and `time.google.com` are passed to Arduino `configTzTime` in `src/services/clock_service.cpp`.
- Timezone is fixed to POSIX `SGT-8` (Asia/Singapore), and synchronized time is accepted only at or after epoch `1700000000`, in `src/services/clock_service.cpp`.
- SDK/Client: Arduino-ESP32 time APIs and SNTP integration exposed through `<time.h>` in `src/services/clock_service.cpp`; there is no separately declared NTP library in `platformio.ini`.
- Auth: none; NTP requests use no token or account in `src/services/clock_service.cpp`.

**Wi-Fi network:**
- ESP32 station mode connects with `WiFi.begin`, reports IP/RSSI or disconnect reason over serial, and retries every 30 seconds without blocking the UI in `src/services/clock_service.cpp`.
- SDK/Client: Arduino-ESP32 `<WiFi.h>` bundled by the Arduino framework selected in `platformio.ini`.
- Auth: local SSID/password compile-time macros from ignored `include/wifi_config.h`; the checked-in shape is documented by `include/wifi_config.example.h` and setup is automated by `justfile`.

**Cloud and application services:**
- No cloud API, cloud sync, phone app, analytics, calendar, account, or Token Pulse integration is implemented; these are explicitly out of scope in `README.md`, and runtime networking in `src/services/clock_service.cpp` is limited to Wi-Fi and NTP.

## Data Storage

**Databases:**
- No external or embedded relational/document database is used; persistence uses the ESP32 Preferences API backed by on-device NVS in `src/services/settings_store.cpp` and `src/services/session_store.cpp`.
- NVS namespace `settings` stores 12/24-hour mode, normal/dim brightness, and dim timeout as typed keys in `src/services/settings_store.cpp`.
- NVS namespace `session` stores a binary, magic-checked, size-checked version-1 record only on state transitions (or once trusted wall time becomes available while running) in `src/services/session_store.cpp` and `src/main.cpp`.
- A running session persists an epoch deadline when wall time is trusted; restoration falls back to paused when time is untrusted, while active timing itself uses monotonic `esp_timer_get_time()` in `src/services/session_store.cpp` and `src/main.cpp`.

**File Storage:**
- No runtime filesystem or remote object storage is used; the only local developer-side secret file is ignored `include/wifi_config.h`, derived from `include/wifi_config.example.h` through `justfile`.

**Caching:**
- No cache service is present; live state is held in memory by the engine and UI, while durable state uses Preferences/NVS in `src/core/pomodoro_engine.h`, `src/services/session_store.cpp`, and `src/services/settings_store.cpp`.

## Authentication & Identity

**Auth Provider:**
- None: the firmware has no user accounts, identity provider, authorization layer, OAuth flow, or API tokens, consistent with the exclusions in `README.md`.
- Wi-Fi credentials provide network access only and are compile-time macros in ignored `include/wifi_config.h`, with placeholders in `include/wifi_config.example.h`; they are not application identity.

## Monitoring & Observability

**Error Tracking:**
- None: there is no hosted crash reporting, telemetry, or analytics dependency in `platformio.ini`, and analytics are excluded in `README.md`.

**Logs:**
- USB serial at 115200 baud reports hardware details, display/touch failures, Wi-Fi events, and NTP configuration in `src/main.cpp` and `src/services/clock_service.cpp`; monitor speed and command are defined in `platformio.ini` and `justfile`.
- LVGL's own logging is disabled with `LV_USE_LOG 0` in `include/lv_conf.h`.

## CI/CD & Deployment

**Hosting:**
- No application hosting or automated firmware deployment exists; firmware is built locally or in CI and manually uploaded over USB with `pio run -e jc4827w543 -t upload` from `justfile`.
- Production execution is on the Guition JC4827W543 hardware described in `docs/hardware.md`, not a cloud runtime.

**CI Pipeline:**
- GitHub Actions runs on pushes and pull requests using `ubuntu-latest`, Python `3.12`, and `platformio==6.1.18`, then executes native tests and the ESP32 build in `.github/workflows/build.yml`.
- CI actions are referenced as `actions/checkout@v7` and `actions/setup-python@v7`; dependencies are not pinned to commit SHAs in `.github/workflows/build.yml`.
- CI does not upload firmware, publish binaries, or deploy; `.github/workflows/build.yml` ends after `pio run -e jc4827w543`.

## Environment Configuration

**Required env vars:**
- None: builds and tests require no environment variables in `platformio.ini`, `justfile`, or `.github/workflows/build.yml`.
- Runtime network access optionally requires `WIFI_SSID` and `WIFI_PASSWORD` preprocessor macros in `include/wifi_config.h`; absent credentials degrade to offline operation through fallback macros in `src/services/clock_service.cpp`.

**Secrets location:**
- Wi-Fi credentials live only in gitignored `include/wifi_config.h`; the safe template is `include/wifi_config.example.h`, and `AGENTS.md` prohibits committing or logging the credential file's contents.
- No cloud credentials, database URLs, auth secrets, or CI secrets are consumed by `.github/workflows/build.yml`, `platformio.ini`, or `src/`.

## Hardware Interfaces

**Display and backlight:**
- NV3041A 480×272 RGB565 display over four-data-line QSPI uses LCD CS/SCK/D0/D1/D2/D3 GPIO 45/47/21/48/40/39 through Arduino_GFX in `src/board/board_config.h` and `src/board/display_driver.h`.
- Backlight on GPIO 1 uses 5 kHz, 8-bit LEDC PWM with normal and dim brightness loaded from NVS in `src/board/display_driver.cpp`, `src/main.cpp`, and `src/services/settings_store.cpp`.

**Touch:**
- GT911 capacitive touch uses 100 kHz I²C on SDA GPIO 8 and SCL GPIO 4, reset GPIO 38, and interrupt GPIO 3; startup probes addresses `0x5D` then `0x14` and reads the product ID in `src/board/touch_driver.cpp` and `src/board/board_config.h`.

**USB and sound:**
- USB CDC is enabled at boot and serial runs at 115200 baud through build flags and monitor configuration in `platformio.ini`, with diagnostics emitted by `src/main.cpp`.
- No speaker integration is implemented; the unverified `Speak` connector must not be driven, and visual alert is the default according to `docs/hardware.md` and `README.md`.
- All display, touch, Wi-Fi, brightness, and sound integrations still require physical-board acceptance testing defined in `docs/acceptance.md` and noted in `README.md`.

## Webhooks & Callbacks

**Incoming:**
- None: there is no HTTP server, webhook endpoint, broker subscription, or remote command handler in `src/`; interaction is local GT911 touch through LVGL callbacks in `src/ui/app_ui.cpp`.

**Outgoing:**
- None: the firmware sends no webhooks or application API requests; its only outbound network traffic is Wi-Fi association/DHCP and NTP synchronization in `src/services/clock_service.cpp`.

---

*Integration audit: 2026-09-24*
