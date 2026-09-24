# Codebase Structure

**Analysis Date:** 2026-09-24

## Directory Layout

```text
esp32-pomodoro-clock/
├── .agents/                 # Amp orb toolchain and dependency setup
├── .github/workflows/       # GitHub Actions native-test and firmware-build workflow
├── docs/                    # Hardware evidence, acceptance checks, and architecture decisions
│   └── adr/                 # Accepted ADRs and their index
├── include/                 # PlatformIO include root for LVGL and local Wi-Fi configuration
├── src/
│   ├── board/               # Board pin profile and display/touch hardware adapters
│   ├── core/                # Portable Pomodoro domain types and engine
│   ├── services/            # Wi-Fi/NTP and Preferences/NVS adapters
│   ├── ui/                  # LVGL screen, rendering, and interaction handlers
│   └── main.cpp             # Arduino composition root and cooperative loop
├── test/native/             # Host-native Unity test suites
├── .gitignore               # Generated/local file exclusions
├── AGENTS.md                # Repository-specific engineering guidance and invariants
├── justfile                 # Developer command aliases
├── platformio.ini           # Embedded and native PlatformIO environments
├── prek.toml                # Repository hygiene pre-commit hooks
├── renovate.json            # Dependency update policy
├── README.md                # User/developer overview and quick start
└── LICENSE                  # MIT license
```

The represented layout is defined by the current tracked files under `src/`, `include/`, `test/`, `docs/`, `.agents/`, and `.github/`, with root tooling in `platformio.ini`, `justfile`, `prek.toml`, and `renovate.json`.

## Directory Purposes

**`src/board/`:**
- Purpose: Keep target-board electrical details and peripheral adapters outside domain/UI code (`src/board/`).
- Contains: Pin/display dimensions, Arduino_GFX QSPI/NV3041A output, PWM backlight, and GT911 I²C/LVGL input (`src/board/board_config.h`, `src/board/display_driver.cpp`, `src/board/touch_driver.cpp`).
- Key files: `src/board/board_config.h`, `src/board/display_driver.h`, `src/board/touch_driver.h`

**`src/core/`:**
- Purpose: Host platform-independent business logic and the only source set included in native tests (`src/core/`, `platformio.ini`).
- Contains: Pomodoro enums/value structs plus the monotonic state-machine implementation (`src/core/pomodoro_types.h`, `src/core/pomodoro_engine.cpp`).
- Key files: `src/core/pomodoro_engine.h`, `src/core/pomodoro_engine.cpp`, `src/core/pomodoro_types.h`

**`src/services/`:**
- Purpose: Encapsulate runtime infrastructure that is neither board rendering/input nor presentation (`src/services/`).
- Contains: Wi-Fi reconnect/NTP clock service, settings persistence, and versioned session persistence (`src/services/clock_service.cpp`, `src/services/settings_store.cpp`, `src/services/session_store.cpp`).
- Key files: `src/services/clock_service.h`, `src/services/settings_store.h`, `src/services/session_store.h`

**`src/ui/`:**
- Purpose: Own the LVGL object tree, rendering policy, and conversion of UI events into domain/settings commands (`src/ui/`).
- Contains: One `AppUi` class split into declaration and implementation (`src/ui/app_ui.h`, `src/ui/app_ui.cpp`).
- Key files: `src/ui/app_ui.h`, `src/ui/app_ui.cpp`

**`include/`:**
- Purpose: Supply headers found through PlatformIO's global include path (`platformio.ini`, `include/`).
- Contains: Committed LVGL compile-time settings and a safe example for an ignored local credential header (`include/lv_conf.h`, `include/wifi_config.example.h`, `.gitignore`).
- Key files: `include/lv_conf.h`, `include/wifi_config.example.h`; local `include/wifi_config.h` is optional and untracked (`justfile`, `.gitignore`).

**`test/native/`:**
- Purpose: Run host-side Unity tests without Arduino, LVGL, or hardware dependencies (`test/native/`, `platformio.ini`).
- Contains: Suite directories in PlatformIO's `test_<name>` convention and a suite `test_main.cpp` (`test/native/test_pomodoro_engine/test_main.cpp`).
- Key files: `test/native/test_pomodoro_engine/test_main.cpp`

**`docs/`:**
- Purpose: Record the candidate hardware profile, checks that require the physical board, and durable architecture decisions (`docs/hardware.md`, `docs/acceptance.md`, `docs/adr/README.md`).
- Contains: Pin/profile evidence, warnings, references, ordered MVP acceptance steps, and numbered ADRs (`docs/hardware.md`, `docs/acceptance.md`, `docs/adr/`).
- Key files: `docs/hardware.md`, `docs/acceptance.md`, `docs/adr/README.md`

**`.github/workflows/`:**
- Purpose: Define repository CI automation (`.github/workflows/build.yml`).
- Contains: A single workflow that pins PlatformIO, runs native tests, and builds target firmware (`.github/workflows/build.yml`).
- Key files: `.github/workflows/build.yml`

**`.agents/`:**
- Purpose: Prepare fresh Amp orbs with the repository toolchain and dependencies (`.agents/setup`).
- Contains: An executable, idempotent setup script that installs pinned PlatformIO, `just`, and `prek` versions before resolving PlatformIO packages (`.agents/setup`).
- Key files: `.agents/setup`

## Key File Locations

**Entry Points:**
- `src/main.cpp`: Embedded `setup()`/`loop()` composition root and lifecycle.
- `test/native/test_pomodoro_engine/test_main.cpp`: Native Unity executable entry point.

**Configuration:**
- `platformio.ini`: Target `jc4827w543` and host `native` environments, source filtering, flags, board memory, and pinned libraries.
- `include/lv_conf.h`: LVGL color, memory, tick, logging, and font selection.
- `include/wifi_config.example.h`: Template for local Wi-Fi credentials consumed by `src/services/clock_service.cpp`.
- `justfile`: Install, setup, test, build, check, clean, upload, and monitor recipes.
- `.agents/setup`: Fresh-orb provisioning for command-line tools and PlatformIO project packages.
- `.github/workflows/build.yml`: CI's PlatformIO version and required commands.
- `prek.toml`: Lightweight file, syntax, conflict, and private-key hooks.
- `renovate.json`: Renovate's recommended dependency update preset.
- `.gitignore`: Excludes PlatformIO output, editor files, compile database, and local Wi-Fi credentials.

**Core Logic:**
- `src/core/pomodoro_engine.cpp`: Timer commands, elapsed-time computation, completion, cycle selection, and restoration.
- `src/core/pomodoro_types.h`: Shared domain state, phase, duration, and snapshot definitions.
- `src/main.cpp`: Cross-layer loop ordering, transition detection, persistence coordination, and dimming.
- `src/services/session_store.cpp`: Binary NVS session schema and restart-recovery policy.
- `src/services/clock_service.cpp`: Non-blocking Wi-Fi/NTP and local clock formatting.
- `src/ui/app_ui.cpp`: UI construction, periodic projection, and interaction mapping.

**Testing:**
- `test/native/test_pomodoro_engine/test_main.cpp`: All current tests for portable engine behavior.
- `platformio.ini`: Restricts native compilation to `src/core/*` and enables Unity.
- `docs/acceptance.md`: Manual hardware acceptance coverage beyond automated tests.

## Naming Conventions

**Files:**
- Production C++ uses lowercase `snake_case` and paired `.h`/`.cpp` files, such as `src/core/pomodoro_engine.h` and `src/core/pomodoro_engine.cpp`.
- Layer-specific names commonly use role suffixes: `_driver` in `src/board/display_driver.*`, `_service` in `src/services/clock_service.*`, and `_store` in `src/services/session_store.*`.
- Test suite folders use PlatformIO's `test_<subject>` pattern and expose `test_main.cpp`, as in `test/native/test_pomodoro_engine/test_main.cpp`.
- Documentation uses lowercase descriptive Markdown names, as in `docs/hardware.md` and `docs/acceptance.md`.
- Local-only configuration adds `.example` before the header extension, as in `include/wifi_config.example.h`, while the copied `include/wifi_config.h` is ignored (`justfile`, `.gitignore`).

**Directories:**
- Production directories are lowercase architectural roles: `src/board/`, `src/core/`, `src/services/`, and `src/ui/`.
- Tests are grouped first by execution environment and then suite: `test/native/test_pomodoro_engine/`.
- Repository documentation and global includes use conventional plural/root names: `docs/` and `include/`.

**C++ Symbols:**
- Namespaces mirror layer/domain roles: `board` in `src/board/display_driver.h`, `pomodoro` in `src/core/pomodoro_engine.h`, `services` in `src/services/clock_service.h`, and `ui` in `src/ui/app_ui.h`.
- Types use PascalCase (`PomodoroEngine` is represented as `pomodoro::Engine`, `ClockService`, `SessionStore`, `AppUi`) in `src/core/pomodoro_engine.h`, `src/services/clock_service.h`, `src/services/session_store.h`, and `src/ui/app_ui.h`.
- Constants use a `k` prefix (`kWidth`, `kReconnectIntervalMs`, `kVersion`) in `src/board/board_config.h`, `src/services/clock_service.cpp`, and `src/services/session_store.cpp`.
- Private data members use a trailing underscore (`state_`, `clock_`, `address_`) in `src/core/pomodoro_engine.h`, `src/ui/app_ui.h`, and `src/board/touch_driver.h`.

## Where to Add New Code

**New Testable Pomodoro Behavior:**
- Primary code: Add portable types/logic under `src/core/`, preserving its freedom from Arduino, LVGL, and hardware headers (`src/core/pomodoro_engine.h`, `platformio.ini`, `AGENTS.md`).
- Tests: Add cases to `test/native/test_pomodoro_engine/test_main.cpp` or a new `test/native/test_<subject>/test_main.cpp` suite (`test/native/test_pomodoro_engine/test_main.cpp`).

**New Board Peripheral or Hardware Capability:**
- Implementation: Add paired driver files under `src/board/` and constants to `src/board/board_config.h` when they belong to the JC4827W543 pin profile (`src/board/`).
- Integration: Construct and schedule the adapter from `src/main.cpp`; document required physical verification in `docs/acceptance.md` where applicable (`src/main.cpp`, `docs/acceptance.md`).

**New Infrastructure Service:**
- Implementation: Add paired service/store files under `src/services/` for networking, time, or persistence concerns (`src/services/`).
- Integration: Wire lifetime and loop calls in `src/main.cpp`; keep hardware-independent policy in `src/core/` when feasible (`src/main.cpp`, `src/core/`).

**New UI Component or Interaction:**
- Implementation: Extend the current single-screen `AppUi` in `src/ui/app_ui.h` and `src/ui/app_ui.cpp`; introduce additional paired UI classes in `src/ui/` only when separation is warranted (`src/ui/`).
- Domain interaction: Invoke explicit engine commands and render `pomodoro::Snapshot` values rather than maintaining a second timer counter (`src/ui/app_ui.cpp`, `src/core/pomodoro_engine.h`).

**Utilities:**
- Shared helpers: Place domain-only helpers in `src/core/`; keep hardware helpers local to `src/board/`, infrastructure helpers local to `src/services/`, and LVGL helpers local to `src/ui/` to maintain current boundaries (`src/core/`, `src/board/`, `src/services/`, `src/ui/`).

## Special Directories

**`.agents/`:**
- Purpose: Provision fresh Amp orbs before an agent session starts (`.agents/setup`).
- Generated: No; maintained as repository lifecycle configuration (`.agents/setup`).
- Committed: Yes; the setup script must reach the default branch to affect future orbs.

**`.pio/`:**
- Purpose: PlatformIO-generated dependencies, objects, firmware, and test output (`platformio.ini`).
- Generated: Yes, by commands in `justfile` and `.github/workflows/build.yml`.
- Committed: No; excluded by `.gitignore`.

**`include/`:**
- Purpose: Mix committed global build configuration/templates with optional machine-local Wi-Fi configuration (`include/lv_conf.h`, `include/wifi_config.example.h`, `justfile`).
- Generated: Partly; `just setup` copies `include/wifi_config.example.h` to local `include/wifi_config.h` (`justfile`).
- Committed: The directory and examples/config are committed, but `include/wifi_config.h` is not (`.gitignore`).

**`.planning/codebase/`:**
- Purpose: Store generated codebase architecture documentation (`.planning/codebase/ARCHITECTURE.md`, `.planning/codebase/STRUCTURE.md`).
- Generated: Yes, by the codemap analysis represented in `.planning/codebase/ARCHITECTURE.md` and `.planning/codebase/STRUCTURE.md`.
- Committed: Repository policy does not ignore `.planning/` in `.gitignore`; commit status is left to the caller (`.gitignore`).

**`docs/`:**
- Purpose: Preserve human-reviewed hardware evidence, manual acceptance procedures, and accepted architecture decisions (`docs/hardware.md`, `docs/acceptance.md`, `docs/adr/README.md`).
- Generated: No; maintained by contributors (`docs/hardware.md`, `docs/acceptance.md`).
- Committed: Yes; these documents are repository sources (`docs/hardware.md`, `docs/acceptance.md`, `docs/adr/`).

---

*Structure analysis: 2026-09-24*
