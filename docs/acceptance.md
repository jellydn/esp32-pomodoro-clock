# MVP acceptance checklist

Run milestones in this order. Do not skip failed hardware checks.

## 1. Flash

- [ ] `pio run -e jc4827w543 -t upload` succeeds three times.
- [ ] Serial output reports expected chip, flash, PSRAM, and reset reason.

## 2. Display

- [ ] Boot screen appears before Wi-Fi starts.
- [ ] Colors, orientation, edges, and text are correct.
- [ ] Thirty-minute display soak has no corruption, flicker, restart, or heap loss.

## 3. Touch

- [ ] GT911 product ID and address are reported.
- [ ] Center and four-corner targets work.
- [ ] Press, release, and drag work without phantom input.

## 4. Wi-Fi and NTP

- [ ] Association, DHCP, RSSI, gateway, and DNS are visible in diagnostics.
- [ ] Wrong credentials and AP loss do not block the UI.
- [ ] Asia/Singapore time becomes valid and continues offline while powered.

## 5. Clock

- [ ] Date and time match a trusted source.
- [ ] 12/24-hour selection survives reset.
- [ ] Invalid time is visibly different from synchronized time.

## 6. Pomodoro

- [ ] Focus 25 min → short break 5 min.
- [ ] Fourth completed focus → long break 15 min.
- [ ] Start, pause/resume, reset, progress, and cycle count are correct.
- [ ] Delayed UI work does not change timer accuracy.
- [ ] State transitions survive reset without per-second NVS writes.

## 7. Dimming and soak

- [ ] Backlight dims after 60 seconds of no touch and returns on touch.
- [ ] Twenty-four-hour run has no crash, drift, or persistent heap loss.
