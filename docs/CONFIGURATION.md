# Configuration and Service Menu

OpenHR20 stores runtime configuration in EEPROM using the byte-oriented `config_t` structure in `src/eeprom.h`.

The local service menu, UART `G/S` commands and wireless configuration commands address these bytes by hexadecimal index.

**Important:** the index layout depends on the build. HR25 has an additional battery-level field, and RFM builds append radio parameters. Always use the table for the exact firmware target.

## Service-menu controls

Enter the service menu with **PROG + C + AUTO held together**.

- wheel in index mode: select a configuration index;
- **PROG**: switch from index selection to value editing;
- wheel in value mode: change the value within its allowed range;
- **PROG** while editing: save the value to EEPROM;
- **AUTO**: leave the service menu;
- **C**: switch to the watch/debug display.

Values and indexes are shown in hexadecimal on the thermostat display.

## Common configuration indexes

These indexes are common to the HR20 and HR25 builds through index `22`.

| Hex | Field | Default | Min | Max | Meaning / unit |
| --- | --- | ---: | ---: | ---: | --- |
| `00` | `lcd_contrast` | 14 | 0 | 15 | LCD contrast |
| `01` | `temperature0` | 10 | 10 | 60 | frost preset, 0.5 °C units; default 5.0 °C |
| `02` | `temperature1` | 34 | 10 | 60 | economy preset, 0.5 °C units; default 17.0 °C |
| `03` | `temperature2` | 42 | 10 | 60 | comfort preset, 0.5 °C units; default 21.0 °C |
| `04` | `temperature3` | 48 | 10 | 60 | super-comfort preset, 0.5 °C units; default 24.0 °C |
| `05` | `P3_Factor` | 33 | 0 | 255 | cubic proportional controller factor |
| `06` | `P_Factor` | 8 | 0 | 255 | proportional controller factor |
| `07` | `I_Factor` | 32 | 0 | 255 | integral controller factor |
| `08` | `I_max_credit` | 40 | 0 | 127 | integral-credit limit |
| `09` | `I_credit_expiration` | 30 | 0 | 255 | integral-credit expiry in PID intervals |
| `0a` | `PID_interval` | 48 | 4 | 255 | PID interval in 5-second units; default 240 s |
| `0b` | `valve_min` | 30 | 0 | 100 | minimum commanded valve position, % |
| `0c` | `valve_center` | 45 | 0 | 100 | nominal valve position around zero control error, % |
| `0d` | `valve_max` | 80 | 0 | 100 | maximum commanded valve position, % |
| `0e` | `valve_hysteresis` | 64 | 0 | 127 | movement hysteresis, 1/128 % units |
| `0f` | `motor_pwm_min` | 32 | 32 | 255 | minimum motor PWM |
| `10` | `motor_pwm_max` | 250 | 50 | 255 | maximum motor PWM |
| `11` | `motor_eye_low` | 100 | 1 | 255 | minimum accepted low photo-eye pulse length, scaled by 2 |
| `12` | `motor_eye_high` | 25 | 1 | 255 | minimum accepted high photo-eye pulse length, scaled by 2 |
| `13` | `motor_close_eye_timeout` | 78 | 5 | 255 | time from last pulse before eye shutdown, approx. 1/61 s units |
| `14` | `motor_end_detect_cal` | 130 | 110 | 250 | calibration end-stop threshold, % of previous average |
| `15` | `motor_end_detect_run` | 150 | 110 | 250 | normal-run end-stop threshold, % of previous average |
| `16` | `motor_speed` | 184 | 10 | 255 | motor-speed target/scaling parameter |
| `17` | `motor_speed_ctl_gain` | 50 | 10 | 200 | motor speed-control gain |
| `18` | `motor_pwm_max_step` | 10 | 1 | 64 | maximum PWM correction step |
| `19` | `MOTOR_ManuCalibration_L` | 255 | 0 | 255 | low byte of manual motor-travel calibration |
| `1a` | `MOTOR_ManuCalibration_H` | 255 | 0 | 255 | high byte of manual motor-travel calibration |
| `1b` | `temp_cal_table0` | 39 | 0 | 255 | 35 °C calibration-table base value |
| `1c` | `temp_cal_table1` | 45 | 16 | 255 | calibration-table delta 35→30 °C |
| `1d` | `temp_cal_table2` | 57 | 16 | 255 | calibration-table delta 30→25 °C |
| `1e` | `temp_cal_table3` | 75 | 16 | 255 | calibration-table delta 25→20 °C |
| `1f` | `temp_cal_table4` | 77 | 16 | 255 | calibration-table delta 20→15 °C |
| `20` | `temp_cal_table5` | 65 | 16 | 255 | calibration-table delta 15→10 °C |
| `21` | `temp_cal_table6` | 61 | 16 | 255 | calibration-table delta 10→5 °C |
| `22` | `timer_mode` | 0 | 0 | 123 | bit 0 selects one program vs weekday programs; values >1 also encode manual state/saved temperature |

The two manual-calibration bytes at `19/1a` form one 16-bit value. `0xffff` is the normal uncalibrated/default representation. The firmware validates restored calibration values before using them.

## HR20 universal: `HR20_universal_jd`

The HR20 universal build continues after `22` as follows:

| Hex | Field | Default | Min | Max | Meaning / unit |
| --- | --- | ---: | ---: | ---: | --- |
| `23` | `bat_warning_thld` | 120 | 80 | 160 | battery warning threshold, 0.02 V pack units / 0.01 V per cell |
| `24` | `bat_low_thld` | 100 | 80 | 160 | battery-low threshold |
| `25` | `allow_ADC_during_motor` | 1 | 0 | 1 | allow ADC conversion while motor is active |
| `26` | `window_detection_mode` | 1 | 0 | 2 | 0 off, 1 software, 2 PE2 hardware contact |
| `27` | `window_open_detection_diff` | 50 | 7 | 255 | software opening threshold, 0.01 °C |
| `28` | `window_close_detection_diff` | 50 | 7 | 255 | software closing threshold, 0.01 °C |
| `29` | `window_open_detection_time` | 8 | 1 | 32 | opening sample window, 15-second units |
| `2a` | `window_close_detection_time` | 8 | 1 | 32 | closing sample window, 15-second units |
| `2b` | `window_open_timeout` | 90 | 2 | 255 | maximum software-open time, minutes |
| `2c` | `hw_window_open_detection_delay` | 5 | 0 | 240 | PE2 opening delay, seconds |
| `2d` | `hw_window_close_detection_delay` | 5 | 0 | 240 | PE2 closing delay, seconds |
| `2e` | `RFM_devaddr` | 28 | 0 | 29 | radio device address; 0 disables radio networking |
| `2f` | `security_key[0]` | 0x01 | 0 | 255 | RFM security key byte 0 |
| `30` | `security_key[1]` | 0x23 | 0 | 255 | RFM security key byte 1 |
| `31` | `security_key[2]` | 0x45 | 0 | 255 | RFM security key byte 2 |
| `32` | `security_key[3]` | 0x67 | 0 | 255 | RFM security key byte 3 |
| `33` | `security_key[4]` | 0x89 | 0 | 255 | RFM security key byte 4 |
| `34` | `security_key[5]` | 0x01 | 0 | 255 | RFM security key byte 5 |
| `35` | `security_key[6]` | 0x23 | 0 | 255 | RFM security key byte 6 |
| `36` | `security_key[7]` | 0x45 | 0 | 255 | RFM security key byte 7 |
| `37` | `RFM_freqAdjust` | 0 | raw byte | raw byte | signed two's-complement RFM frequency correction |
| `38` | `RFM_tuning` | 0 | 0 | 1 | 0 narrow/high-rate mode, 1 wide/low-rate tuning mode |

## HR25 universal: `HR25_universal_tk`

HR25 has an extra half-battery threshold at index `23`. All later fields are therefore shifted by one byte compared with HR20.

| Hex | Field | Default | Min | Max | Meaning / unit |
| --- | --- | ---: | ---: | ---: | --- |
| `23` | `bat_half_thld` | 125 | 80 | 160 | threshold for half-battery LCD indication |
| `24` | `bat_warning_thld` | 120 | 80 | 160 | battery warning threshold |
| `25` | `bat_low_thld` | 100 | 80 | 160 | battery-low threshold |
| `26` | `allow_ADC_during_motor` | 1 | 0 | 1 | allow ADC conversion while motor is active |
| `27` | `window_detection_mode` | 1 | 0 | 2 | 0 off, 1 software, 2 PE2 hardware contact |
| `28` | `window_open_detection_diff` | 50 | 7 | 255 | software opening threshold, 0.01 °C |
| `29` | `window_close_detection_diff` | 50 | 7 | 255 | software closing threshold, 0.01 °C |
| `2a` | `window_open_detection_time` | 8 | 1 | 32 | opening sample window, 15-second units |
| `2b` | `window_close_detection_time` | 8 | 1 | 32 | closing sample window, 15-second units |
| `2c` | `window_open_timeout` | 90 | 2 | 255 | maximum software-open time, minutes |
| `2d` | `hw_window_open_detection_delay` | 5 | 0 | 240 | PE2 opening delay, seconds |
| `2e` | `hw_window_close_detection_delay` | 5 | 0 | 240 | PE2 closing delay, seconds |
| `2f` | `RFM_devaddr` | 28 | 0 | 29 | radio device address; 0 disables radio networking |
| `30` | `security_key[0]` | 0x01 | 0 | 255 | RFM security key byte 0 |
| `31` | `security_key[1]` | 0x23 | 0 | 255 | RFM security key byte 1 |
| `32` | `security_key[2]` | 0x45 | 0 | 255 | RFM security key byte 2 |
| `33` | `security_key[3]` | 0x67 | 0 | 255 | RFM security key byte 3 |
| `34` | `security_key[4]` | 0x89 | 0 | 255 | RFM security key byte 4 |
| `35` | `security_key[5]` | 0x01 | 0 | 255 | RFM security key byte 5 |
| `36` | `security_key[6]` | 0x23 | 0 | 255 | RFM security key byte 6 |
| `37` | `security_key[7]` | 0x45 | 0 | 255 | RFM security key byte 7 |
| `38` | `RFM_freqAdjust` | 0 | raw byte | raw byte | signed two's-complement RFM frequency correction |
| `39` | `RFM_tuning` | 0 | 0 | 1 | 0 narrow/high-rate mode, 1 wide/low-rate tuning mode |

## Other build variants

Standalone builds without RFM end before the radio fields.

Older compile-time-only window-detection targets have a different set of window fields. For those builds, do not assume the universal-build indexes above; refer to the exact `config_t` / `ee_config` layout in `src/eeprom.h`.

Optional compile-time features such as `BOOST_CONTROLER_AFTER_CHANGE` and `TEMP_COMPENSATE_OPTION` also append fields and therefore change later indexes.

## EEPROM layout and migration

The runtime-window layout uses EEPROM layout version `0x16`. Legacy window layouts are migrated before `config_raw` is loaded.

The firmware validates values against the min/max table in `src/eeprom.h`.

Motor calibration, date handling and communication parsing include additional reliability checks in this fork.

## Safety notes

Changing PID, motor, calibration or battery parameters can materially alter thermostat behaviour. Record the original value before experimenting.

The RFM security key shown above is the repository default. For installations that rely on radio security, use installation-specific key material.

The authoritative definitions remain `config_t` and `ee_config` in `src/eeprom.h`.
