# OpenHR20

[![Reliability build](https://github.com/joker-mik/OpenHR20/actions/workflows/reliability-build.yml/badge.svg)](https://github.com/joker-mik/OpenHR20/actions/workflows/reliability-build.yml)

OpenHR20 is open firmware for Honeywell Rondostat HR20 and related ATmega-based radiator thermostats. It is a complete open-source firmware implementation and is not based on Honeywell's proprietary firmware.

This fork keeps the original HR20/HR25 functionality while adding reliability fixes, safer motor/calendar handling, runtime window-detection selection, optional RFM12B support and reproducible CI memory checks.

Supported thermostat families:

- Honeywell HR20
- Honeywell HR25
- Thermotronic

## Recommended HR20 build

For most HR20 installations, use:

```sh
make HR20_universal_jd
```

`HR20_universal_jd` provides:

- normal local operation with wheel and keys;
- optional JD_INTERNAL RFM12B with service-menu OFF / ON / AUTO mode and runtime detection;
- the same HEX with or without the RFM module fitted;
- RFM frequency tuning;
- software window detection (toggle between off/software/hardware can be set in service menu at runtime);
- PE2 hardware window-contact detection (toggle between off/software/hardware can be set in service menu at runtime);
- runtime selection of window-detection mode;
- JTAG pins left available because JD_INTERNAL does not use them.

The universal build is checked in CI against the ATmega169P limits of 16 KiB flash and 1 KiB SRAM.

## Which build do I need?

| Hardware / setup | Build |
| --- | --- |
| HR20, recommended universal JD_INTERNAL setup | `HR20_universal_jd` |
| HR20 without RFM | `HR20_original_sww` |
| HR20 with JD_INTERNAL RFM, software window detection | `HR20_rfm_int_sww` |
| HR20 with JD_INTERNAL RFM, hardware PE2 window detection | `HR20_rfm_int_hww` |
| HR20 with MARIOJTAG RFM | `HR20_rfm_ext_sww` |
| HR25, recommended universal TK_INTERNAL setup | `HR25_universal_tk` |
| HR25 without RFM | `HR25_original_sww` |
| HR25 with fixed TK_INTERNAL RFM | `HR25_rfm_int_sww` |
| Thermotronic | `thermotronic_sww` |
| RFM master/gateway | `rfm_master` |

Build all supported targets with:

```sh
make all
```

## Documentation

Use the README as the project start page; detailed information is split into focused guides:

- **[User guide](docs/USER_GUIDE.md)** — wheel/buttons, long-press combinations, date/time, timers, preset temperatures, key lock, service menu and window handling.
- **[UART reference](docs/UART.md)** — 9600-baud serial interface, pinout, command syntax and all local commands.
- **[RFM12B guide](docs/RFM.md)** — thermostat-side JD_INTERNAL, MARIOJTAG, TK_INTERNAL, runtime detection, tuning and protocol notes.
- **[RFM master / gateway](docs/RFM_MASTER.md)** — separate gateway hardware, original master-board schematics and build notes.
- **[Hardware archive](docs/HARDWARE.md)** — index of the historical schematics, PCB files, photographs and measurements under `doc/`.
- **[Build guide](docs/BUILD.md)** — toolchain, build matrix, JTAG behaviour, artifacts and CI memory limits.
- **[Configuration reference](docs/CONFIGURATION.md)** — EEPROM/service-menu parameters, window detection, motor calibration, battery and RFM settings.
- **[Reliability notes](RELIABILITY.md)** — reliability-oriented changes and design notes.

## Quick local operation

The front panel uses **PROG**, **C**, **AUTO** and the rotary wheel.

Common actions:

| Control | Action |
| --- | --- |
| Wheel | change requested temperature |
| AUTO | change operating mode |
| C | cycle alternate home displays |
| Long AUTO | date/time setup |
| Long PROG | switching-time setup |
| Long C | preset-temperature setup |
| Long AUTO + C | toggle key lock |
| Long PROG + AUTO | toggle software-window state when software detection is active |
| Long PROG + C + AUTO | service menu |

See the [User guide](docs/USER_GUIDE.md) for the complete behaviour.

## UART at a glance

Non-RFM HR20 builds expose the local UART at **9600 baud** on PE0/PE1.

The command set includes version/status, watch variables, configuration, timers, date/time, target temperature, mode, key lock and reboot.

Examples:

```text
V
D
Gff
L02
```

See the [UART reference](docs/UART.md) for exact command formats.

RFM builds use the corresponding binary wireless command protocol instead of compiling the local ASCII UART parser.

## RFM12B and JTAG

The recommended universal HR20 radio wiring is **JD_INTERNAL**:

| Signal | AVR pin |
| --- | --- |
| SCK | PF1 |
| SDI | PF0 |
| nSEL | PA3 |
| SDO | PE6 / PCINT6 |

JD_INTERNAL leaves PE2 free for a hardware window contact and does not require JTAG to be disabled.

**MARIOJTAG** reuses JTAG pins and PE2; therefore JTAG must be disabled and PE2 hardware window detection is incompatible with that wiring.

See the [RFM guide](docs/RFM.md) for details.

## HR20 external connector

| ATmega169PV | Function / MCU pin |  |  |  |
| --- | --- | --- | --- | --- |
| Vcc | RXD (PE0/02) | TDO (PF6/55) | TMS (PF5/56) | /RST (PG5/20) |
| GND | TDI (PF7/54) | TXD (PE1/03) | TCK (PF4/57) | PE2/04 |

The connector can be used for programming/JTAG and wired UART communication.

## Build requirements

An AVR-compatible GCC toolchain and AVR libc are required. On Debian-based systems, typical packages are `gcc-avr`, `avr-libc`, `make` and `avrdude`.

The GitHub Actions reliability build uses a pinned AVR toolchain and checks the supported firmware matrix, static analysis, EEPROM layout, feature profile and universal HR20 memory limits.

## Project history

OpenHR20 was started around 2008 by Jiri Dobry and Dario Carluccio and has since been extended by many contributors.

The historical project remains available on [SourceForge](https://sourceforge.net/projects/openhr20/). A detailed German description of the original project is available at [mikrocontroller.net](https://www.mikrocontroller.net/articles/Heizungssteuerung_mit_Honeywell_HR20).

## License

The source files retain their original copyright and license notices. See `src/license.txt`.
