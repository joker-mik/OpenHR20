# RFM master / gateway

`rfm_master` is firmware for a **separate RFM12B gateway**, not firmware for an HR20 or HR25 thermostat.

Its role is:

```text
PC / home automation
        |
     UART/USB
        |
   RFM master board
   AVR + RFM12B
        )))
        ))) RFM12B radio
        )))
    HR20 / HR25
```

## Supported master hardware

The build system currently supports:

| Variant | MCU | Clock |
| --- | --- | ---: |
| default master board | ATmega32 | 10 MHz |
| JeeNode | ATmega328P | 16 MHz |
| Nanode | ATmega328P | 16 MHz |

The original OpenHR20 master-board design is archived in `doc/rfm/master_board/`.

Use the newest original revision as the primary historical reference:

- schematic: `doc/rfm/master_board/master_schematic_v06.png`
- PCB image: `doc/rfm/master_board/master_board_v06.png`
- Eagle schematic: `doc/rfm/master_board/master_06.sch`
- Eagle board: `doc/rfm/master_board/master_06.brd`
- board notes: `doc/rfm/master_board/read_me.txt`

Revision 0.5 is retained for historical comparison.

## Default ATmega32 radio wiring

The firmware configuration maps the RFM12B as follows:

| RFM12B signal | ATmega32 |
| --- | --- |
| SCK | PB7 |
| SDI / MOSI | PB5 |
| nSEL | PB4 |
| SDO / MISO | PB6 |
| IRQ | INT2 |

The original v0.6 board uses an FTDI interface. Its historical board notes warn that L1 must only be populated when USB/FTDI power is not used, otherwise the USB +5 V and FTDI 3.3 V supplies would be connected together. Consult the original schematic before assembling the board.

## Build

From the repository root:

```sh
make rfm_master
```

The result is written below `bin/rfm_master/`. That directory is build output only; current installation/build instructions are maintained here and in [BUILD.md](BUILD.md).

## Important distinction

The thermostat universal builds and `rfm_master` are independent firmware images:

- `HR20_universal_jd` -> HR20 thermostat
- `HR25_universal_tk` -> HR25 thermostat
- `rfm_master` -> separate gateway hardware

They cannot be combined into one HEX because they target different hardware.

For thermostat-side radio wiring, runtime OFF/ON/AUTO mode and protocol notes see [RFM.md](RFM.md).
