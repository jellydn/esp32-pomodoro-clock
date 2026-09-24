# 0003. Persist versioned session transitions

Date: 2026-09-24

## Status

Accepted

## Context

The device should recover Pomodoro state after a reset, but ESP32 NVS has finite write endurance.
Writing the remaining time on every UI refresh or every second would add avoidable flash wear. Stored
bytes can also outlive the firmware schema that created them.

## Decision

Persist session state only when state, phase, next phase, or completed-focus count changes. Also save
once when trusted wall time first becomes available during a running phase so the record gains a
recovery deadline. Do not persist periodic countdown changes.

Store the session as one `Record` made of fixed-width integer fields in the `session` Preferences
namespace. Guard it with a magic value, schema version, exact byte-size check, and range validation
before restoring it. Change the schema version when the record layout changes.

Keep persistence coordination in `src/main.cpp` and serialization/recovery in
`src/services/session_store.cpp`; the platform-independent engine remains unaware of NVS.

## Consequences

### Positive

- NVS writes follow meaningful transitions instead of countdown frequency.
- Invalid or incompatible records are rejected before they mutate engine state.
- Persistence details do not enter the portable domain core.

### Negative

- The application loop must track the last saved transition and whether a wall deadline was saved.
- A raw C++ record can change size because of layout or ABI changes, so version and size checks are
  required and old sessions can be discarded after an incompatible change.
- Session persistence and write frequency are not covered by the current native test boundary.

## Alternatives considered

- Save remaining time every second. This would simplify recovery data but increase NVS writes.
- Save only when the user pauses. Resets during a running phase would lose the latest transition.
- Store session fields as unrelated keys. This would make atomic schema validation and migration
  rules less explicit.
