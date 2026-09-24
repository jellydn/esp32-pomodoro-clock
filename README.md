# ESP32 Pomodoro Clock 👋

[![Build](https://github.com/jellydn/esp32-pomodoro-clock/actions/workflows/build.yml/badge.svg)](https://github.com/jellydn/esp32-pomodoro-clock/actions/workflows/build.yml)
[![GitHub license](https://img.shields.io/github/license/jellydn/esp32-pomodoro-clock)](LICENSE)
[![GitHub stars](https://img.shields.io/github/stars/jellydn/esp32-pomodoro-clock)](https://github.com/jellydn/esp32-pomodoro-clock/stargazers)

Always-on NTP clock and touch-controlled Pomodoro firmware for the Guition JC4827W543
ESP32-S3 480×272 display.

> [!IMPORTANT]
> The firmware builds and the timer engine is tested. Display, touch, Wi-Fi, brightness, and
> sound still require verification on the physical board.

## ✨ Features

- 🕐 Always-on NTP clock with Asia/Singapore as the default timezone
- ⏱️ Independent monotonic Pomodoro engine that does not decrement a UI counter
- 🍅 25-minute focus, 5-minute short break, and 15-minute long break after four sessions
- 👆 Touch controls for start, pause/resume, reset, and 12/24-hour format
- 📊 Visual progress, current phase, and completed-focus count
- 💾 Versioned Preferences/NVS settings and session state
- 🌙 Automatic backlight dimming after inactivity
- 📡 Touch Wi-Fi setup with nearby-network scan, saved credentials, and non-blocking reconnect
- 🔔 Visual completion alert; optional sound is reserved for verified speaker hardware

The board profile is based on the matching factory package and must pass the checks in
[`docs/hardware.md`](docs/hardware.md) on the physical unit.

## 🧱 Tech stack

| Technology | Purpose |
| --- | --- |
| [PlatformIO](https://platformio.org/) | Reproducible build, upload, and native tests |
| Arduino ESP32 | ESP32-S3 runtime, Wi-Fi, NTP, and Preferences |
| [Arduino_GFX](https://github.com/moononournation/Arduino_GFX) | NV3041A QSPI display driver |
| [LVGL 8.4](https://github.com/lvgl/lvgl) | Touch user interface |
| GT911 | Capacitive touch input over I²C |

## Prerequisites

- [PlatformIO Core](https://docs.platformio.org/en/latest/core/installation/index.html)
- [just](https://github.com/casey/just) (recommended)
- A Guition JC4827W543 connected over USB for upload and hardware verification

## 🚀 Quick start

Install dependencies:

```sh
just install
```

After upload, tap the Wi-Fi icon, scan for a network, select it, and enter its password on the
touch keyboard. The password is masked and saved to the ESP32 Preferences/NVS store only after a
successful connection. The settings screen can also forget the saved network.

For unattended first boot, `just setup` creates the ignored `include/wifi_config.h` fallback.
Edit only that ignored file and never commit credentials. A network selected on the device takes
priority over the fallback; after **Forget**, the fallback stays disabled.

Build and test:

```sh
just check
```

Connect the board, then upload and monitor:

```sh
just upload
just monitor
```

The equivalent raw PlatformIO commands are documented in the [`justfile`](justfile).

## ⏱️ Timer behavior

The UI does not decrement a counter. `PomodoroEngine` derives remaining time from a 64-bit
monotonic timestamp. NTP corrections therefore cannot shorten or extend an active phase.

Session state is written only on transitions. A running phase stores a wall-clock deadline when
time is trusted. If firmware restarts without trusted time, the saved phase restores paused
instead of guessing how much time passed while power was off. NVS writes occur only on state
transitions, not once per second.

## 📡 Wi-Fi setup

The Wi-Fi icon opens the settings screen. Scans and connection attempts are asynchronous, so the
timer and touch UI continue to run. The screen lists up to eight strongest unique visible
networks, marks open and secured networks, and reports scanning, connecting, connected, offline,
timeout, unavailable-network, and authentication-failure states.

Passwords are limited to the ESP32 station limit, are not printed to serial output, and remain
masked during entry. A new SSID and password replace the saved credentials only after the station
connects. **Forget** clears the application credentials and the ESP32 station configuration.
Preferences/NVS is suitable for device-local configuration, but it is not a defense against an
attacker with physical flash access.

## 📦 Project structure

```text
esp32-pomodoro-clock/
├── include/             # LVGL and local Wi-Fi configuration
├── src/
│   ├── board/           # Display, touch, backlight, and pin profile
│   ├── core/            # LVGL-independent Pomodoro engine
│   ├── services/        # Wi-Fi, time, settings, and session persistence
│   └── ui/              # LVGL screen and input actions
├── test/native/         # Host-side engine tests
├── docs/                # Hardware evidence, acceptance checklist, and architecture decisions
├── platformio.ini
└── justfile
```

## 🔧 Development

```sh
just test     # Native engine tests
just build    # ESP32-S3 firmware build
just check    # Test and build, as run by CI
just clean    # Remove generated build files
```

## 🧪 Hardware verification

Compilation and native engine tests are automated. Display, touch, Wi-Fi, brightness, and sound
must still be verified on the physical JC4827W543. Follow [`docs/acceptance.md`](docs/acceptance.md).

## Scope

Accounts, cloud sync, phone apps, analytics, calendars, and Token Pulse integration are excluded.

## 🤝 Contributing

Contributions, issues, and feature requests are welcome. Before opening a pull request, run
`just check` and describe any physical-board checks you completed.

## ⭐ Show your support

Give this project a star if it helped you build a focused desk clock.

## 📜 License

Copyright © 2026 [Dung Duc Huynh](https://github.com/jellydn). This project is
[MIT licensed](LICENSE).

## Author

👤 **Dung Duc Huynh**

- Website: [productsway.com](https://productsway.com)
- GitHub: [@jellydn](https://github.com/jellydn)
