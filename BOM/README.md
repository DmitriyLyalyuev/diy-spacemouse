# BOM

The CSV files contain local Ozon links for parts used in this build. Links may change over time; verify dimensions, quantities, and availability before ordering.

## Files

- [`Parts_BOM.csv`](Parts_BOM.csv) — electronics and small electrical parts.
- [`Hardware_BOM.csv`](Hardware_BOM.csv) — magnets, springs, steel balls, inserts, and screws.

## Columns

- `Qty` — quantity used or recommended for one build.
- `Item` — part description, usually in Russian because the links are local.
- `Link` — local supplier URL used during the original build.

## Caveats

- The RP2040 board row currently points to a local Raspberry Pi Pico source. The firmware documented in this repository is tested on **Adafruit QT Py RP2040** with FQBN `rp2040:rp2040:adafruit_qtpy:usbstack=picosdk`. A Pico is not a drop-in replacement; check pinout, I2C bus, GPIO27 Home button wiring, and upload settings.
- The tested sensor wiring uses TLV493D on STEMMA QT / `Wire1`, address `0x5E`.
- Some printed insert holes were oversized in the tested build. M2.5 inserts/screws are listed, but M3 inserts and matching M3 screws may be needed in loose holes.
- Magnets, springs, and steel balls affect centering and feel. Treat the listed dimensions as a starting point and test-fit before final assembly.

For Russian build notes and part caveats, see [`../README_RU.md`](../README_RU.md). For assembly details, see [`../docs/assembly.md`](../docs/assembly.md).
