#!/usr/bin/env python3
from pathlib import Path
import subprocess
import sys

def run(*args, check=True):
    return subprocess.run(args, text=True, capture_output=True, check=check)


def read_text(path):
    p = Path(path)
    raw = p.read_bytes()
    eol = "\r\n" if b"\r\n" in raw else "\n"
    text = raw.decode("utf-8").replace("\r\n", "\n")
    return p, text, eol


def write_text(p, text, eol):
    if eol == "\r\n":
        text = text.replace("\n", "\r\n")
    p.write_bytes(text.encode("utf-8"))


def replace_once(path, old, new):
    p, text, eol = read_text(path)
    count = text.count(old)
    if count != 1:
        raise RuntimeError(f"{path}: expected exactly one match, found {count}\n--- expected ---\n{old}")
    text = text.replace(old, new, 1)
    write_text(p, text, eol)
    print(f"updated {path}")


def create_file(path, content, executable=False):
    p = Path(path)
    if p.exists():
        raise RuntimeError(f"refusing to overwrite existing new file: {path}")
    p.parent.mkdir(parents=True, exist_ok=True)
    p.write_text(content, encoding="utf-8", newline="\n")
    if executable:
        p.chmod(0o755)
    print(f"created {path}")


def verify_repo():
    if not Path(".git").exists():
        raise RuntimeError("run this script from the repository root")
    origin = run("git", "config", "--get", "remote.origin.url", check=False).stdout.strip()
    if "joker-mik/OpenHR20" not in origin:
        raise RuntimeError(f"unexpected origin: {origin!r}")


def already_applied():
    try:
        text = Path("src/config.h").read_text(encoding="utf-8")
    except UnicodeDecodeError:
        text = Path("src/config.h").read_bytes().decode("utf-8")
    return "#define REVLOW   2" in text and Path("RELIABILITY.md").exists()


def main():
    verify_repo()
    if already_applied():
        print("Reliability changes already applied; nothing to do.")
        return

    replace_once("Makefile",
        "default: HR20_rfm_int_sww rfm_master\n",
        "# Safe default for this fork: plain Honeywell HR20 without radio.\n"
        "# Radio builds remain available through their explicit targets and `make all`.\n"
        "default: HR20_original_sww\n")

    for target_context in (
        "\t\tHW_WINDOW_DETECTION=0 \\\n\t\tREV=-DREVISION=\\\\\\\"$(REV)\\\\\\\"\n",
        "\t\tHW_WINDOW_DETECTION=1 \\\n\t\tREV=-DREVISION=\\\\\\\"$(REV)\\\\\\\"\n",
    ):
        replace_once("Makefile", target_context,
                     target_context.replace("\t\tREV=", "\t\tRFM=1 \\\n\t\tREV="))

    replace_once("Makefile",
        "\t\tHW_WINDOW_DETECTION=0\\\n\t\tRFM_WIRE=MARIOJTAG \\\n",
        "\t\tHW_WINDOW_DETECTION=0\\\n\t\tRFM=1 \\\n\t\tRFM_WIRE=MARIOJTAG \\\n")
    replace_once("Makefile",
        "\t\tRFM_WIRE=TK_INTERNAL \\\n\t\tHW=HR25 \\\n",
        "\t\tRFM_WIRE=TK_INTERNAL \\\n\t\tRFM=1 \\\n\t\tHW=HR25 \\\n")

    replace_once("src/Makefile",
        "ifeq ($(HW),THERMOTRONIC)\n RFM?=0\nelse\n RFM?=1\nendif\n",
        "# This fork defaults to a standalone thermostat. Radio targets in the top-level\n"
        "# Makefile pass RFM=1 explicitly.\nRFM?=0\n")
    replace_once("src/Makefile",
        "BOOST_CONTROLER_AFTER_CHANGE?=0\n",
        "BOOST_CONTROLER_AFTER_CHANGE?=0\nMOTOR_AUTO_RESYNC?=1\n")
    replace_once("src/Makefile",
        "CFLAGS += -DBOOST_CONTROLER_AFTER_CHANGE=$(BOOST_CONTROLER_AFTER_CHANGE)\n",
        "CFLAGS += -DBOOST_CONTROLER_AFTER_CHANGE=$(BOOST_CONTROLER_AFTER_CHANGE)\n"
        "CFLAGS += -DMOTOR_AUTO_RESYNC=$(MOTOR_AUTO_RESYNC)\n")
    replace_once("src/Makefile",
        "AVRDUDE_FLAGS = -p $(MCU) -P $(AVRDUDE_PORT) -c $(AVRDUDE_PROGRAMMER) $(AVRDUDE_DELAY)\n",
        "# avr-gcc and avrdude use different part naming conventions.\n"
        "MCU_TO_PART=$(patsubst atmega%,m%,$(patsubst attiny%,t%,$1))\n"
        "AVRDUDE_PART = $(call MCU_TO_PART,$(MCU))\n\n"
        "AVRDUDE_FLAGS = -p $(AVRDUDE_PART) -P $(AVRDUDE_PORT) -c $(AVRDUDE_PROGRAMMER) $(AVRDUDE_DELAY)\n")
    replace_once("src/Makefile",
        "\t@echo \"BOOST_CONTROLER_AFTER_CHANGE=$(BOOST_CONTROLER_AFTER_CHANGE)\" >> $@\n",
        "\t@echo \"BOOST_CONTROLER_AFTER_CHANGE=$(BOOST_CONTROLER_AFTER_CHANGE)\" >> $@\n"
        "\t@echo \"MOTOR_AUTO_RESYNC=$(MOTOR_AUTO_RESYNC)\" >> $@\n")

    replace_once("src/config.h", "#define REVLOW   1", "#define REVLOW   2")
    replace_once("src/config.h",
        "/* we support UART */\n//#define COM_UART 1\n",
        "/* we support UART */\n#if !RFM\n#define COM_UART 1\n#endif\n")

    replace_once("src/adc.h",
        "bool task_ADC(void);\nvoid start_task_ADC(void);\n",
        "bool task_ADC(void);\nvoid start_task_ADC(void);\nbool ADC_BatteryReady(void);\n")
    replace_once("src/adc.c",
        "static int32_t ring_sum [2] = { 0, 0 };\nint16_t ring_average [2] = { 0, 0 };\n",
        "static int32_t ring_sum [2] = { 0, 0 };\nint16_t ring_average [2] = { 0, 0 };\n\n"
        "#define BATTERY_READY_SAMPLES 4\nstatic uint8_t battery_valid_samples = 0;\n")
    replace_once("src/adc.c",
        "static void shift_ring(void)\n",
        "bool ADC_BatteryReady(void)\n{\n\treturn battery_valid_samples >= BATTERY_READY_SAMPLES;\n}\n\n"
        "static void shift_ring(void)\n")
    replace_once("src/adc.c",
        "\t\tupdate_ring(BAT_RING_TYPE, ADC_Get_Bat_Voltage(ad));\n\n\t\t// activate voltage divider\n",
        "\t\tupdate_ring(BAT_RING_TYPE, ADC_Get_Bat_Voltage(ad));\n"
        "\t\tif (battery_valid_samples < BATTERY_READY_SAMPLES)\n\t\t{\n"
        "\t\t\tbattery_valid_samples++;\n\t\t}\n\n\t\t// activate voltage divider\n")

    replace_once("src/motor.h",
        "uint8_t MOTOR_GetPosPercent(void);                              // get percental position of motor (0-100%)\n",
        "uint8_t MOTOR_GetPosPercent(void);                              // get percental position of motor (0-100%)\n"
        "bool MOTOR_StartCloseReference(void);                           // re-reference physical closed end stop\n"
        "bool MOTOR_CloseReferenceActive(void);\n")
    replace_once("src/motor.h",
        "extern volatile int16_t MOTOR_PosAct;\n",
        "extern volatile int16_t MOTOR_PosAct;\nextern int16_t MOTOR_PosMax;\n")
    replace_once("src/motor.h",
        "extern volatile uint8_t MOTOR_PosOvershoot;\n",
        "extern volatile uint8_t MOTOR_PosOvershoot;\n"
        "extern uint8_t MOTOR_close_reference_count;\n"
        "extern int16_t MOTOR_last_close_reference_delta;\n")
    replace_once("src/motor.c",
        "static volatile uint16_t longest_low_eye = 0;\n\n\nstatic void MOTOR_Control(motor_dir_t);",
        "static volatile uint16_t longest_low_eye = 0;\n\n"
        "static bool MOTOR_close_reference_active = false;\n"
        "uint8_t MOTOR_close_reference_count = 0;\n"
        "int16_t MOTOR_last_close_reference_delta = 0;\n\n\n"
        "static void MOTOR_Control(motor_dir_t);")
    replace_once("src/motor.c",
        "\t\tMOTOR_calibration_step = -2;    // not calibrated\n\t\tMOTOR_wait_for_new_calibration = 5;\n",
        "\t\tMOTOR_calibration_step = -2;    // not calibrated\n\t\tMOTOR_close_reference_active = false;\n\t\tMOTOR_wait_for_new_calibration = 5;\n")
    replace_once("src/motor.c",
        "}\n\n/*!\n *******************************************************************************\n * Set PWM for motor with range check\n",
        "}\n\n"
        "/* Re-reference the physical closed end stop while preserving full travel. */\n"
        "bool MOTOR_StartCloseReference(void)\n{\n"
        "\tif (!MOTOR_IsCalibrated() || MOTOR_run_test() || MOTOR_eye_test())\n\t{\n\t\treturn false;\n\t}\n\n"
        "\tMOTOR_close_reference_active = true;\n"
        "\tMOTOR_PosStop = MOTOR_PosAct - MOTOR_MAX_IMPULSES;\n"
        "\tMOTOR_Control(close);\n\treturn true;\n}\n\n"
        "bool MOTOR_CloseReferenceActive(void)\n{\n\treturn MOTOR_close_reference_active;\n}\n\n"
        "/*!\n *******************************************************************************\n * Set PWM for motor with range check\n")
    replace_once("src/motor.c",
        "\tif (motor_timer > 0)                            // normal stop on wanted position\n\t{\n\t\tif (MOTOR_calibration_step != 0)\n",
        "\tif (motor_timer > 0)                            // normal stop on wanted position\n\t{\n"
        "\t\tif (MOTOR_close_reference_active)\n\t\t{\n"
        "\t\t\tMOTOR_close_reference_active = false;\n"
        "\t\t\tCTL_set_error(CTL_ERR_MOTOR);\n\t\t\treturn;\n\t\t}\n"
        "\t\tif (MOTOR_calibration_step != 0)\n")
    replace_once("src/motor.c",
        "\t\telse if (d == close)     // stopped on end\n\t\t{\n\t\t\t{\n\t\t\t\tif (MOTOR_calibration_step == 3)\n",
        "\t\telse if (d == close)     // stopped on end\n\t\t{\n"
        "\t\t\tif (MOTOR_close_reference_active)\n\t\t\t{\n"
        "\t\t\t\tMOTOR_close_reference_active = false;\n"
        "\t\t\t\t/* Do not turn a jam far from zero into a false reference. */\n"
        "\t\t\t\tif (MOTOR_PosAct <= MOTOR_MIN_IMPULSES)\n\t\t\t\t{\n"
        "\t\t\t\t\tMOTOR_last_close_reference_delta = MOTOR_PosAct;\n"
        "\t\t\t\t\tMOTOR_PosAct = 0;\n"
        "\t\t\t\t\tMOTOR_close_reference_count++;\n\t\t\t\t}\n"
        "\t\t\t\telse\n\t\t\t\t{\n\t\t\t\t\tCTL_set_error(CTL_ERR_MOTOR);\n\t\t\t\t}\n"
        "\t\t\t}\n\t\t\telse\n\t\t\t{\n"
        "\t\t\t\tif (MOTOR_calibration_step == 3)\n")
    replace_once("src/motor.c",
        "\t\tMOTOR_H_BRIDGE_stop();          // ensurance that motor is stop\n\t}\n\telse\n",
        "\t\tMOTOR_H_BRIDGE_stop();          // ensurance that motor is stop\n"
        "\t\tif (MOTOR_close_reference_active)\n\t\t{\n"
        "\t\t\tmotor_timer = 0;\n\t\t\ttask |= TASK_MOTOR_STOP;\n\t\t}\n"
        "\t}\n\telse\n")

    marker = "/*!\n *******************************************************************************\n * main program\n"
    resync = r'''#if MOTOR_AUTO_RESYNC
#define MOTOR_RESYNC_OVER_TEMP        100        /* 1.00 C, temp unit is 0.01 C */
#define MOTOR_RESYNC_MINUTES          15
#define MOTOR_RESYNC_COOLDOWN_MINUTES (12 * 60)

static uint16_t motor_resync_cooldown = 0;
static uint8_t motor_resync_minutes = 0;

static void motor_resync_update(void)
{
	if (motor_resync_cooldown > 0)
	{
		motor_resync_cooldown--;
	}

	if ((motor_resync_cooldown != 0)
	    || MOTOR_CloseReferenceActive()
	    || !MOTOR_IsCalibrated()
	    || (MOTOR_Dir != stop)
	    || mode_window()
	    || (CTL_error & (CTL_ERR_MOTOR | CTL_ERR_MONTAGE | CTL_ERR_BATT_WARNING | CTL_ERR_BATT_LOW))
	    || (CTL_temp_wanted < TEMP_MIN)
	    || (CTL_temp_wanted > TEMP_MAX)
	    || (valve_wanted > config.valve_min)
	    || (temp_average < ((int16_t)CTL_temp_wanted * 50 + MOTOR_RESYNC_OVER_TEMP)))
	{
		motor_resync_minutes = 0;
		return;
	}

	if (++motor_resync_minutes >= MOTOR_RESYNC_MINUTES)
	{
		if (MOTOR_StartCloseReference())
		{
			motor_resync_cooldown = MOTOR_RESYNC_COOLDOWN_MINUTES;
		}
		motor_resync_minutes = 0;
	}
}
#endif

'''
    replace_once("src/main.c", marker, resync + marker)
    replace_once("src/main.c",
        "\t// We should do the following once here to have valid data from the start\n\n\n",
        "\t// Start ADC immediately; motor movement waits for qualified battery data.\n\tstart_task_ADC();\n\n")
    replace_once("src/main.c",
        "#if RFM\n\t\t\t\t\twirelesTimeSyncCheck();\n#endif\n\t\t\t\t}\n",
        "#if RFM\n\t\t\t\t\twirelesTimeSyncCheck();\n#endif\n"
        "#if MOTOR_AUTO_RESYNC\n\t\t\t\t\tmotor_resync_update();\n#endif\n\t\t\t\t}\n")
    replace_once("src/main.c",
        "\t\t\t\tif (bat_average > 0)\n",
        "\t\t\t\tif (ADC_BatteryReady())\n")

    replace_once("src/watch.h",
        "#define WATCH_N (11)\n",
        "#if DEBUG_MOTOR_COUNTER\n#define WATCH_N (13)\n#else\n#define WATCH_N (11)\n#endif\n")
    replace_once("src/watch.c", "int16_t MOTOR_PosMax;\n\n\n", "")
    replace_once("src/watch.c",
        "#if DEBUG_MOTOR_COUNTER\n#define WATCH_LAYOUT 0x85\n#else\n#define WATCH_LAYOUT 0x05\n#endif\n",
        "#if DEBUG_MOTOR_COUNTER\n#define WATCH_LAYOUT 0x86\n#else\n#define WATCH_LAYOUT 0x06\n#endif\n")
    replace_once("src/watch.c",
        "\t/* 08 */ ((uint16_t)&MOTOR_PosOvershoot) + B8,\n#if DEBUG_MOTOR_COUNTER\n"
        "\t/* 09 */ ((uint16_t)&MOTOR_counter) + B16,\n\t/* 0a */ ((uint16_t)&MOTOR_counter) + 2 + B16,\n#endif\n",
        "\t/* 08 */ ((uint16_t)&MOTOR_PosOvershoot) + B8,\n"
        "\t/* 09 */ ((uint16_t)&MOTOR_close_reference_count) + B8,\n"
        "\t/* 0a */ ((uint16_t)&MOTOR_last_close_reference_delta) + B16,\n"
        "#if DEBUG_MOTOR_COUNTER\n"
        "\t/* 0b */ ((uint16_t)&MOTOR_counter) + B16,\n\t/* 0c */ ((uint16_t)&MOTOR_counter) + 2 + B16,\n#endif\n")

    replace_once("src/com.c", '#include "menu.h"\n', '#include "menu.h"\n#include "motor.h"\n')
    replace_once("src/com.c",
        "\tprint_s_p(PSTR(\" V: \"));\n\tprint_decXX(valve_wanted);\n",
        "\tprint_s_p(PSTR(\" V: \"));\n\tprint_decXX(valve_wanted);\n"
        "\tprint_s_p(PSTR(\" P: \"));\n\tprint_decXX(MOTOR_GetPosPercent());\n"
        "\tprint_s_p(PSTR(\" R: \"));\n\tprint_decXX(MOTOR_close_reference_count);\n")

    create_file("tools/check_eeprom_layout.sh", r'''#!/usr/bin/env bash
set -euo pipefail

elf=${1:?usage: check_eeprom_layout.sh path/to/hr20.elf}
nm_bin=${AVR_NM:-avr-nm}

symbol_addr() {
    local symbol=$1
    local raw
    raw=$($nm_bin -n "$elf" | awk -v symbol="$symbol" '$3 == symbol {print $1; exit}')
    if [[ -z "$raw" ]]; then
        echo "missing EEPROM symbol: $symbol" >&2
        return 1
    fi
    printf '%04x' $(( (16#$raw) & 0xffff ))
}

check_symbol() {
    local symbol=$1 expected=$2 actual
    actual=$(symbol_addr "$symbol")
    if [[ "$actual" != "$expected" ]]; then
        echo "EEPROM layout mismatch: $symbol expected 0x$expected, got 0x$actual" >&2
        return 1
    fi
    echo "EEPROM $symbol = 0x$actual"
}

check_symbol ee_layout       0003
check_symbol ee_timers       0004
check_symbol ee_reserved2_60 0084
check_symbol ee_config       00c0

echo "EEPROM layout OK"
''', executable=True)

    create_file(".github/workflows/reliability-build.yml", r'''name: Reliability build

on:
  push:
  pull_request:

permissions:
  contents: read

jobs:
  standalone-hr20:
    runs-on: ubuntu-22.04
    steps:
      - uses: actions/checkout@v4
      - name: Install AVR toolchain
        run: |
          sudo apt-get update -qq
          sudo apt-get install -y -qq gcc-avr binutils-avr avr-libc cppcheck
          avr-gcc --version
          avr-nm --version
      - name: Build standalone HR20
        run: make HR20_original_sww REV=${GITHUB_SHA::8}
      - name: Static checks
        run: make check
      - name: Verify EEPROM layout
        run: bash tools/check_eeprom_layout.sh bin/HR20_original_sww/hr20.elf
      - name: Show build metadata
        run: cat bin/HR20_original_sww/hr20.txt
      - name: Upload firmware artifacts
        uses: actions/upload-artifact@v4
        with:
          name: HR20-standalone-${{ github.sha }}
          path: |
            bin/HR20_original_sww/hr20.hex
            bin/HR20_original_sww/hr20.eep
            bin/HR20_original_sww/hr20.elf
            bin/HR20_original_sww/hr20.map
            bin/HR20_original_sww/hr20.txt
''')

    create_file("RELIABILITY.md", r'''# Standalone HR20 reliability profile

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
''')

    subprocess.run(["git", "diff", "--check"], check=True)
    print("\nChanges prepared successfully.")
    print("Review: git diff")
    print("Build with: make HR20_original_sww")


if __name__ == "__main__":
    try:
        main()
    except Exception as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        sys.exit(1)
