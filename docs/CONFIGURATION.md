# Configuration and Service Menu

OpenHR20 stores runtime configuration in EEPROM using the `config_t` layout in `src/eeprom.h`.

The service menu and UART/RFM configuration commands operate on this byte-oriented structure.

## Access methods

Configuration can be changed through:

- the local service menu;
- UART `G` / `S` commands on non-RFM builds;
- corresponding wireless configuration commands on RFM builds.

The firmware validates values against the min/max table in `src/eeprom.h`.

## Important groups

The structure contains parameters for:

- temperature presets
- PID/controller behaviour
- valve/motor control
- motor calibration
- temperature calibration table
- timer mode
- battery thresholds
- ADC behaviour
- window detection
- optional boost/temperature compensation
- RFM address/security/tuning on RFM builds

## Runtime window detection

When `WINDOW_DETECTION_RUNTIME=1`, the configuration includes:

| Field | Meaning |
| --- | --- |
| `window_detection_mode` | 0 off, 1 software, 2 PE2 hardware |
| `window_open_detection_diff` | software opening temperature-change threshold |
| `window_close_detection_diff` | software closing threshold |
| `window_open_detection_time` | software opening sample window |
| `window_close_detection_time` | software closing sample window |
| `window_open_timeout` | maximum software-open time |
| `hw_window_open_detection_delay` | PE2 opening delay |
| `hw_window_close_detection_delay` | PE2 closing delay |

The default mode is software detection.

## Motor calibration

The manual motor calibration value is stored as two bytes. The reliability branch validates restored values against the configured motor travel range before using them.

Invalid or implausible stored travel values are rejected rather than silently driving against an unrealistic target.

## Battery thresholds

Battery warning and battery-low thresholds are configurable. Motor startup and movement include additional low-voltage protection in the reliability branch.

## RFM configuration

RFM builds add:

- `RFM_devaddr`
- eight-byte `security_key`
- `RFM_freqAdjust`
- `RFM_tuning`

when tuning support is enabled.

A device address of zero disables radio networking.

## EEPROM layout

The EEPROM layout has an explicit layout version and migration handling. CI checks the expected EEPROM layout for the main thermostat builds.

Because offsets depend on compile-time features and hardware type, use the configuration table generated for the exact firmware build rather than assuming an offset from another target.

The authoritative definitions, defaults, minimums and maximums are in `src/eeprom.h`.
