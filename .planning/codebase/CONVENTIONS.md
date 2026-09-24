# Coding Conventions

**Analysis Date:** 2026-09-24

## Guidance Status

- **Explicit guidance:** Keep host-testable logic in `src/core/` without Arduino, LVGL, or hardware headers; place hardware, service, and UI concerns under `src/board/`, `src/services/`, and `src/ui/` respectively (`AGENTS.md`, `platformio.ini`).
- **Explicit guidance:** Preserve monotonic microsecond timing, transition-only NVS writes, versioned session records, and paused fallback when wall time is untrusted (`AGENTS.md`, `src/main.cpp`, `src/services/session_store.cpp`).
- **Observed convention:** The style and patterns below are inferred from the current implementation; there is no configured C++ formatter or linter (`AGENTS.md`, `prek.toml`).

## Naming Patterns

**Files:**
- C++ implementation/header pairs use lowercase `snake_case`, grouped by responsibility, such as `src/core/pomodoro_engine.cpp` and `src/core/pomodoro_engine.h`.
- Tests use a PlatformIO suite directory named `test_<subject>` with a `test_main.cpp` entry point (`test/native/test_pomodoro_engine/test_main.cpp`).

**Functions:**
- Methods and free functions use lower `camelCase`, while Unity test functions use descriptive `snake_case` beginning with `test_` (`src/core/pomodoro_engine.cpp`, `test/native/test_pomodoro_engine/test_main.cpp`).
- Boolean queries describe their condition, such as `timeValid`, `wifiConnected`, and `touchedSinceLastRead` (`src/services/clock_service.h`, `src/board/touch_driver.h`).

**Variables:**
- Local variables and parameters use lower `camelCase`; private data members add a trailing underscore (`src/main.cpp`, `src/ui/app_ui.h`).
- Constants use `k` plus PascalCase and are usually `constexpr`; file-private constants live in unnamed namespaces (`src/board/board_config.h`, `src/services/clock_service.cpp`).

**Types:**
- Classes, structs, and scoped enums use PascalCase; enum values also use PascalCase (`src/core/pomodoro_types.h`, `src/services/settings_store.h`).
- Code uses fixed-width `std::uint*_t`/`std::int*_t` types for persisted data, timestamps, pins, and protocol values (`src/services/session_store.cpp`, `src/board/board_config.h`).

## Code Style

**Formatting:**
- Observed C++ style is two-space indentation, braces on the same line, wrapped argument lists, and namespace-closing comments (`src/core/pomodoro_engine.cpp`, `src/ui/app_ui.cpp`).
- Headers use `#pragma once`; classes place public members before private members (`src/core/pomodoro_engine.h`, `src/services/clock_service.h`).
- No formatter is configured, and explicit guidance says not to add a heavy formatter that rewrites existing code (`AGENTS.md`, `prek.toml`).

**Linting:**
- No C++ linter or static type-check step is configured (`AGENTS.md`, `.github/workflows/build.yml`).
- `prek` provides repository hygiene, config syntax, conflict, large-file, and private-key checks, but no C++ style checks (`prek.toml`).

## Import Organization

**Order:**
1. An implementation includes its matching header first (`src/core/pomodoro_engine.cpp`, `src/board/display_driver.cpp`).
2. System/framework headers follow, separated by a blank line (`src/services/clock_service.cpp`, `src/main.cpp`).
3. Project headers follow in responsibility/path order (`src/main.cpp`, `src/ui/app_ui.h`).

**Path Aliases:**
- Project includes are rooted at `src/` where cross-directory references need paths such as `core/pomodoro_engine.h`; same-directory files use local names (`src/services/session_store.h`, `src/core/pomodoro_engine.cpp`).
- `include/` is added explicitly for firmware builds, allowing `lv_conf.h` and optional `wifi_config.h` discovery (`platformio.ini`, `src/services/clock_service.cpp`).

## Error Handling

**Patterns:**
- Invalid state-machine operations are idempotent early returns rather than exceptions (`src/core/pomodoro_engine.cpp`).
- Hardware and persistence operations report success with `bool`; callers log failures or safely continue (`src/board/touch_driver.cpp`, `src/main.cpp`, `src/services/session_store.cpp`).
- Persisted session data is accepted only after size, magic, version, enum-range, and count validation (`src/services/session_store.cpp`).
- Optional Wi-Fi credentials are compile-time guarded and degrade to an unconfigured state instead of breaking the build (`src/services/clock_service.cpp`, `include/wifi_config.example.h`).

## Logging

**Framework:** Arduino `Serial` (`src/main.cpp`, `src/services/clock_service.cpp`)

**Patterns:**
- Startup diagnostics report hardware, display/touch failures, and connectivity transitions (`src/main.cpp`, `src/services/clock_service.cpp`).
- Severity is conveyed in message text with `ERROR:` or `WARNING:` rather than through a logging abstraction (`src/main.cpp`).
- Explicit guidance forbids committing or logging Wi-Fi credentials; current code logs the SSID but not the password (`AGENTS.md`, `src/services/clock_service.cpp`).

## Comments

**When to Comment:**
- Comments are sparse and document hardware provenance or configuration intent rather than restating implementation (`src/board/board_config.h`, `include/wifi_config.example.h`).
- Explicit hardware cautions and behavioral invariants are maintained in repository guidance and hardware documentation (`AGENTS.md`, `docs/hardware.md`).

**JSDoc/TSDoc:**
- No API documentation comment format is used; names, types, and repository docs carry the explanation (`src/core/pomodoro_engine.h`, `README.md`).

## Function Design

**Size:** Functions generally perform one operation, while orchestration remains in Arduino `setup()` and `loop()` (`src/core/pomodoro_engine.cpp`, `src/main.cpp`).

**Parameters:** Dependencies are passed by reference, read-only values use `const` where appropriate, and timestamp-dependent engine calls receive an explicit monotonic `nowUs` (`src/ui/app_ui.h`, `src/core/pomodoro_engine.h`).

**Return Values:** Queries return values or snapshots; fallible initialization, I/O, and restore operations return `bool`; commands commonly return `void` (`src/core/pomodoro_engine.h`, `src/board/touch_driver.h`, `src/services/session_store.h`).

## Module Design

**Exports:** Public interfaces are declared in paired headers and implementations remain in `.cpp` files; file-local helpers/constants use unnamed namespaces (`src/services/clock_service.h`, `src/services/clock_service.cpp`).

**Barrel Files:** No barrel/aggregate headers are used; consumers include each concrete module directly (`src/main.cpp`, `src/ui/app_ui.h`).

---

*Convention analysis: 2026-09-24*
