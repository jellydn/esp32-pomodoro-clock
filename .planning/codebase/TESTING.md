# Testing Patterns

**Analysis Date:** 2026-09-24

## Test Framework

**Runner:**
- PlatformIO native test runner with Unity; PlatformIO is pinned to `6.1.18` in CI (`platformio.ini`, `.github/workflows/build.yml`).
- Config: `platformio.ini`

**Assertion Library:**
- Unity macros cover enum, integer, and 64-bit value assertions (`test/native/test_pomodoro_engine/test_main.cpp`).

**Run Commands:**
```bash
just test                         # Run all native tests (`justfile`)
pio test -e native -f test_pomodoro_engine  # Run the single engine suite (`AGENTS.md`)
just check                        # Run native tests, then build firmware (`justfile`)
```
- No watch-mode or coverage command is configured (`justfile`, `platformio.ini`).

## Test File Organization

**Location:**
- Host tests are separate from production code under `test/native/`; the only current suite targets the Pomodoro engine (`test/native/test_pomodoro_engine/test_main.cpp`).

**Naming:**
- Suite directories use `test_<subject>`, the entry point is `test_main.cpp`, and test cases begin with `test_` (`test/native/test_pomodoro_engine/test_main.cpp`).

**Structure:**
```text
test/native/
└── test_pomodoro_engine/
    └── test_main.cpp
```
The complete current structure is represented by `test/native/test_pomodoro_engine/test_main.cpp`.

## Test Structure

**Suite Organization:**
```cpp
int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_completes_at_exact_deadline_once);
  RUN_TEST(test_pause_and_resume_uses_elapsed_monotonic_time);
  return UNITY_END();
}
```
This is the observed registration pattern in `test/native/test_pomodoro_engine/test_main.cpp`.

**Patterns:**
- Each test constructs a fresh `Engine` with short deterministic durations; there is no shared setup or teardown (`test/native/test_pomodoro_engine/test_main.cpp`).
- Tests advance explicit microsecond timestamps rather than sleeping, preserving deterministic monotonic-time behavior (`test/native/test_pomodoro_engine/test_main.cpp`, `src/core/pomodoro_engine.h`).
- Assertions inspect public `Snapshot` values and cast scoped enums for Unity comparison (`test/native/test_pomodoro_engine/test_main.cpp`).
- A local helper, `completeCurrentPhase`, removes repetition for phase-cycle scenarios (`test/native/test_pomodoro_engine/test_main.cpp`).

## Mocking

**Framework:** None (`test/native/test_pomodoro_engine/test_main.cpp`, `platformio.ini`)

**Patterns:**
```cpp
Engine engine(Durations{10, 2, 4});
engine.start(100 * kSecond);
engine.pause(103 * kSecond);
```
Tests inject time and durations directly instead of mocking clocks or framework APIs (`test/native/test_pomodoro_engine/test_main.cpp`).

**What to Mock:**
- No explicit mocking guidance or examples exist; the explicit architecture guidance instead places deterministic logic in framework-free `src/core/` (`AGENTS.md`, `platformio.ini`).

**What NOT to Mock:**
- Current tests exercise the real `pomodoro::Engine`; Arduino, LVGL, Wi-Fi, Preferences/NVS, display, and touch code are outside the native compilation boundary (`platformio.ini`, `test/native/test_pomodoro_engine/test_main.cpp`).

## Fixtures and Factories

**Test Data:**
```cpp
Engine engine(Durations{2, 1, 3});
constexpr std::uint64_t kSecond = 1000000U;
```
Tests use inline duration values and a shared time-unit constant (`test/native/test_pomodoro_engine/test_main.cpp`).

**Location:**
- No external fixtures or factories exist; data and the one helper are local to `test/native/test_pomodoro_engine/test_main.cpp`.

## Coverage

**Requirements:** None enforced; CI runs tests and compilation without a coverage threshold or report (`.github/workflows/build.yml`, `platformio.ini`).

**View Coverage:**
```bash
# No coverage command is configured (`justfile`, `platformio.ini`).
```

## Native Test Boundary

- The `native` environment compiles only `src/core/*` via `build_src_filter = +<core/*>` and enables production sources with `test_build_src = yes` (`platformio.ini`).
- Explicit guidance requires `src/core/` to remain free of Arduino, LVGL, and hardware headers so it stays host-testable (`AGENTS.md`).
- Consequently, native success proves engine state transitions and timestamp arithmetic, not board drivers, UI behavior, Wi-Fi/NTP, Preferences/NVS persistence, or Arduino orchestration (`src/core/pomodoro_engine.cpp`, `src/board/`, `src/ui/`, `src/services/`, `src/main.cpp`).

## Test Types

**Unit Tests:**
- Five deterministic engine tests cover deadline completion, pause/resume timing, four-focus cycle selection, reset behavior, and restoring a running snapshot (`test/native/test_pomodoro_engine/test_main.cpp`).

**Integration Tests:**
- No automated integration tests exist across engine, NVS, clock, UI, display, or touch; firmware compilation verifies those sources can build together (`.github/workflows/build.yml`, `platformio.ini`).

**E2E Tests:**
- Not automated; end-to-end acceptance is a manual physical-board checklist (`docs/acceptance.md`).

## Common Patterns

**Async Testing:**
```cpp
engine.update(2007 * kSecond - 1);
engine.update(2007 * kSecond);
```
Boundary times are supplied synchronously; tests do not use real waits or async infrastructure (`test/native/test_pomodoro_engine/test_main.cpp`).

**Error Testing:**
```cpp
engine.update(50 * kSecond);
TEST_ASSERT_EQUAL_UINT8(1, engine.snapshot(50 * kSecond).completedFocusInCycle);
```
Invalid/failure branches are tested as state invariants rather than exceptions because the engine uses guarded no-op transitions (`test/native/test_pomodoro_engine/test_main.cpp`, `src/core/pomodoro_engine.cpp`).

## CI and Build Verification

- `just check` is the required pre-PR verification and runs `pio test -e native` followed by `pio run -e jc4827w543` (`AGENTS.md`, `justfile`).
- GitHub Actions repeats those commands on pushes and pull requests using Ubuntu, Python 3.12, and pinned PlatformIO `6.1.18` (`.github/workflows/build.yml`).
- The firmware build targets Arduino on ESP32-S3 with GNU++17, pinned ESP32 platform and library versions, 4 MB flash, and OPI PSRAM settings (`platformio.ini`).
- `prek run --all-files` is an optional/manual hygiene and config-syntax check, not part of CI or `just check` (`AGENTS.md`, `prek.toml`, `.github/workflows/build.yml`).

## Physical Hardware Checks

- Compilation and native engine tests do not verify display, touch, Wi-Fi, brightness, sound, NTP behavior, or electrical pin correctness (`AGENTS.md`, `README.md`).
- Before claiming hardware work complete, run the ordered flash, display, touch, Wi-Fi/NTP, clock, Pomodoro, dimming, and soak checks in `docs/acceptance.md`; board and clock-service changes explicitly require them (`AGENTS.md`, `docs/acceptance.md`).
- Key checks include three successful flashes, chip/flash/PSRAM diagnostics, a 30-minute display soak, corner/drag touch input, offline/non-blocking Wi-Fi behavior, timer and reset persistence, dim/wake behavior, and a 24-hour soak (`docs/acceptance.md`).
- Validate the QSPI NV3041A and GT911 profile against the first-device checklist and do not substitute a parallel RGB profile (`src/board/board_config.h`, `docs/hardware.md`).
- Do not drive the `Speak` connector until its amplifier path is confirmed; visual alert remains the safe default (`AGENTS.md`, `docs/hardware.md`).

---

*Testing analysis: 2026-09-24*
