# Build Guide

## Toolchain

OpenHR20 uses AVR-GCC and AVR libc. The reliability CI uses a pinned AVR-GCC 4.9.2-based toolchain so code-size checks remain reproducible.

Typical dependencies on Debian-based Linux systems include:

- gcc-avr
- avr-libc
- avrdude
- make

## Default build

```sh
make
```

The repository default is the plain HR20 standalone firmware.

## Build matrix

| Target | Hardware / function |
| --- | --- |
| `HR20_universal_jd` | HR20; optional JD_INTERNAL RFM; local controls; runtime software/PE2 window selection |
| `HR20_original_sww` | HR20 without RFM; runtime software/PE2 window selection |
| `HR20_original_hww` | compatibility target for standalone HR20 runtime window selection |
| `HR20_rfm_int_sww` | HR20 with JD_INTERNAL RFM; software window detection |
| `HR20_rfm_int_hww` | HR20 with JD_INTERNAL RFM; hardware window detection |
| `HR20_rfm_ext_sww` | HR20 with MARIOJTAG RFM; software window detection; JTAG disabled |
| `HR25_original_sww` | HR25 without RFM |
| `HR25_rfm_int_sww` | HR25 with TK_INTERNAL RFM |
| `thermotronic_sww` | Thermotronic without RFM |
| `rfm_master` | separate RFM master/gateway firmware |

Build all targets:

```sh
make all
```

## Universal HR20 target

`HR20_universal_jd` enables:

- `RFM=1`
- `RFM_WIRE=JD_INTERNAL`
- `RFM_TUNING=1`
- `RFM_RUNTIME_DETECT=1`
- `REMOTE_SETTING_ONLY=0`
- `WINDOW_DETECTION_RUNTIME=1`
- `GC_SECTIONS=1`
- `LTO=1`

This combination provides one HR20 image for JD_INTERNAL installations with or without the radio module.

## JTAG

JD_INTERNAL does not require JTAG to be disabled.

MARIOJTAG and TK_INTERNAL do require JTAG to be disabled because they reuse JTAG pins.

## Output

Build products are written below:

```text
bin/<target>/
```

Typical outputs include:

- `hr20.hex`
- `hr20.eep`
- `hr20.elf`
- `hr20.map`
- `hr20.txt`

The text file records the build configuration and AVR memory use.

## CI and memory limits

The GitHub Actions reliability build compiles the supported target matrix, runs static checks, checks EEPROM layout, verifies the universal feature profile and enforces the ATmega169P limits for the universal HR20:

- flash: 16 KiB
- SRAM: 1 KiB

A build that links successfully but exceeds those limits is considered a CI failure.
