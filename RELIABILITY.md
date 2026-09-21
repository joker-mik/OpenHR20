# Thermostat reliability profile

This fork carries reliability improvements across the supported thermostat targets. A plain Honeywell HR20 without RFM remains the default build, but the runtime hardening is not limited to that configuration.

## Defaults

- plain HR20 / `RFM=0` is the default build, while CI also compiles the supported RFM, HR25 and Thermotronic targets;
- UART is enabled automatically for non-RFM HR20 builds;
- the existing PID/controller and software window detection algorithms are unchanged;
- the existing AUTO display behaviour and battery status page are retained.

## Battery startup qualification

The first motor movement is delayed until four accepted battery ADC samples are available. The existing ADC noise filter still decides whether an individual conversion is accepted.

## Motor calibration and battery use

The additional time-based close re-reference has been removed. It caused extra motor travel from the normal minimum valve position toward the mechanical zero and back, which is undesirable for battery life.

The existing weekly valve-protection calibration remains the only periodic automatic zero search. It runs on Saturday at 10:00 when no battery warning/low error is active.

Motor-calibration hardening remains in place: stored manual travel and completed calibration results must stay within the plausible impulse range. Implausible values are rejected with a motor error instead of being accepted as valid travel.
## EEPROM layout guard

OpenHR20 historically stores independent top-level objects in `.eeprom` and relies on their addresses. Compiler/linker changes can silently produce an incompatible EEPROM image.

Runtime window detection migrates legacy layouts `0x14`/`0x15` to `0x16`. CI independently verifies the physical EEPROM symbol addresses after standalone builds and fails if they move:

- `ee_layout`: 0x0003
- `ee_timers`: 0x0004
- `ee_reserved2_60`: 0x0084
- `ee_config`: 0x00c0

Migration stages the legacy window values first and commits the layout byte last; a valid staging header is also used to recover from an interrupted layout-byte write.

## Runtime-selectable window detection

Standalone Honeywell HR20 builds compile both software and PE2 hardware window detection. EEPROM index `0x26` selects `0=off`, `1=software`, `2=hardware`; software and hardware delays have separate EEPROM entries. Legacy window layouts `0x14` and `0x15` are migrated to the common layout `0x16` before normal configuration loading. The migration stages legacy window values in the reserved EEPROM area and writes the new layout marker last so it can be retried after an interrupted write.
