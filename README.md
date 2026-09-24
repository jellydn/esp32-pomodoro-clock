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
- 📡 Non-blocking Wi-Fi reconnect with connection diagnostics
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

Install dependencies and create the ignored local Wi-Fi configuration:

```sh
just install
just setup
```

Edit only `include/wifi_config.h`. Do not commit credentials.

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
├── docs/                # Hardware evidence and acceptance checklist
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
