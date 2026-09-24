# Hardware profile and verification

## Target

- PCB silkscreen observed on the physical board: `JC4827W543`
- Product listing: 480×272, 4 MB flash, 8 MB PSRAM
- Expected MCU module: ESP32-S3-WROOM-1-N4R8
- Expected display: NV3041A over four-data-line QSPI
- Expected touch: GT911 capacitive controller over I²C

The display and touch values below come from the matching Guition factory package and the
independent Token Pulse reference. They are not inferred from the board photograph.

## Candidate pin map

| Function | GPIO |
| --- | ---: |
| LCD CS | 45 |
| LCD SCK | 47 |
| LCD D0 | 21 |
| LCD D1 | 48 |
| LCD D2 | 40 |
| LCD D3 | 39 |
| Backlight | 1 |
| GT911 SDA | 8 |
| GT911 SCL | 4 |
| GT911 reset | 38 |
| GT911 interrupt | 3 |

Do not use the unrelated 16/24-bit parallel RGB pin map published for another board profile.
The product phrase “RGB 65K” describes RGB565 color depth, not the electrical bus.

## First-device acceptance checks

Record the serial log and mark each item before application testing:

- [ ] Boot report identifies ESP32-S3.
- [ ] Flash report is 4 MB.
- [ ] PSRAM report is 8 MB.
- [ ] Backlight turns on without flicker.
- [ ] Boot screen is stable, correctly oriented, and uses the full 480×272 area.
- [ ] GT911 responds at `0x5D` or `0x14` and returns a readable product ID.
- [ ] Center and all four screen corners report correct touch coordinates.
- [ ] Wi-Fi-only test associates, receives DHCP configuration, and prints RSSI.
- [ ] NTP sets Asia/Singapore time and time continues after Wi-Fi is removed.

## Sound

The board has a connector marked `Speak`, but the photographed unit has no attached speaker.
Visual alert is the MVP default. Do not drive the connector until its amplifier and control path
are confirmed from the matching schematic.

## References

- Guition factory-package mirror: <https://github.com/lsdlsd88/JC4827W543>
- Token Pulse reference PR: <https://github.com/jellydn/token-pulse/pull/11>
