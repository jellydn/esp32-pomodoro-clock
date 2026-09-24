# ESP32 Pomodoro Clock

Always-on NTP clock and touch-controlled Pomodoro firmware for the Guition JC4827W543
ESP32-S3 480×272 display.

## Current MVP

- NV3041A QSPI display through Arduino_GFX
- LVGL 8.4 user interface
- Direct GT911 I²C driver with `0x5D`/`0x14` detection
- Non-blocking Wi-Fi reconnect and NTP
- Asia/Singapore timezone and persistent 12/24-hour setting
- Independent monotonic Pomodoro engine
- 25/5/15-minute cycle with a long break after four focus sessions
- Start, pause/resume, reset, progress, and focus count
- Versioned Preferences/NVS session state
- Automatic backlight dimming
- Visual completion alert; sound remains disabled pending hardware verification

The board profile is based on the matching factory package and must pass the checks in
[`docs/hardware.md`](docs/hardware.md) on the physical unit.

## Setup

Install PlatformIO, then create the ignored Wi-Fi configuration:

```sh
cp include/wifi_config.example.h include/wifi_config.h
```

Edit only `include/wifi_config.h`. Do not commit credentials.

Build and test:

```sh
pio test -e native
pio run -e jc4827w543
```

Connect the board, then upload and monitor:

```sh
pio run -e jc4827w543 -t upload
pio device monitor -b 115200
```

## Timer behavior

The UI does not decrement a counter. `PomodoroEngine` derives remaining time from a 64-bit
monotonic timestamp. NTP corrections therefore cannot shorten or extend an active phase.

Session state is written only on transitions. A running phase stores a wall-clock deadline when
time is trusted. If firmware restarts without trusted time, the saved phase restores paused
instead of guessing how much time passed while power was off.

## Hardware status

Compilation and native engine tests are automated. Display, touch, Wi-Fi, brightness, and sound
must still be verified on the physical JC4827W543. Follow [`docs/acceptance.md`](docs/acceptance.md).

## Scope

Accounts, cloud sync, phone apps, analytics, calendars, and Token Pulse integration are excluded.
