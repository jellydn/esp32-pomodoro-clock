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
- [ ] The passive scan remains responsive, reports raw and unique counts on serial, and lists up to
      eight strongest unique visible 2.4 GHz SSIDs with RSSI and security. Swipe vertically when
      the screen reports more networks than fit at once.
- [ ] The test SSID has a 2.4 GHz radio and WPA2-Personal, WPA3-Personal, or WPA2/WPA3
      transition security. WEP, WPA-only, and enterprise networks are marked unsupported.
- [ ] Selecting an open network starts connection without a password prompt.
- [ ] Selecting a secured network shows a masked touch keyboard and rejects passwords under eight
      characters.
- [ ] The keyboard Close key hides the keyboard without clearing the password; tapping the password
      field opens it again, and Back and Connect stay reachable in both states.
- [ ] The keyboard OK key starts connection for a valid password and keeps the credential screen
      visible for a validation error.
- [ ] Connecting and connected states name the selected SSID without showing its password.
- [ ] Association and DHCP complete within 30 seconds. The UI progresses from `Associating` to
      `Associated; getting IP address`, then `Connected`.
- [ ] Serial diagnostics show the selected SSID and lengths, `stage=associated`, `stage=got-ip`, IP,
      and RSSI, but never show the password. A normal internal transition can report reason 8
      (`ASSOC_LEAVE`) followed by `ignored voluntary driver disconnect`.
- [ ] Wrong credentials show an error, do not replace known-good credentials, and do not block the
      timer or touch UI.
- [ ] A failed connection shows the numeric and named disconnect reason on screen and serial, for
      example `202 AUTH_FAIL`. Record the complete `Wi-Fi stage=...` lines for diagnosis.
- [ ] A transient reason such as `2 AUTH_EXPIRE` shows `stage=retrying` and remains in the
      connecting state. It succeeds on a later attempt or reports the same reason after 30 seconds.
- [ ] A 63-character passphrase and a 64-character hexadecimal PSK can connect without truncation.
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
