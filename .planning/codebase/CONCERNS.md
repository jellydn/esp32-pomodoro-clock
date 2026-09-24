# Codebase Concerns

**Analysis Date:** 2026-09-24

## Tech Debt

**Hardware behavior and application orchestration are tightly coupled:**
- Issue: The single Arduino `setup()`/`loop()` path owns persistence decisions, UI servicing, touch-driven dimming, clock updates, and timer updates, while all collaborators are concrete global objects.
- Files: `src/main.cpp`, `src/ui/app_ui.cpp`, `src/board/display_driver.cpp`, `src/board/touch_driver.cpp`
- Impact: Service and failure-path behavior cannot be exercised on the native target without substantial hardware/LVGL mocking, so integration regressions are primarily discovered on-device.
- Fix approach: Move transition detection, persistence policy, and dimming policy into hardware-independent coordinators under `src/core/`, leaving `src/main.cpp` as wiring.

**Persistence formats have uneven schema discipline:**
- Issue: Session data has magic/version/size checks, but stores a raw C++ `Record` whose padding and ABI are part of the persisted representation; settings are independent unversioned NVS keys and loaded values are not range-checked.
- Files: `src/services/session_store.cpp`, `src/services/settings_store.cpp`, `src/services/settings_store.h`
- Impact: Compiler/layout changes can invalidate sessions, and stale or corrupted brightness/dimming values can produce unexpected display behavior. Settings migrations have no explicit schema boundary.
- Fix approach: Serialize fixed-width fields explicitly, document and test migration behavior, version settings, and clamp loaded percentages and timeout values before use.

**No automated C++ static/style analysis:**
- Issue: CI runs native tests and a firmware compile, while hooks cover file/config hygiene only.
- Files: `.github/workflows/build.yml`, `prek.toml`, `AGENTS.md`
- Impact: Warning-level defects, unsafe conversions, and style drift can pass the established checks.
- Fix approach: Add targeted compiler warnings first, then a lightweight analysis step that does not mass-reformat existing code.

## Known Bugs

**No repository-verified runtime bug is documented:**
- Symptoms: The repository explicitly distinguishes successful compilation/native engine tests from unverified device behavior; hardware uncertainties below must not be treated as confirmed defects.
- Files: `README.md`, `AGENTS.md`, `docs/acceptance.md`, `docs/hardware.md`
- Trigger: Physical JC4827W543 operation has not completed the documented acceptance checklist.
- Workaround: Use `docs/acceptance.md` and retain logs/results before classifying observed behavior as a bug.

## Security Considerations

**Wi-Fi credentials are compile-time plaintext secrets:**
- Risk: `WIFI_SSID` and `WIFI_PASSWORD` are compiled into firmware; credentials can be exposed through accidental file handling or firmware extraction. The SSID is also printed to serial during connection attempts.
- Files: `include/wifi_config.example.h`, `.gitignore`, `src/services/clock_service.cpp`, `README.md`
- Current mitigation: The real `include/wifi_config.h` is gitignored, optional through `__has_include`, and the password is not logged; the hook set includes private-key detection.
- Recommendations: Keep credentials out of logs, avoid distributable firmware built with personal credentials, consider device provisioning/NVS storage for shared builds, and add secret scanning suited to Wi-Fi macro patterns.

**Clock trust depends on unauthenticated network time:**
- Risk: Public NTP responses determine wall time and restart deadlines; an untrusted network can influence restored running-session duration even though active timers use a monotonic clock.
- Files: `src/services/clock_service.cpp`, `src/services/session_store.cpp`, `src/core/pomodoro_engine.cpp`
- Current mitigation: Wall time must exceed a minimum epoch, and active phase timing uses `esp_timer_get_time()` rather than wall time.
- Recommendations: Document the trust boundary, cap restored deadlines to the configured phase duration, and treat implausible backward/forward wall-time changes as untrusted.

## Performance Bottlenecks

**Display and touch work runs synchronously in the main loop:**
- Problem: LVGL flushes call the display driver synchronously, while touch polling performs blocking I²C transactions and the loop includes a fixed delay.
- Files: `src/main.cpp`, `src/board/display_driver.cpp`, `src/board/touch_driver.cpp`, `src/ui/app_ui.cpp`
- Cause: There is no DMA completion path, task separation, or measured loop-time budget; a 20-row draw buffer causes repeated transfers for large redraws.
- Improvement path: Measure frame/loop latency and heap on hardware first, then consider DMA/asynchronous flush or tuned buffering only if acceptance tests show missed input or display lag.

## Fragile Areas

**Candidate board pin map and peripheral initialization:**
- Files: `src/board/board_config.h`, `src/board/display_driver.h`, `src/board/touch_driver.cpp`, `docs/hardware.md`
- Why fragile: The profile is sourced from external references but remains physically unverified; bus type, orientation, backlight polarity, touch reset/address selection, and coordinate mapping are hardware-specific.
- Safe modification: Do not substitute parallel-RGB profiles; validate changes on the exact JC4827W543 using the first-device and display/touch checks in `docs/hardware.md` and `docs/acceptance.md`.
- Test coverage: The native environment excludes `src/board/`; display, touch, backlight, Wi-Fi, PSRAM, and sound have no automated or recorded physical verification.

**Session restore and transition-triggered writes:**
- Files: `src/main.cpp`, `src/services/session_store.cpp`, `src/core/pomodoro_engine.cpp`, `AGENTS.md`
- Why fragile: Correctness spans monotonic time, wall-clock validity, a persisted deadline, and `lastSavedSession`/`savedWithWallTime` bookkeeping. Changing snapshot fields or transition rules can silently alter NVS write frequency or restart semantics.
- Safe modification: Preserve the paused fallback for untrusted wall time, keep writes transition-only, bump `kVersion` for record layout changes, and retain magic/size/enum validation.
- Test coverage: Engine restore anchoring is tested, but NVS serialization, expired deadlines, invalid records, boot before NTP, NTP becoming valid while running, and write-count behavior are not.

**Persisted numeric values are only partially validated:**
- Files: `src/services/session_store.cpp`, `src/services/settings_store.cpp`, `src/main.cpp`, `src/board/display_driver.cpp`
- Why fragile: Session enum/count fields are checked but `remainingMs` and deadline plausibility are not; settings loaded from NVS are accepted without bounds before brightness and timeout calculations.
- Safe modification: Validate persisted values against explicit maxima and configured phase durations, clamp brightness to 0–100, and define behavior for invalid settings/session records.
- Test coverage: No native tests cover either persistence service or malformed/stale stored data.

## Scaling Limits

**Fixed embedded memory and UI buffers:**
- Current capacity: The target is configured for 4 MB flash and 8 MB PSRAM; LVGL uses a 48 KiB heap and the display driver statically allocates a `480 × 20` RGB565 draw buffer (19,200 bytes).
- Limit: Additional screens, fonts, widgets, or buffering consume fixed MCU RAM/flash, and no automated heap watermark or long-soak assertion exists.
- Scaling path: Measure free/internal heap and fragmentation during the 30-minute display and 24-hour soak checks before expanding UI assets; size buffers from measured hardware results.
- Files: `platformio.ini`, `include/lv_conf.h`, `src/board/display_driver.h`, `src/main.cpp`, `docs/acceptance.md`

## Dependencies at Risk

**Board support is represented by a generic PlatformIO board definition:**
- Risk: The firmware uses `esp32-s3-devkitc-1` plus custom flash/PSRAM flags for a JC4827W543, so upstream board defaults can diverge from the actual module or upload/partition requirements.
- Impact: Builds may succeed while flash, PSRAM, USB, upload speed, or runtime peripheral assumptions fail on the physical board.
- Migration plan: After hardware verification, capture a dedicated board manifest or lock all required overrides and keep the boot hardware report as an acceptance gate.
- Files: `platformio.ini`, `src/main.cpp`, `docs/hardware.md`

**Firmware libraries and platform require coordinated upgrades:**
- Risk: ESP32 platform, Arduino_GFX, and LVGL are pinned, but compatibility is validated only by compilation and native core tests; Renovate can propose updates without physical-device coverage.
- Impact: Display initialization, LVGL driver APIs, touch timing, Wi-Fi events, or LEDC behavior could regress despite a green host test.
- Migration plan: Upgrade one dependency group at a time, require `just check`, and run the relevant physical acceptance sections before declaring an update complete.
- Files: `platformio.ini`, `renovate.json`, `.github/workflows/build.yml`, `docs/acceptance.md`

## Missing Critical Features

**Physical-board qualification evidence:**
- Problem: Display, touch, Wi-Fi/NTP, brightness, sound, reset persistence, and long-run stability are explicitly unverified; the sound connector must not be driven until its amplifier/control path is confirmed.
- Blocks: Production-readiness claims, confident peripheral changes, and enabling audible alerts.
- Files: `AGENTS.md`, `README.md`, `docs/acceptance.md`, `docs/hardware.md`

**Failure diagnostics do not satisfy all acceptance evidence:**
- Problem: The checklist requests reset reason, gateway, DNS, and detailed Wi-Fi diagnostics, while current serial output reports chip/flash/PSRAM/free heap, local IP/RSSI, disconnect reason, and touch identity only.
- Blocks: Completing the documented flash and Wi-Fi acceptance evidence without temporary instrumentation.
- Files: `docs/acceptance.md`, `docs/hardware.md`, `src/main.cpp`, `src/services/clock_service.cpp`

## Test Coverage Gaps

**Hardware, service, UI, and integration layers:**
- What's not tested: `src/board/`, `src/services/`, `src/ui/`, and `src/main.cpp` are excluded from native compilation; there are no mocks or hardware-in-loop tests for display flush, GT911 protocol/coordinates, Wi-Fi reconnect, NTP validity, NVS, UI callbacks, dimming, or application transition persistence.
- Files: `platformio.ini`, `test/native/test_pomodoro_engine/test_main.cpp`, `src/board/`, `src/services/`, `src/ui/`, `src/main.cpp`
- Risk: Firmware compilation can remain green despite runtime wiring, persistence, or peripheral failures.
- Priority: High

**Engine edge cases and input invariants:**
- What's not tested: Invalid/restored snapshots, zero or extreme durations, monotonic timestamp wrap/backward input assumptions, no-op calls in incompatible states, reset during every phase/state, and alert acknowledgement without immediate restart.
- Files: `src/core/pomodoro_engine.cpp`, `src/core/pomodoro_types.h`, `test/native/test_pomodoro_engine/test_main.cpp`
- Risk: Future callers or persistence changes can violate implicit invariants without a failing test.
- Priority: Medium

**Persistence/schema compatibility:**
- What's not tested: Raw `Record` size/layout compatibility, magic/version rejection, enum/count validation, settings defaults and corruption, deadline rounding/expiry, and migrations between schema versions.
- Files: `src/services/session_store.cpp`, `src/services/settings_store.cpp`, `test/native/test_pomodoro_engine/test_main.cpp`
- Risk: Upgrades or corrupted NVS can lose state or restore implausible values unnoticed.
- Priority: High

**Acceptance and soak boundaries:**
- What's not tested: Three consecutive flashes, 30-minute display stability, wrong-credential/AP-loss behavior, offline clock continuation, full real-duration Pomodoro cycles, timer accuracy under delayed UI work, and 24-hour heap/drift stability.
- Files: `docs/acceptance.md`, `README.md`, `.github/workflows/build.yml`
- Risk: Timing, memory, connectivity, and power-cycle defects remain outside CI and current evidence.
- Priority: High

---

*Concerns audit: 2026-09-24*
