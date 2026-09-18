# RFM12B Support

OpenHR20 supports the legacy RFM12/RFM12B sub-GHz transceiver.

## Recommended HR20 wiring: JD_INTERNAL

The universal HR20 build uses **JD_INTERNAL**:

| RFM signal | AVR pin |
| --- | --- |
| SCK | PF1 |
| SDI | PF0 |
| nSEL | PA3 |
| SDO | PE6 / PCINT6 |
| nIRQ | not connected |

This wiring does not consume the JTAG pins and leaves PE2 available for a hardware window contact.

Build:

```sh
make HR20_universal_jd
```

The universal build compiles RFM support but detects the module at runtime. The same HEX can therefore run with or without a JD_INTERNAL RFM module fitted.

## MARIOJTAG

MARIOJTAG uses the external JTAG pins for the RFM connection:

| RFM signal | AVR pin |
| --- | --- |
| SCK | PF4 / TCK |
| SDI | PF6 / TDO |
| nSEL | PF5 / TMS |
| SDO | PE2 / PCINT2 |

Consequences:

- JTAG must be disabled;
- PE2 is consumed by RFM SDO;
- PE2 hardware window detection is therefore incompatible with MARIOJTAG.

Build:

```sh
make HR20_rfm_ext_sww
```

## HR25: TK_INTERNAL

HR25 radio builds use TK_INTERNAL:

- SCK: PF1
- SDI: PF7
- nSEL: PF0
- SDO: PE6 / PCINT6

Recommended universal build:

```sh
make HR25_universal_tk
```

The universal HR25 firmware uses runtime RFM detection, keeps local controls active, retains RFM tuning and supports runtime software/PE2 window detection. Because TK_INTERNAL reuses PF7/TDI, JTAG is disabled for this target.

The fixed radio compatibility build remains available as:

```sh
make HR25_rfm_int_sww
```


## Runtime mode: OFF / ON / AUTO

The universal HR20 and HR25 builds expose `RFM_mode` in the service/configuration interface:

- `0 = OFF`: radio hardware remains disabled;
- `1 = ON`: radio support is forced on without probing;
- `2 = AUTO`: the module is probed at startup and enabled only when detected.

AUTO is the default and is recommended for universal firmware.

When AUTO is selected, startup briefly reports the result on the LCD:

- `rFon` — module found;
- `rF--` — no module found.

A changed runtime mode becomes active on the next reboot. `RFM_devaddr=0` remains a separate setting that disables radio network participation; it is not used as the hardware on/off switch.

## Frequency and tuning

The default main radio band is configured in the top-level Makefile. The firmware also supports EEPROM-based frequency adjustment through `RFM_freqAdjust` and tuning mode through `RFM_tuning`.

The universal build keeps `RFM_TUNING=1`.

## Device address and security key

RFM builds store:

- device address;
- eight-byte security key;
- optional frequency adjustment/tuning parameters

in the configuration structure.

The repository contains default values for development/compatibility. Installations that rely on radio security should use their own key material.

## Runtime detection

`HR20_universal_jd` uses runtime RFM detection. If no valid module response is detected, the radio path remains disabled and the thermostat continues as a local standalone thermostat.

## Protocol

The radio protocol is the existing OpenHR20 binary protocol and is not Bluetooth/BLE.

Wireless commands correspond to many UART operations but transport command arguments as binary bytes. Packet authentication/encryption uses the existing XTEA/CMAC code.

See `common/rfm.c`, `common/wireless.c` and `src/com.c` for the protocol implementation.
