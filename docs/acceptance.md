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
- [ ] Start, Reset, 12/24h, and Wi-Fi controls respond at their displayed positions.
- [ ] Press, release, and drag work without phantom input.

## 4. Wi-Fi and NTP

- [ ] Tapping the Wi-Fi icon opens settings; Back returns to the active timer unchanged.
- [ ] Scan remains responsive and lists the strongest unique visible SSIDs with RSSI and security.
- [ ] Selecting an open network starts connection without a password prompt.
- [ ] Selecting a secured network shows a masked touch keyboard and rejects passwords under eight
      characters.
- [ ] The keyboard Close key hides the keyboard without clearing the password; tapping the password
      field opens it again, and Back and Connect stay reachable in both states.
- [ ] The keyboard OK key starts connection for a valid password and keeps the credential screen
      visible for a validation error.
- [ ] Connecting and connected states name the selected SSID without showing its password.
- [ ] Association and DHCP complete; serial diagnostics show IP and RSSI but no credentials.
- [ ] Wrong credentials show an error, do not replace known-good credentials, and do not block the
      timer or touch UI.
- [ ] AP loss shows offline state and retries the saved network without blocking the UI.
- [ ] A network selected in settings reconnects after reset.
- [ ] Forget disconnects, clears saved credentials, and keeps the compile-time fallback disabled
      after reset.
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
