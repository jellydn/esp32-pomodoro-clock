# Architecture

**Analysis Date:** 2026-09-24

## Pattern Overview

**Overall:** Single-process Arduino firmware with a cooperative superloop, explicit hardware/service/UI boundaries, and a platform-independent domain core (`src/main.cpp`, `src/core/pomodoro_engine.h`).

**Key Characteristics:**
- `setup()` constructs the runtime by coordinating global, statically allocated components; `loop()` advances them in one cooperative application flow without creating separate application tasks or using an application container (`src/main.cpp`).
- The Pomodoro state machine is isolated from Arduino, LVGL, networking, and storage, allowing the native environment to compile only `src/core/` (`src/core/pomodoro_engine.cpp`, `platformio.ini`).
- Board adapters own display, backlight, and touch details; services own network time and NVS; the UI translates LVGL events into domain commands (`src/board/`, `src/services/`, `src/ui/app_ui.cpp`).
- Time has two distinct domains: monotonic microseconds drive an active session, while wall-clock epoch time is used for display and restart recovery only (`src/main.cpp`, `src/services/clock_service.cpp`, `src/services/session_store.cpp`).

## Layers

**Composition / Application Loop:**
- Purpose: Own component lifetimes, startup ordering, periodic orchestration, transition-triggered persistence, and inactivity dimming (`src/main.cpp`).
- Location: `src/main.cpp`
- Contains: Arduino `setup()` and `loop()`, global component instances, save-tracking state, and hardware diagnostics (`src/main.cpp`).
- Depends on: Board drivers, Pomodoro core, services, LVGL, Arduino, and ESP timer APIs (`src/main.cpp`).
- Used by: The Arduino framework invokes `setup()` once and `loop()` repeatedly (`platformio.ini`, `src/main.cpp`).

**Board / Hardware Adapters:**
- Purpose: Encapsulate the JC4827W543 pin profile and adapt NV3041A/GT911 hardware to LVGL (`src/board/board_config.h`, `src/board/display_driver.cpp`, `src/board/touch_driver.cpp`).
- Location: `src/board/`
- Contains: QSPI display flush registration, PWM backlight control, GT911 I²C probing/reads, touch activity reporting, and pin/dimension constants (`src/board/display_driver.h`, `src/board/touch_driver.h`, `src/board/board_config.h`).
- Depends on: Arduino GPIO/Wire/LEDC APIs, Arduino_GFX, LVGL, and the board constants (`src/board/display_driver.cpp`, `src/board/touch_driver.h`).
- Used by: Startup, LVGL rendering/input, and loop-level inactivity dimming (`src/main.cpp`).

**Domain Core:**
- Purpose: Implement timer state transitions, phase cycling, monotonic elapsed-time calculation, and serializable snapshots (`src/core/pomodoro_engine.cpp`, `src/core/pomodoro_types.h`).
- Location: `src/core/`
- Contains: `pomodoro::Engine`, `State`, `Phase`, `Durations`, and `Snapshot` (`src/core/pomodoro_engine.h`, `src/core/pomodoro_types.h`).
- Depends on: Only standard fixed-width integer types and its own domain types (`src/core/pomodoro_types.h`, `src/core/pomodoro_engine.h`).
- Used by: The application loop, UI, session persistence, and native tests (`src/main.cpp`, `src/ui/app_ui.cpp`, `src/services/session_store.cpp`, `test/native/test_pomodoro_engine/test_main.cpp`).

**Services:**
- Purpose: Isolate infrastructure concerns: Wi-Fi/NTP clock access and Preferences/NVS persistence (`src/services/clock_service.cpp`, `src/services/settings_store.cpp`, `src/services/session_store.cpp`).
- Location: `src/services/`
- Contains: `ClockService`, `SettingsStore`/`Settings`, and `SessionStore` (`src/services/clock_service.h`, `src/services/settings_store.h`, `src/services/session_store.h`).
- Depends on: Arduino Wi-Fi/time APIs, Preferences/NVS, optional local Wi-Fi configuration, and the domain engine for session snapshots/restoration (`src/services/clock_service.cpp`, `src/services/settings_store.h`, `src/services/session_store.h`).
- Used by: The composition root and `AppUi`; the UI accesses the clock and settings but session persistence remains loop-owned (`src/main.cpp`, `src/ui/app_ui.h`).

**UI / Presentation:**
- Purpose: Build the single LVGL screen, render clock/session snapshots, and map touch button events to engine/settings operations (`src/ui/app_ui.cpp`).
- Location: `src/ui/`
- Contains: `AppUi`, LVGL object handles, static event callbacks, rendering throttling, styles, and button handlers (`src/ui/app_ui.h`, `src/ui/app_ui.cpp`).
- Depends on: LVGL, Arduino `millis()`, the domain engine, `ClockService`, and settings persistence (`src/ui/app_ui.h`, `src/ui/app_ui.cpp`).
- Used by: `setup()` initializes it and `loop()` refreshes it before LVGL handles timers (`src/main.cpp`).

## Data Flow

**Startup / Setup Composition:**
1. Static objects wire `AppUi` to shared references for the engine, clock, settings, and settings store before Arduino startup (`src/main.cpp`).
2. `setup()` starts serial diagnostics, loads settings from the `settings` NVS namespace, and initializes LVGL (`src/main.cpp`, `src/services/settings_store.cpp`).
3. The display initializes PWM/GFX, shows a pre-LVGL boot screen, registers its LVGL flush adapter, and applies persisted brightness (`src/main.cpp`, `src/board/display_driver.cpp`).
4. Touch probes GT911 addresses `0x5D` then `0x14`; success registers an LVGL pointer input, while failure is logged and startup continues (`src/main.cpp`, `src/board/touch_driver.cpp`).
5. The clock service starts station-mode Wi-Fi, then session persistence restores the engine using current monotonic time and the current wall-time trust result (`src/main.cpp`, `src/services/clock_service.cpp`, `src/services/session_store.cpp`).
6. The initial snapshot/save bookkeeping is captured, the LVGL widget tree is built, and inactivity timing begins (`src/main.cpp`, `src/ui/app_ui.cpp`).

**Main Loop Composition:**
1. Each iteration reads `esp_timer_get_time()`, advances Wi-Fi/NTP connection management, and updates the engine for deadline completion (`src/main.cpp`).
2. A snapshot is compared with the last saved state/phase/next-phase/completion count; persistence runs only for such transitions or once when a running session first gains trusted wall time (`src/main.cpp`).
3. `AppUi::update()` renders at most every 100 ms, then `lv_timer_handler()` performs LVGL work (`src/main.cpp`, `src/ui/app_ui.cpp`).
4. Touch activity restores configured brightness and resets inactivity timing; otherwise the backlight dims after `dimAfterSeconds`; the loop yields with a 5 ms delay (`src/main.cpp`).

**Pomodoro State and Monotonic Time:**
1. LVGL click callbacks recover `AppUi` from event user data and invoke start, pause, resume, acknowledge-and-start-next, or reset with the most recently supplied monotonic timestamp (`src/ui/app_ui.cpp`).
2. `Engine` stores a monotonic start anchor and remaining duration at that anchor; snapshots derive remaining time from elapsed microseconds rather than decrementing a counter (`src/core/pomodoro_engine.cpp`).
3. `Engine::update()` changes a running phase to `Alert` at zero, increments focus completion count, and chooses short or fourth-cycle long break; acknowledging the alert activates the chosen phase (`src/core/pomodoro_engine.cpp`).
4. UI rendering pulls a fresh immutable `Snapshot` for timer text, progress, phase, primary-button label, and completed-focus count (`src/ui/app_ui.cpp`, `src/core/pomodoro_types.h`).

**Clock and Wall Time:**
1. `ClockService::begin()` starts non-blocking station Wi-Fi and installs connection diagnostics; `update()` retries every 30 seconds while disconnected (`src/services/clock_service.cpp`).
2. On first connectivity, `configTzTime()` configures `SGT-8` with three NTP servers; epoch values at or above the fixed minimum are considered trusted (`src/services/clock_service.cpp`).
3. The UI displays placeholders until time is trusted, then formats local date/time independently of the engine's monotonic timing (`src/ui/app_ui.cpp`, `src/services/clock_service.cpp`).

**Session Persistence and Restore:**
1. On a qualifying transition, `loop()` sends a domain `Snapshot` and wall-time validity to `SessionStore::save()` (`src/main.cpp`, `src/services/session_store.h`).
2. `SessionStore` writes one versioned binary `Record` to the `session` Preferences namespace; a running trusted session also stores a rounded-up wall-clock deadline (`src/services/session_store.cpp`).
3. At boot, restore requires the exact record size plus valid magic, version, enum ranges, and focus count (`src/services/session_store.cpp`).
4. A saved running phase uses its wall deadline when current wall time is trusted; otherwise it is deliberately restored paused. The reconstructed snapshot is anchored to the new monotonic timestamp and immediately updated, allowing an expired trusted deadline to complete (`src/services/session_store.cpp`, `src/core/pomodoro_engine.cpp`).

**Settings Persistence:**
1. Startup reads format, normal brightness, dim brightness, and dim timeout from the `settings` Preferences namespace with struct defaults as fallbacks (`src/services/settings_store.cpp`, `src/services/settings_store.h`).
2. The 12/24-hour button mutates the shared `Settings` instance and writes all settings immediately; loop-level brightness behavior observes the same shared instance (`src/ui/app_ui.cpp`, `src/main.cpp`).

**State Management:**
- Domain state is private to the single `pomodoro::Engine`; callers observe it through value `Snapshot`s and mutate it through commands (`src/core/pomodoro_engine.h`, `src/core/pomodoro_types.h`).
- Runtime dependencies and mutable settings are shared by reference through global-lifetime objects; there is no event bus, scheduler, or dependency-injection framework (`src/main.cpp`, `src/ui/app_ui.h`).
- LVGL object state is owned as pointers inside `AppUi`, while touch activity and display brightness state bridge board drivers and the loop (`src/ui/app_ui.h`, `src/board/touch_driver.h`, `src/main.cpp`).

## Key Abstractions

**Pomodoro Engine and Snapshot:**
- Purpose: Separate state-machine mutation from read-only presentation/persistence data (`src/core/pomodoro_engine.h`, `src/core/pomodoro_types.h`).
- Examples: `src/core/pomodoro_engine.cpp`, `src/core/pomodoro_types.h`
- Pattern: Stateful domain object with command methods and immutable-by-value snapshots (`src/core/pomodoro_engine.h`).

**LVGL Hardware Adapters:**
- Purpose: Translate LVGL's callback-based display/input contracts into object-owned hardware drivers (`src/board/display_driver.cpp`, `src/board/touch_driver.cpp`).
- Examples: `src/board/display_driver.h`, `src/board/touch_driver.h`
- Pattern: Static callbacks recover instances through LVGL `user_data` (`src/board/display_driver.cpp`, `src/board/touch_driver.cpp`).

**Versioned Session Record:**
- Purpose: Persist enough domain state for safe restart recovery while detecting incompatible or corrupt NVS bytes (`src/services/session_store.cpp`).
- Examples: `src/services/session_store.cpp`
- Pattern: Private fixed-layout data-transfer record guarded by magic, schema version, exact byte size, and range validation (`src/services/session_store.cpp`).

**Application-Owned Persistence Detection:**
- Purpose: Avoid per-render/per-second writes by detecting meaningful domain transitions outside the engine (`src/main.cpp`).
- Examples: `src/main.cpp`
- Pattern: Compare selected snapshot fields against `lastSavedSession`, with an additional one-shot trusted-deadline condition (`src/main.cpp`).

## Entry Points

**Firmware Setup:**
- Location: `src/main.cpp`
- Triggers: Arduino invokes `setup()` after boot (`platformio.ini`, `src/main.cpp`).
- Responsibilities: Diagnostics, settings load, LVGL/display/touch initialization, Wi-Fi start, session restore, UI creation, and inactivity initialization (`src/main.cpp`).

**Firmware Loop:**
- Location: `src/main.cpp`
- Triggers: Arduino repeatedly invokes `loop()` after setup (`platformio.ini`, `src/main.cpp`).
- Responsibilities: Advance clock and timer, persist transitions, update UI/LVGL, manage dimming, and yield (`src/main.cpp`).

**Native Test Process:**
- Location: `test/native/test_pomodoro_engine/test_main.cpp`
- Triggers: `pio test -e native`, exposed as `just test` and run by CI (`platformio.ini`, `justfile`, `.github/workflows/build.yml`).
- Responsibilities: Execute Unity tests against only the portable core's deadline, pause/resume, cycle, reset, and restore behavior (`test/native/test_pomodoro_engine/test_main.cpp`, `platformio.ini`).

## Error Handling

**Strategy:** Firmware favors validation, logged diagnostics, and degraded operation rather than exceptions or startup aborts (`src/main.cpp`, `src/services/session_store.cpp`).

**Patterns:**
- Display initialization failure and touch absence are printed; startup continues, and touch is only registered after a successful probe (`src/main.cpp`).
- I²C operations return booleans, reject invalid touch counts, clear GT911 status, and clamp coordinates to display bounds (`src/board/touch_driver.cpp`).
- Missing Wi-Fi configuration compiles via empty fallback macros and produces a diagnostic instead of failing the build (`src/services/clock_service.cpp`, `include/wifi_config.example.h`).
- Invalid/missing session bytes cause `restore()` to return false without mutating the engine; the caller intentionally does not branch on that result (`src/services/session_store.cpp`, `src/main.cpp`).
- Engine commands are idempotent no-ops when invoked from an incompatible state (`src/core/pomodoro_engine.cpp`).

## Cross-Cutting Concerns

**Logging:** Serial output reports hardware, display/touch failures, Wi-Fi events, and NTP setup; the core and UI do not log, and LVGL logging is disabled (`src/main.cpp`, `src/services/clock_service.cpp`, `include/lv_conf.h`).

**Validation:** NVS session records have structural/range checks, touch input is bounded, brightness is capped at 100%, and trusted wall time uses a minimum epoch threshold (`src/services/session_store.cpp`, `src/board/touch_driver.cpp`, `src/board/display_driver.cpp`, `src/services/clock_service.cpp`).

**Authentication:** There is no user authentication; Wi-Fi credentials are supplied through ignored `include/wifi_config.h`, whose committed template is `include/wifi_config.example.h` (`.gitignore`, `include/wifi_config.example.h`, `src/services/clock_service.cpp`).

---

*Architecture analysis: 2026-09-24*
