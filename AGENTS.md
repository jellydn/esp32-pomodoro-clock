# AGENTS.md

ESP32-S3 PlatformIO firmware for the Guition JC4827W543 (480×272) — NTP clock + touch Pomodoro.

## Commands

- `just check` — what CI runs: `pio test -e native` then `pio run -e jc4827w543`. Run before any PR.
- `just test` — native engine tests only. Single suite: `pio test -e native -f test_pomodoro_engine`.
- `just build` — firmware build only.
- `just setup` — creates gitignored `include/wifi_config.h` from the example (build works without it; see below).
- `just upload` / `just monitor` — requires the board on USB.

There is no linter, formatter, or typecheck step. CI (`.github/workflows/build.yml`) is only the
two commands above with `platformio==6.1.18`. Keep dependencies pinned (`platformio.ini`,
Renovate-managed).

Pre-commit hooks: `prek.toml` (builtin hygiene/syntax/safety hooks only — no C++ style checks).
Hooks install via `prek install`; run manually with `prek run --all-files`. Don't add heavy
formatters or linters that reformat existing code.

## Structure and testability boundary

- `src/core/` — Pomodoro engine. **The native env compiles only `+<core/*>`** (`platformio.ini`),
  so this is the only natively tested code. Keep it free of Arduino, LVGL, and hardware headers;
  put new testable logic here, everything else in `src/board/`, `src/services/`, `src/ui/`.
- `src/main.cpp` — wires setup/loop: save session → update UI → `lv_timer_handler()` → dimming.
- `src/board/board_config.h` — pin map (QSPI NV3041A + GT911). Source of truth matches
  `docs/hardware.md`. Never substitute the parallel RGB pin map from other board profiles.

## Invariants (do not break)

- Engine time is a monotonic `uint64` microsecond timestamp (`esp_timer_get_time()`), never a UI
  counter or wall clock. NTP steps must not affect an active phase.
- NVS writes happen only on state transitions, never per second. Session record is a versioned
  struct (`kVersion` in `src/services/session_store.cpp`) — bump it if `Record` layout changes,
  and keep the magic/size/field validation.
- Restore of a `Running` phase falls back to `Paused` when wall time is untrusted; don't "fix"
  this by guessing elapsed time.
- `include/wifi_config.h` holds credentials and is gitignored — never commit or log its contents
  (`__has_include` guard in `clock_service.cpp` makes it optional). Timezone is hardcoded
  `SGT-8` (Asia/Singapore) there.

## Hardware status

Display, touch, Wi-Fi, brightness, and sound are **unverified on the physical board** — only
build and engine tests are proven. Any change to `src/board/` or `src/services/clock_service.*`
needs checks in `docs/acceptance.md` before being called done. Do not drive the `Speak`
connector (`docs/hardware.md`): amplifier path unconfirmed; visual alert is the default.

## Scope

No accounts, cloud sync, phone apps, analytics, calendars, or Token Pulse integration (README).
