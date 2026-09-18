# Hardware archive

This directory contains the original OpenHR20 hardware material: schematics, PCB files, photographs, LCD documentation, fuse screenshots and measurements.

The files are historical engineering sources and are intentionally kept in their original locations below `doc/` so old references and binary design files are not broken. Current user/build instructions live in `docs/`.

## Useful hardware material

| Area | Location |
| --- | --- |
| HR20 schematic | `doc/schaltplan/schaltplan.png` |
| HR20 connector | `doc/steckerbelegung/steckerbelegung.jpg` |
| LCD segments/photos | `doc/lcd/` |
| Fuse/lock-bit screenshots | `doc/fuses/` |
| Logic-analyser captures | `doc/logicanalyser/` |
| Internal HR20 RFM12 hardware | `doc/rfm/internal_RFM12/` |
| External HR20 RFM12 adapter | `doc/rfm/external_RFM12/` |
| Internal HR25 RFM12 hardware | `doc/rfm/internal_RFM12_HR25/` |
| RFM master PCB/schematic | `doc/rfm/master_board/` |
| Wireless expansion board | `doc/rfm/expansion_board/` |

For current radio wiring and firmware behavior see [RFM.md](RFM.md). For the separate gateway hardware see [RFM_MASTER.md](RFM_MASTER.md).

## HR25 external connector

The HR25 schematic used by the project shows PE2 on the external 10-pin connector. This makes the firmware hardware-window input accessible without modifying the MCU wiring.

| Connector pin | Signal |
| --- | --- |
| 1 | /RESET |
| 2 | PE2 (hardware window input) |
| 3 | TMS |
| 4 | TCK |
| 5 | TDO |
| 6 | TDI / PF7 |
| 7 | RXD |
| 8 | TXD |
| 9 | +BAT |
| 10 | GND |

For `HR25_universal_tk`, TK_INTERNAL uses PF1 (RFM SCK), PF7/TDI (RFM SDI), PF0 (RFM nSEL), and PE6 (RFM SDO). PE2 therefore remains available for the external hardware-window contact. Because PF7/TDI is shared with the radio wiring, JTAG is disabled by firmware after startup for normal RFM operation; keep the JTAGEN fuse enabled if the external JTAG connector is to remain usable for firmware updates.

## Historical documents

The PDFs, ODT/DOC files, old README files and screenshots under `doc/` document the original project and remain useful as primary engineering material. They are **not** the current build or operating instructions.

In particular, old SourceForge/SVN links, old target names and compile switches in those files may no longer describe this fork. Start with the root README and `docs/` for current instructions.
