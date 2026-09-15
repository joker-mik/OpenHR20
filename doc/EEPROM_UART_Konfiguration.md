# EEPROM- und UART-Konfiguration des OpenHR20

Diese Notiz beschreibt, wie die laufende OpenHR20-Firmware ihre Konfigurationswerte im EEPROM verwaltet und wie sie ueber die serielle Schnittstelle gelesen bzw. geaendert werden koennen.

Die Tabelle unten gilt fuer den in diesem Fork bevorzugten **Honeywell HR20 ohne RFM (`RFM=0`) mit Software-Fenstererkennung (`HW_WINDOW_DETECTION=0`)**. Bei anderen Build-Varianten koennen sich insbesondere die Indizes **ab `0x23`** verschieben oder andere Bedeutungen haben.

> Wichtig: Die Befehle `Gxx` und `Sxxyy` arbeiten mit einem **Konfigurationsindex**, nicht mit einer direkten physischen EEPROM-Adresse.

## 1. Serielle Verbindung

Die Firmware benutzt bei einem Nicht-RFM-HR20 automatisch den UART.

- Baudrate: **9600 Baud**
- Format: **8N1**
- Befehlsende: **Newline** (`\n`; ein empfangenes `\r` wird intern in `\n` umgesetzt)
- Befehlsbuchstaben sind **gross**.
- Hex-Ziffern `a` bis `f` muessen **klein** geschrieben werden.
- Es gibt keine Leerzeichen innerhalb eines Befehls.

Am externen HR20-Stecker liegen die MCU-Signale direkt an:

| Signal | ATmega169 |
| --- | --- |
| RXD | PE0 |
| TXD | PE1 |
| GND | GND |
| Versorgung/Logikreferenz | Vcc |

Das ist ein MCU-UART und **kein klassischer RS-232-Pegel mit +/-12 V**. Einen USB-UART-Adapter deshalb nur mit zum HR20 passendem Logikpegel und gemeinsamem GND verwenden.

## 2. EEPROM-Konfiguration lesen und schreiben

### Lesen

```text
Gaa
```

`aa` ist der zweistellige hexadezimale Konfigurationsindex.

Beispiel:

```text
G0b
```

Bei Werkseinstellung kommt fuer `valve_min` zurueck:

```text
G[0b]=1e
```

`0x1e` = 30 dezimal.

### Schreiben

```text
Saadd
```

- `aa` = Konfigurationsindex in Hex
- `dd` = neuer 8-Bit-Wert in Hex

Beispiel: Komforttemperatur (`03`) auf 20.0 degC setzen. Temperaturen sind in 0.5-degC-Schritten gespeichert, also 20.0 / 0.5 = 40 = `0x28`:

```text
S0328
```

Antwort:

```text
S[03]=28
```

### Bereichspruefung

Jeder Eintrag besitzt im EEPROM vier Bytes:

1. aktueller Wert
2. Defaultwert
3. Minimalwert
4. Maximalwert

Beim Schreiben prueft die Firmware den erlaubten Bereich. Ein ungueltiger Wert wird **nicht auf Min/Max begrenzt**, sondern auf den **Defaultwert** des Parameters zurueckgesetzt und so gespeichert.

Darum nach jedem `S...` immer die Rueckgabe pruefen oder anschliessend noch einmal mit `G...` lesen.

### EEPROM-Layout-Version

```text
Gff
```

liefert die kompilierte EEPROM-Layout-Version.

Fuer die hier dokumentierte Software-Fenster-Variante ist sie:

```text
14
```

Bei Hardware-Fensterkontakt ist sie `15`.

## 3. Parameter-Tabelle fuer HR20, RFM=0, Software-Fenstererkennung

Alle Werte im seriellen Protokoll sind hexadezimal. Die Tabelle zeigt zusaetzlich den dezimalen bzw. physikalischen Defaultwert.

| Index | Name / Bedeutung | Default Hex | Default lesbar | Erlaubter Bereich / Einheit | Hinweis |
| --- | --- | ---: | ---: | --- | --- |
| `00` | `lcd_contrast` | `0e` | 14 | 0..15 | LCD-Kontrast |
| `01` | `temperature0` | `0a` | 5.0 degC | 5.0..30.0 degC, 0.5 degC/LSB | Frostschutz |
| `02` | `temperature1` | `22` | 17.0 degC | 5.0..30.0 degC, 0.5 degC/LSB | Energiespar-Temperatur |
| `03` | `temperature2` | `2a` | 21.0 degC | 5.0..30.0 degC, 0.5 degC/LSB | Komfort-Temperatur |
| `04` | `temperature3` | `30` | 24.0 degC | 5.0..30.0 degC, 0.5 degC/LSB | Superkomfort |
| `05` | `P3_Factor` | `21` | 33 | 0..255 | kubischer P-Anteil; Regler-Tuning |
| `06` | `P_Factor` | `08` | 8 | 0..255 | proportionaler Anteil; Regler-Tuning |
| `07` | `I_Factor` | `20` | 32 | 0..255 | Integralanteil; Regler-Tuning |
| `08` | `I_max_credit` | `28` | 40 | 0..127 | Begrenzung des Integrators |
| `09` | `I_credit_expiration` | `1e` | 30 | 0..255 PID-Intervalle | Default entspricht 30 x 240 s = 2 h |
| `0a` | `PID_interval` | `30` | 48 -> 240 s | Wert x 5 s; min. 20 s | Abstand zwischen regulaeren PID-Berechnungen |
| `0b` | `valve_min` | `1e` | 30 % | 0..100 % | minimale Regel-Stellposition; **nicht gleich 30 % Heizleistung** |
| `0c` | `valve_center` | `2d` | 45 % | 0..100 % | Start-/Mittelposition zur Stabilisierung |
| `0d` | `valve_max` | `50` | 80 % | 0..100 % | maximale Regel-Stellposition |
| `0e` | `valve_hysteresis` | `40` | 64 -> 0.5 % | 1/128 % pro LSB, max. 127 | verhindert unnoetige kleine Motorbewegungen |
| `0f` | `motor_pwm_min` | `20` | 32 | 32..255 | minimale Motor-PWM |
| `10` | `motor_pwm_max` | `fa` | 250 | 50..255 | maximale Motor-PWM |
| `11` | `motor_eye_low` | `64` | 100 | 1..255, interne Zeitbasis | Mindestlaenge LOW des Lichtschranken-Signals; intern x2 |
| `12` | `motor_eye_high` | `19` | 25 | 1..255, interne Zeitbasis | Mindestlaenge HIGH des Lichtschranken-Signals; intern x2 |
| `13` | `motor_close_eye_timeout` | `4e` | 78 -> ca. 1.28 s | 5..255, 1/61 s pro LSB | Zeit ohne Puls bis Lichtschranke beim Schliessen deaktiviert wird |
| `14` | `motor_end_detect_cal` | `82` | 130 % | 110..250 % | Endanschlag-Erkennung waehrend Kalibrierung relativ zur vorherigen Pulszeit |
| `15` | `motor_end_detect_run` | `96` | 150 % | 110..250 % | Endanschlag-Erkennung im normalen Lauf |
| `16` | `motor_speed` | `b8` | 184 | 10..255, interne Einheit (/8 im Code) | Sollwert der Motor-Pulszeitregelung |
| `17` | `motor_speed_ctl_gain` | `32` | 50 | 10..200 | Verstaerkung der Motor-Drehzahlregelung |
| `18` | `motor_pwm_max_step` | `0a` | 10 | 1..64 | max. PWM-Aenderung pro Motorpuls |
| `19` | `MOTOR_ManuCalibration_L` | `ff` | 255 | 0..255 | Low-Byte der gespeicherten manuellen Motorkalibrierung; normalerweise nicht direkt aendern |
| `1a` | `MOTOR_ManuCalibration_H` | `ff` | 255 | 0..255 | High-Byte der manuellen Motorkalibrierung; `ffff` bedeutet automatische Kalibrierung |
| `1b` | `temp_cal_table0` | `27` | 39 | 0..255 ADC-Schritte | erster Temperatur-Kalibrierpunkt; siehe Abschnitt 4 |
| `1c` | `temp_cal_table1` | `2d` | 45 | 16..255 ADC-Schritte | Delta zum naechsten Kalibrierpunkt |
| `1d` | `temp_cal_table2` | `39` | 57 | 16..255 ADC-Schritte | Delta |
| `1e` | `temp_cal_table3` | `4b` | 75 | 16..255 ADC-Schritte | Delta |
| `1f` | `temp_cal_table4` | `4d` | 77 | 16..255 ADC-Schritte | Delta |
| `20` | `temp_cal_table5` | `41` | 65 | 16..255 ADC-Schritte | Delta |
| `21` | `temp_cal_table6` | `3d` | 61 | 16..255 ADC-Schritte | Delta |
| `22` | `timer_mode` | `00` | 0 | 0..`7b` | Bit 0 waehlt Wochenprogramm-Modus; im manuellen Betrieb speichert die Firmware hier zusaetzlich die Solltemperatur |
| `23` | `bat_warning_thld` | `78` | 120 -> 2.40 V | 80..160, 0.02 V/LSB | Batterie-Warnschwelle fuer die Gesamtspannung |
| `24` | `bat_low_thld` | `64` | 100 -> 2.00 V | 80..160, 0.02 V/LSB | Batterie-Unterspannungsgrenze |
| `25` | `allow_ADC_during_motor` | `01` | an | 0/1 | Temperatur/Batterie-ADC auch waehrend Motorlauf erlauben |
| `26` | `window_open_detection_diff` | `32` | 50 -> 0.50 degC | 7..255, 0.01 degC/LSB | benoetigter Temperaturabfall fuer Fenster-auf-Erkennung |
| `27` | `window_close_detection_diff` | `32` | 50 -> 0.50 degC | 7..255, 0.01 degC/LSB | benoetigter Temperaturanstieg fuer Fenster-zu-Erkennung |
| `28` | `window_open_detection_time` | `08` | 8 -> 120 s | 1..32, 15 s/LSB | Betrachtungszeit fuer Fenster-auf |
| `29` | `window_close_detection_time` | `08` | 8 -> 120 s | 1..32, 15 s/LSB | Betrachtungszeit fuer Fenster-zu |
| `2a` | `window_open_timeout` | `5a` | 90 min | 2..255 min | maximale Dauer des Fenster-offen-Zustands |

## 4. Temperatur-Kalibriertabelle (`1b` bis `21`)

Diese Werte sollte man nur aendern, wenn die Temperaturmessung gezielt kalibriert werden soll.

Beim HR20 ist der feste ADC-Offset `256`. `1b` ist der Abstand zum ersten Punkt, danach folgen nur noch Differenzen:

```text
35 degC: 256 + 0x27 = 295
30 degC: 295 + 0x2d = 340
25 degC: 340 + 0x39 = 397
20 degC: 397 + 0x4b = 472
15 degC: 472 + 0x4d = 549
10 degC: 549 + 0x41 = 614
 5 degC: 614 + 0x3d = 675
```

Ein einzelner geaenderter Delta-Wert verschiebt daher auch alle darunterliegenden Punkte.

## 5. Batterie-Schwellen

Die Firmware vergleicht die gemessene Batteriespannung in Millivolt mit:

```text
Schwelle [mV] = EEPROM-Wert x 20
```

Beispiele:

```text
G23  -> 78 hex = 120 dezimal = 2400 mV
G24  -> 64 hex = 100 dezimal = 2000 mV
```

## 6. Fenstererkennung

Bei der Software-Fenstererkennung werden Temperaturmittelwerte betrachtet. Die Zeitparameter `28` und `29` arbeiten in 15-Sekunden-Schritten.

Default:

```text
Fenster auf:  Temperaturabfall > 0.50 degC innerhalb von 2 min
Fenster zu:   Temperaturanstieg > 0.50 degC innerhalb von 2 min
Timeout:      90 min
```

Bei `HW_WINDOW_DETECTION=1` ist die Belegung ab `26` anders:

| Index | Bedeutung | Default |
| --- | --- | ---: |
| `26` | `window_open_detection_enable` | 1 |
| `27` | `window_open_detection_delay` | 5 s |
| `28` | `window_close_detection_delay` | 5 s |

Diese Variante hat EEPROM-Layout-Version `0x15` statt `0x14`.

## 7. Schaltzeiten im EEPROM

Die Wochen-Schaltzeiten liegen getrennt von der Konfiguration im Array `ee_timers`. Es gibt acht Slots pro Tag.

### Timer lesen

```text
Rab
```

- `a` = Tag
- `b` = Slot 0..7

Tag-Code:

| Code | Tag |
| ---: | --- |
| `0` | komplette Woche / Fallback-Programm |
| `1` | Montag |
| `2` | Dienstag |
| `3` | Mittwoch |
| `4` | Donnerstag |
| `5` | Freitag |
| `6` | Samstag |
| `7` | Sonntag |

Die Antwort ist `cddd`:

- `c=0` -> Temperatur 0 / Frostschutz
- `c=1` -> Temperatur 1 / Energiesparen
- `c=2` -> Temperatur 2 / Komfort
- `c=3` -> Temperatur 3 / Superkomfort
- `ddd` -> Minuten seit Mitternacht, hexadezimal
- `ddd=fff` -> Slot deaktiviert

Beispiel:

```text
R10
```

liest Montag, Slot 0.

### Timer schreiben

```text
Wabcddd
```

Beispiel: Montag, Slot 0, Komfort (`2`) um 07:00 Uhr. 07:00 = 420 Minuten = `0x1a4`:

```text
W1021a4
```

Danach den Slot mit `R10` kontrollieren.

## 8. Weitere nuetzliche serielle Befehle

| Befehl | Funktion |
| --- | --- |
| `V` | Firmware-Version ausgeben |
| `D` | Statuszeile ausgeben |
| `Taa` | Diagnose-/Watch-Wert lesen |
| `Gaa` | Konfigurationswert lesen |
| `Saadd` | Konfigurationswert schreiben |
| `Rab` | Timer lesen |
| `Wabcddd` | Timer schreiben |
| `Axx` | Solltemperatur setzen, Einheit 0.5 degC |
| `M00` | manueller Modus |
| `M01` | Automatikmodus |
| `Mfd` | Modus unveraendert, Fensterzustand schliessen/zuruecksetzen |
| `L00` / `L01` / `L02` | Tasten entsperren / sperren / Status abfragen |
| `Hhhmmss` | Uhrzeit setzen; Werte sind hexadezimal |
| `Yyymmdd` | Datum setzen; Werte sind hexadezimal |
| `B1324` | Neustart ueber Watchdog |

## 9. Diagnosewerte `Txx` in diesem Reliability-Branch

`Txx` liest **RAM-Diagnosewerte**, nicht EEPROM.

| Index | Bedeutung |
| --- | --- |
| `00`/`01` | untere/obere 16 Bit des PID-Integrators `sumError` |
| `02` | Integrator-Credit |
| `03` | Credit-Expiration |
| `04` | Fenster-Modus/Timeout |
| `05` | Motor-Pulsdiagnose `motor_diag` |
| `06` | kalibrierter Motorweg `MOTOR_PosMax` |
| `07` | aktuelle interne Motorposition `MOTOR_PosAct` |
| `08` | erkannter Motor-Overshoot |
| `09` | Anzahl erfolgreicher automatischer Close-Re-References seit Boot |
| `0a` | Positionskorrektur der letzten Close-Re-Reference, signed 16 Bit |
| `0b`/`0c` | Motor-Pulszaehler, falls `DEBUG_MOTOR_COUNTER=1` |

Gerade fuer die Untersuchung von Positionsdrift sind `T06`, `T07`, `T09` und `T0a` interessant.

## 10. Alle Konfigurationswerte auf Default zuruecksetzen

Beim Einschalten prueft die Firmware die drei Tasten **PROG + C + AUTO**. Werden alle drei waehrend des Starts gedrueckt gehalten, werden die Konfigurationswerte aus den Default-Spalten des EEPROM-Schemas neu geladen und gespeichert.

Achtung: Das betrifft die Konfigurationswerte. Schaltzeiten liegen in einem eigenen EEPROM-Bereich.

## 11. Physisches EEPROM-Layout

Im von uns abgesicherten Standalone-Build prueft CI derzeit folgende Startadressen:

```text
0x0003  ee_layout
0x0004  ee_timers
0x0084  reservierter Bereich
0x00c0  ee_config
```

`ee_config` besteht aus vier Bytes pro Konfigurationsindex:

```text
Adresse = 0x00c0 + 4 * Index

+0  aktueller Wert
+1  Default
+2  Minimum
+3  Maximum
```

Beispiel `valve_min`, Index `0x0b`:

```text
0x00c0 + 4 * 0x0b = 0x00ec
```

Der aktuelle Wert von `valve_min` liegt in diesem Build damit physisch bei `0x00ec`.

**Direktes Bearbeiten des EEPROM-Images ist trotzdem nicht die bevorzugte Methode.** `G`/`S` benutzt dieselbe Firmware-Struktur, fuehrt die Bereichspruefung aus und ist wesentlich weniger fehleranfaellig.

## 12. Was sollte man besser nicht nebenbei veraendern?

Besonders vorsichtig mit:

- `05`..`09`: PID-Tuning
- `0f`..`18`: Motorregelung und Lichtschranken-/Endanschlag-Erkennung
- `19`/`1a`: gespeicherte Motorkalibrierung
- `1b`..`21`: Temperatur-ADC-Kalibrierung
- `23`/`24`: Batterie-Schutzschwellen

Fuer normale Experimente sind die Temperatur-Sollwerte, Ventilgrenzen und Fensterparameter leichter nachvollziehbar. Vor Aenderungen am besten den alten Wert notieren und immer nur einen Parameter gleichzeitig veraendern.
