# Standalone HR20 reliability profile

This fork carries a conservative reliability profile for a plain Honeywell HR20 without RFM radio.

## Defaults

- plain HR20 / `RFM=0` is the default build;
- UART is enabled automatically for non-RFM HR20 builds;
- the existing PID/controller and software window detection algorithms are unchanged;
- the existing AUTO display behaviour and battery status page are retained.

## Battery startup qualification

The first motor movement is delayed until four accepted battery ADC samples are available. The existing ADC noise filter still decides whether an individual conversion is accepted.

## Automatic close re-reference

`MOTOR_AUTO_RESYNC=1` is enabled by default. A close re-reference is requested only after all of these conditions have remained true for 15 minutes:

- the motor is calibrated and stopped;
- no motor, mounting, battery-warning or battery-low error is active;
- window-open mode is inactive;
- the requested valve position is at `valve_min` or below;
- room temperature is at least 1.00 C above the requested temperature.

The motor drives toward the physical closed end stop using the existing pulse/end-stop timeout. The calibrated full travel is retained and only the zero reference is corrected. A 12-hour cooldown prevents frequent homing.

A stop far away from the expected zero region is treated as a motor error rather than a successful reference, so a jammed valve cannot silently redefine zero.

Diagnostics are RAM-only and add no EEPROM wear:

- `T09`: successful automatic close references since boot;
- `T0a`: signed 16-bit position correction from the latest reference (two's-complement hex).

The normal `D` UART status line additionally prints actual motor percent (`P`) and re-reference count (`R`).

## EEPROM layout guard

OpenHR20 historically stores independent top-level objects in `.eeprom` and relies on their addresses. Compiler/linker changes can silently produce an incompatible EEPROM image.

This first reliability release intentionally does not migrate the EEPROM format. CI instead verifies the legacy addresses after every standalone build and fails if they move:

- `ee_layout`: 0x0003
- `ee_timers`: 0x0004
- `ee_reserved2_60`: 0x0084
- `ee_config`: 0x00c0

A future format migration should use one explicitly versioned EEPROM image structure plus a migration path.
