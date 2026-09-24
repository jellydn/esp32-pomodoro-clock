# Architecture decision records

These records explain durable architecture choices that are easy to break without their context.
They describe the current firmware architecture; hardware evidence and physical validation remain in
[`docs/hardware.md`](../hardware.md) and [`docs/acceptance.md`](../acceptance.md).

| ADR | Status | Decision |
| --- | --- | --- |
| [0001](0001-keep-the-pomodoro-core-platform-independent.md) | Accepted | Keep the Pomodoro core platform-independent |
| [0002](0002-separate-active-timer-time-from-recovery-time.md) | Accepted | Separate active timer time from recovery time |
| [0003](0003-persist-versioned-session-transitions.md) | Accepted | Persist versioned session transitions |

Create future records with the next four-digit number. Do not rewrite an accepted decision when the
architecture changes. Add a new ADR that supersedes it and update this index.
