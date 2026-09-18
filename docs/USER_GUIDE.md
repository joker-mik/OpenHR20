# OpenHR20 User Guide

This guide covers local operation of the thermostat firmware. The exact display symbols differ slightly between HR20 and HR25, but the control logic is shared.

## Controls

The front panel uses three keys and a rotary wheel:

- **PROG**
- **C**
- **AUTO**
- **Rotary wheel**

Short presses generate immediate actions. Long presses are detected after the firmware's long-press threshold.

## Normal operation

| Control | Action |
| --- | --- |
| Wheel | Change the requested temperature |
| AUTO | Change operating mode |
| C | Cycle alternate home displays |
| Long AUTO | Enter date/time setting |
| Long PROG | Enter timer/switching-program setting |
| Long C | Enter preset-temperature setting |
| Long AUTO + C | Toggle key lock |
| Long PROG + AUTO | Manually toggle software-window state when software window detection is active |
| Long PROG + C + AUTO | Enter service menu |

The alternate home displays are:

1. requested temperature
2. measured room temperature
3. valve position
4. time
5. battery voltage, when enabled

In automatic mode, turning the wheel creates a temporary override until the next scheduled temperature event.

## Date and time

Hold **AUTO** to enter date/time setup.

The sequence is:

1. year
2. month
3. day
4. hour
5. minute

Use the wheel to change the current value and **PROG** to confirm and advance.

The RTC implementation uses Gregorian leap-year rules, including the 100-year and 400-year exceptions. The representable calendar range is 2000 through 2255.

## Timer programming

Hold **PROG** to enter timer programming.

Use the wheel to select the day/day-group and **PROG** to enter the timer slots. In a timer slot:

- wheel changes the time;
- **C** changes the associated preset temperature type;
- **PROG** stores the slot and advances;
- **AUTO** leaves the editor without storing the current edit.

## Preset temperatures

Hold **C** to edit the four preset temperatures.

Use the wheel to change the current preset and **PROG** to store it and advance to the next preset. **AUTO** leaves the editor without storing the current edit.

## Window detection

On builds with runtime window selection, configuration value `window_detection_mode` selects:

| Value | Mode |
| ---: | --- |
| 0 | disabled |
| 1 | software detection from temperature change |
| 2 | hardware contact on PE2 |

The default is software detection.

When software detection is active, holding **PROG + AUTO** toggles the window-open state manually. In hardware mode PE2 remains authoritative.

## Key lock

Hold **AUTO + C** to toggle the key lock.

On HR25 the lock state is shown with the lock icon. On HR20 the firmware uses the lock display message when appropriate.

## Service menu

Hold **PROG + C + AUTO** to enter the service menu.

The service menu works directly on configuration bytes:

- wheel in index mode: select configuration index;
- **PROG**: switch between index selection and value editing;
- wheel in value mode: change the value within the configured min/max range;
- **PROG** while editing: save the value to EEPROM;
- **AUTO**: leave the service menu;
- **C**: enter the watch/debug display.

The configuration layout and allowed ranges are documented in [CONFIGURATION.md](CONFIGURATION.md).

Use care in the service menu: motor, temperature, window and radio parameters are low-level firmware settings.
