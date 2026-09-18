# UART Interface

OpenHR20 provides a local ASCII UART command interface on non-RFM HR20 builds.

## Electrical interface

The HR20 external connector exposes:

- RXD: PE0
- TXD: PE1
- GND
- Vcc

The firmware baud rate is **9600 baud**.

RFM builds intentionally do not compile the local UART command parser. Their corresponding control operations are available through the wireless protocol instead.

## Command format

Commands are line-oriented and terminate with a newline.

- command letters are uppercase;
- hexadecimal arguments use lowercase hexadecimal digits;
- carriage return is normalized to newline by the receive path.

## Commands

| Command | Meaning |
| --- | --- |
| `V\n` | Print firmware version |
| `D\n` | Print status/debug line |
| `Taa\n` | Read watch/debug variable `aa` |
| `Gaa\n` | Read configuration byte `aa`; `ff` returns EEPROM layout version |
| `Saadd\n` | Write configuration byte `aa` with value `dd` |
| `Rab\n` | Read timer for day `a`, slot `b` |
| `Wabcddd\n` | Write timer: day `a`, slot `b`, mode `c`, time `ddd` |
| `Yyymmdd\n` | Set date; all fields are hexadecimal |
| `Hhhmmss\n` | Set time; all fields are hexadecimal |
| `Axx\n` | Set requested temperature, units 0.5 °C |
| `Mxx\n` | Set mode / close window state |
| `Lxx\n` | Key lock control/query |
| `B1324\n` | Reboot |

### Mode command

`Mxx` uses:

- `00`: manual
- `01`: automatic
- `fd`: leave operating mode unchanged and close window state

### Lock command

`Lxx` uses:

- `00`: unlock
- `01`: lock
- `02`: query current lock state

## Configuration access

`Gaa` and `Saa..` address the raw configuration layout. See [CONFIGURATION.md](CONFIGURATION.md) before modifying values.

The parser rejects configuration indexes outside the current configuration structure.

## Wireless equivalent

The RFM command parser implements corresponding operations for version/status, watch/config access, timer access, reboot, mode, requested temperature and lock state.

Wireless arguments are binary bytes, not ASCII hexadecimal strings. See [RFM.md](RFM.md).

The implementation in `src/com.c` remains the authoritative protocol reference.
