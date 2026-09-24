# 0002. Separate active timer time from recovery time

Date: 2026-09-24

## Status

Accepted

## Context

NTP can correct wall time while a Pomodoro phase is active. Using wall time for elapsed duration
could therefore shorten or extend a session. A monotonic clock is stable during one boot, but it
cannot show how much time passed while the device was powered off.

## Decision

Use the 64-bit monotonic timestamp from `esp_timer_get_time()` for active phase calculations.
`pomodoro::Engine` stores a monotonic start anchor and derives remaining time from elapsed
microseconds; the UI does not own or decrement a second counter.

Use wall-clock time only for clock display and restart recovery. When wall time is trusted, save an
epoch deadline for a running phase. On restore, calculate the remaining duration from that deadline
and establish a new monotonic anchor. If wall time is not trusted, restore a saved running phase as
paused instead of estimating elapsed time.

## Consequences

### Positive

- NTP corrections do not change the duration of an active phase.
- UI delays and rendering frequency do not control timer accuracy.
- A trusted clock permits running sessions to account for time across a restart.
- An untrusted clock produces an explicit paused state instead of a false deadline.

### Negative

- Restart behavior depends on whether wall time is trusted at boot.
- Recovery code must coordinate wall-clock deadlines with a new monotonic anchor.
- A device without trusted time requires the user to resume an interrupted running phase.

## Alternatives considered

- Decrement a UI counter once per second. Rendering delays would accumulate timer error and make the
  UI the source of domain state.
- Use wall time for all timer calculations. NTP steps could alter active-session duration.
- Continue from the saved remaining value after every restart. This would ignore time spent powered
  off and present an unsupported estimate as accurate.
