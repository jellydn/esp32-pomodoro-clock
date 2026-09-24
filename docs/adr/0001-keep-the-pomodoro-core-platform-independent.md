# 0001. Keep the Pomodoro core platform-independent

Date: 2026-09-24

## Status

Accepted

## Context

Most firmware modules require Arduino, LVGL, ESP32, or physical peripherals. These dependencies are
not available in the PlatformIO native test environment. The timer state machine has deadline,
pause/resume, phase-cycle, reset, and restore rules that need deterministic automated tests.

## Decision

Keep Pomodoro domain types and behavior in `src/core/` with no Arduino, LVGL, networking, storage,
or hardware dependencies. Pass monotonic timestamps and configured durations into
`pomodoro::Engine`, and expose state through value `Snapshot` objects.

Configure the PlatformIO `native` environment to compile production sources only from `src/core/`.
Keep board adapters in `src/board/`, infrastructure adapters in `src/services/`, presentation in
`src/ui/`, and composition in `src/main.cpp`.

## Consequences

### Positive

- Engine behavior runs as deterministic host tests without a board or framework mocks.
- The domain model does not depend on LVGL rendering cadence, Wi-Fi, NVS, or peripheral drivers.
- Hardware-independent behavior has a clear destination and test boundary.

### Negative

- Native tests do not cover application wiring, persistence adapters, UI callbacks, or peripherals.
- Logic that remains in `src/main.cpp` or framework-bound modules needs physical or integration
  verification.
- New domain APIs must use portable data rather than framework types.

## Alternatives considered

- Test the complete Arduino firmware on the native target with broad mocks. This would couple tests
  to framework details and require substantial test infrastructure.
- Test only on physical hardware. This would make state-machine boundary cases slower and less
  repeatable.
