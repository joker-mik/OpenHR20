# OpenHR20: komplette UART-, EEPROM- und Diagnose-Anleitung

Diese Anleitung gilt primaer fuer die **standalone HR20/HR25 ohne RFM (`RFM=0`)**. Bei beiden werden Software- und PE2-Hardware-Fenstererkennung gemeinsam kompiliert und zur Laufzeit ausgewaehlt.

Sie beschreibt:

- Anschluss und Nutzung der seriellen Schnittstelle,
- Lesen und Aendern der im EEPROM gespeicherten Konfiguration,
- Bedeutung aller Konfigurations-Indizes,
- Schaltzeiten,
- Solltemperatur und Betriebsart,
- Software-Fenstererkennung,
- Diagnosewerte,
- Reset auf Defaults,
- physisches EEPROM-Layout.

> Wichtig: Die Befehle `Gxx` und `Sxxyy` arbeiten mit einem **Konfigurationsindex**, nicht mit einer direkten physischen EEPROM-Adresse.

> Wichtig: Bei anderen Build-Varianten koennen sich insbesondere die Indizes **ab `0x23`** verschieben oder andere Bedeutungen haben.

---

## 1. Serielle Verbindung

Bei einem Nicht-RFM-HR20 aktiviert dieser Fork den UART automatisch.

- Baudrate: **9600 Baud**
- Datenformat: **8N1**
- Befehlsende: **Newline** (`\n`)
- ein empfangenes `\r` wird intern in `\n` umgewandelt
- Befehlsbuchstaben sind **gross**
- Hex-Ziffern `a` bis `f` muessen **klein** geschrieben werden
- keine Leerzeichen innerhalb eines Befehls

Am externen HR20-Stecker liegen die MCU-Signale direkt an:

| Signal | ATmega169 |
| --- | --- |
| RXD | PE0 |
| TXD | PE1 |
| GND | GND |
| Logik-/Versorgungsreferenz | Vcc |

Das ist ein MCU-UART und **kein klassischer RS-232-Pegel mit +/-12 V**. Einen USB-UART-Adapter nur mit passendem Logikpegel und gemeinsamem GND verwenden. Nicht blind 5 V auf RX/TX einspeisen.

### Minimaler Funktionstest

Nach dem Verbinden:

```text
V
```

liefert die Firmware-Version.

```text
D
```

liefert eine Statuszeile.

---

## 2. Syntax der wichtigsten Befehle

| Befehl | Funktion |
| --- | --- |
| `V` | Firmware-Version ausgeben |
| `D` | Statuszeile ausgeben |
| `Taa` | Diagnose-/Watch-Wert lesen |
| `Gaa` | Konfigurationswert lesen |
| `Saadd` | Konfigurationswert schreiben |
| `Rab` | Timer lesen |
| `Wabcddd` | Timer schreiben |
| `Axx` | Solltemperatur setzen |
| `M00` | manueller Modus |
| `M01` | Automatikmodus |
| `Mfd` | Betriebsart nicht aendern, Fensterzustand beenden |
| `L00` | Tastensperre aus |
| `L01` | Tastensperre ein |
| `L02` | Tastensperren-Status lesen |
| `Hhhmmss` | Uhrzeit setzen, Werte hexadezimal |
| `Yyymmdd` | Datum setzen, Werte hexadezimal |
| `B1324` | Neustart ueber Watchdog |

Alle Befehle mit Hex-Parametern erwarten die Ziffern `a` bis `f` in **Kleinschreibung**.

---

## 3. EEPROM-Konfiguration lesen und schreiben

### 3.1 Lesen

```text
Gaa
```

`aa` ist der zweistellige hexadezimale Konfigurationsindex.

Beispiel:

```text
G0b
```

Bei Defaultwerten kommt fuer `valve_min` zurueck:

```text
G[0b]=1e
```

`0x1e` = 30 dezimal.

### 3.2 Schreiben

```text
Saadd
```

- `aa` = Konfigurationsindex in Hex
- `dd` = neuer 8-Bit-Wert in Hex

Beispiel: Komforttemperatur (`03`) auf 20.0 degC setzen.

20.0 degC / 0.5 degC = 40 = `0x28`:

```text
S0328
```

Antwort:

```text
S[03]=28
```

### 3.3 Bereichspruefung

Jeder Konfigurationseintrag besitzt im EEPROM vier Bytes:

1. aktueller Wert
2. Defaultwert
3. Minimalwert
4. Maximalwert

Beim Schreiben prueft die Firmware den erlaubten Bereich.

Ein ungueltiger Wert wird **nicht auf Min oder Max begrenzt**, sondern auf den **Defaultwert** des Parameters zurueckgesetzt und gespeichert.

Darum nach jedem `S...` die Antwort kontrollieren oder noch einmal mit `G...` lesen.

### 3.4 EEPROM-Layout-Version

```text
Gff
```

liefert die kompilierte EEPROM-Layout-Version.

Fuer die runtime-umschaltbaren Standalone-HR20/HR25 gilt:

```text
16
```

Beim ersten Start migriert Firmware 1.4 die alten Layouts `0x14` (Software) und `0x15` (Hardware) automatisch auf `0x16`. Die bisherigen Fensterwerte werden dabei soweit moeglich uebernommen. Fuer ein Upgrade mit erhaltenen Einstellungen nur das Flash-HEX programmieren und das bestehende EEPROM nicht mit der neuen `.eep`-Datei ueberschreiben.

---

## 4. Vollstaendige Parameter-Tabelle

Gueltig fuer **HR20, RFM=0, Software-Fenstererkennung**.

| Index | Name / Bedeutung | Default Hex | Default lesbar | Bereich / Einheit | Hinweis |
| --- | --- | ---: | ---: | --- | --- |
| `00` | `lcd_contrast` | `0e` | 14 | 0..15 | LCD-Kontrast |
| `01` | `temperature0` | `0a` | 5.0 degC | 5.0..30.0 degC, 0.5 degC/LSB | Frostschutz |
| `02` | `temperature1` | `22` | 17.0 degC | 5.0..30.0 degC, 0.5 degC/LSB | Energiesparen |
| `03` | `temperature2` | `2a` | 21.0 degC | 5.0..30.0 degC, 0.5 degC/LSB | Komfort |
| `04` | `temperature3` | `30` | 24.0 degC | 5.0..30.0 degC, 0.5 degC/LSB | Superkomfort |
| `05` | `P3_Factor` | `21` | 33 | 0..255 | kubischer P-Anteil |
| `06` | `P_Factor` | `08` | 8 | 0..255 | proportionaler Anteil |
| `07` | `I_Factor` | `20` | 32 | 0..255 | Integralanteil |
| `08` | `I_max_credit` | `28` | 40 | 0..127 | Integrator-Begrenzung |
| `09` | `I_credit_expiration` | `1e` | 30 | 0..255 PID-Intervalle | Default: 30 x 240 s = 2 h |
| `0a` | `PID_interval` | `30` | 48 -> 240 s | Wert x 5 s, min. 20 s | Reglerintervall |
| `0b` | `valve_min` | `1e` | 30 % | 0..100 % | minimale Regel-Stellposition; **nicht 30 % Heizleistung** |
| `0c` | `valve_center` | `2d` | 45 % | 0..100 % | Mittel-/Startposition |
| `0d` | `valve_max` | `50` | 80 % | 0..100 % | maximale Regel-Stellposition |
| `0e` | `valve_hysteresis` | `40` | 0.5 % | 1/128 % pro LSB, max. 127 | verhindert kleine Motorbewegungen |
| `0f` | `motor_pwm_min` | `20` | 32 | 32..255 | minimale Motor-PWM |
| `10` | `motor_pwm_max` | `fa` | 250 | 50..255 | maximale Motor-PWM |
| `11` | `motor_eye_low` | `64` | 100 | 1..255 | Mindestlaenge LOW des Lichtschrankensignals, intern x2 |
| `12` | `motor_eye_high` | `19` | 25 | 1..255 | Mindestlaenge HIGH des Lichtschrankensignals, intern x2 |
| `13` | `motor_close_eye_timeout` | `4e` | ca. 1.28 s | 5..255, 1/61 s pro LSB | Zeit ohne Puls bis Endanschlag angenommen wird |
| `14` | `motor_end_detect_cal` | `82` | 130 % | 110..250 % | Enderkennung bei Kalibrierung |
| `15` | `motor_end_detect_run` | `96` | 150 % | 110..250 % | Enderkennung im normalen Lauf |
| `16` | `motor_speed` | `b8` | 184 | 10..255 | Sollwert der Motor-Pulszeitregelung |
| `17` | `motor_speed_ctl_gain` | `32` | 50 | 10..200 | Verstaerkung der Motorregelung |
| `18` | `motor_pwm_max_step` | `0a` | 10 | 1..64 | max. PWM-Aenderung pro Puls |
| `19` | `MOTOR_ManuCalibration_L` | `ff` | 255 | 0..255 | Low-Byte manuelle Motorkalibrierung |
| `1a` | `MOTOR_ManuCalibration_H` | `ff` | 255 | 0..255 | High-Byte; `ffff` = automatische Kalibrierung |
| `1b` | `temp_cal_table0` | `27` | 39 | 0..255 ADC-Schritte | erster Temperatur-Kalibrierpunkt |
| `1c` | `temp_cal_table1` | `2d` | 45 | 16..255 ADC-Schritte | Delta |
| `1d` | `temp_cal_table2` | `39` | 57 | 16..255 ADC-Schritte | Delta |
| `1e` | `temp_cal_table3` | `4b` | 75 | 16..255 ADC-Schritte | Delta |
| `1f` | `temp_cal_table4` | `4d` | 77 | 16..255 ADC-Schritte | Delta |
| `20` | `temp_cal_table5` | `41` | 65 | 16..255 ADC-Schritte | Delta |
| `21` | `temp_cal_table6` | `3d` | 61 | 16..255 ADC-Schritte | Delta |
| `22` | `timer_mode` | `00` | 0 | 0..`7b` | Bit 0: Wochenprogramm; im manuellen Modus zusaetzlich Solltemperatur |
| `23` | `bat_warning_thld` | `78` | 2.40 V | 80..160, 0.02 V/LSB | Batterie-Warnschwelle |
| `24` | `bat_low_thld` | `64` | 2.00 V | 80..160, 0.02 V/LSB | Batterie-Unterspannungsgrenze |
| `25` | `allow_ADC_during_motor` | `01` | an | 0/1 | ADC auch waehrend Motorlauf |
| `26` | `window_detection_mode` | `01` | Software | 0=aus, 1=Software, 2=Hardware PE2 | Auswahl ohne Neu-Flashen |
| `27` | `window_open_detection_diff` | `32` | 0.50 degC | 7..255, 0.01 degC/LSB | SW: Temperaturabfall fuer Fenster auf |
| `28` | `window_close_detection_diff` | `32` | 0.50 degC | 7..255, 0.01 degC/LSB | SW: Temperaturanstieg fuer Fenster zu |
| `29` | `window_open_detection_time` | `08` | 120 s | 1..32, 15 s/LSB | SW: Beobachtungszeit Fenster auf |
| `2a` | `window_close_detection_time` | `08` | 120 s | 1..32, 15 s/LSB | SW: Beobachtungszeit Fenster zu |
| `2b` | `window_open_timeout` | `5a` | 90 min | 2..255 min | SW: maximale Fenster-offen-Dauer |
| `2c` | `hw_window_open_detection_delay` | `05` | 5 s | 0..240 s | HW/PE2: Oeffnungsverzoegerung |
| `2d` | `hw_window_close_detection_delay` | `05` | 5 s | 0..240 s | HW/PE2: Schliessverzoegerung |

**HR25-Hinweis:** Der HR25 besitzt zusaetzlich `bat_half_thld` an Index `23`. Dadurch verschieben sich beim HR25 alle Eintraege ab der HR20-Zeile `23` um **+1**: `bat_warning_thld=24`, `bat_low_thld=25`, `allow_ADC_during_motor=26`, `window_detection_mode=27` und die Fensterparameter laufen bis `2e`. Die Migration verwendet deshalb Struktur-Offsets statt fest codierter Indizes.

---

## 5. Temperaturen umrechnen

Die vier Solltemperaturen `01` bis `04` arbeiten in 0.5-degC-Schritten.

Formel:

```text
EEPROM-Wert = Temperatur in degC x 2
```

Beispiele:

| Temperatur | Dezimal | Hex |
| ---: | ---: | ---: |
| 5.0 degC | 10 | `0a` |
| 17.0 degC | 34 | `22` |
| 20.0 degC | 40 | `28` |
| 21.0 degC | 42 | `2a` |
| 22.0 degC | 44 | `2c` |
| 24.0 degC | 48 | `30` |

Beispiel Komforttemperatur 22.0 degC:

```text
S032c
```

---

## 6. Temperatur-Kalibriertabelle `1b` bis `21`

Diese Werte nur gezielt aendern, wenn die Temperaturmessung kalibriert werden soll.

Beim HR20 ist der feste ADC-Offset `256`. `1b` ist der erste Abstand, danach folgen Differenzen:

```text
35 degC: 256 + 0x27 = 295
30 degC: 295 + 0x2d = 340
25 degC: 340 + 0x39 = 397
20 degC: 397 + 0x4b = 472
15 degC: 472 + 0x4d = 549
10 degC: 549 + 0x41 = 614
 5 degC: 614 + 0x3d = 675
```

Ein geaenderter Delta-Wert verschiebt damit auch die darunterliegenden Punkte.

---

## 7. Batterie-Schwellen

Die Firmware vergleicht die Batteriespannung mit:

```text
Schwelle [mV] = EEPROM-Wert x 20
```

Beispiele:

```text
G23 -> 78 hex = 120 dezimal = 2400 mV
G24 -> 64 hex = 100 dezimal = 2000 mV
```

---

## 8. Software-Fenstererkennung

### 8.1 Automatische Erkennung

Die Software betrachtet Temperaturmittelwerte ueber ein Zeitfenster.

Default:

```text
Fenster auf: Temperaturabfall > 0.50 degC innerhalb von 2 min
Fenster zu:  Temperaturanstieg > 0.50 degC innerhalb von 2 min
Timeout:     90 min
```

Beim erkannten offenen Fenster wird intern `CTL_mode_window` auf `window_open_timeout` gesetzt.

Solange der Fensterzustand aktiv ist, regelt der Controller auf die Frostschutz-Untergrenze `TEMP_MIN` (5 degC), unabhaengig von der normalen Solltemperatur.

Der Fenster-Timer wird einmal pro Minute heruntergezaehlt. Der Zustand endet:

- durch erkannte Fenster-Schliessung,
- durch Ablauf von `window_open_timeout`,
- oder durch einen expliziten Befehl, der den Fensterzustand loescht.

### 8.2 Fensterzustand abfragen

Mit:

```text
D
```

erscheint am Ende der Statuszeile ein `W`, solange der Fensterzustand aktiv ist.

Direkter Diagnosewert:

```text
T04
```

`T04` liefert `CTL_mode_window`. `0000` bedeutet Fensterzustand aus. Ein Wert groesser Null ist der verbleibende interne Fenster-Timer.

### 8.3 Erkennungsart ueber EEPROM waehlen

Index `26` waehlt die aktive Erkennungsart zur Laufzeit:

| Wert | Modus |
| ---: | --- |
| `00` | Fenstererkennung aus |
| `01` | Software-Fenstererkennung ueber Temperaturverlauf |
| `02` | Hardware-Fensterkontakt an PE2 |

Beispiele ueber UART:

```text
S2600   Erkennung aus
S2601   Software
S2602   Hardware PE2
```

Der gleiche Wert kann im Servicemenue direkt am HR20 geaendert werden. Beim Wechsel des Modus loescht die Firmware einen eventuell noch aktiven Fensterzustand, damit ein SW-Minutenzaehler nicht als HW-Zustand weiterverwendet wird.

### 8.4 Software-Fenster manuell am Geraet schalten

Nur bei `26 = 01`:

```text
PROG + AUTO ca. 3 Sekunden gedrueckt halten
```

schaltet den Software-Fensterzustand ein bzw. aus. AUTO/MANU bleibt unveraendert. Beim Einschalten wird der Timer aus `2b` geladen. `OPEn`, Frostschutzregelung, automatische Schliesserkennung und Timeout verhalten sich danach wie bei automatisch erkanntem Fenster.

Bei `26 = 00` oder `26 = 02` wird diese Tastenkombination fuer den Fensterzustand ignoriert. Im Hardwaremodus bleibt PE2 die alleinige Quelle.

Ueber UART beendet `Mfd` weiterhin einen aktiven Fensterzustand, ohne AUTO/MANU zu aendern.

### 8.5 Software-Parameter

| Index | Parameter | Default |
| --- | --- | ---: |
| `27` | Temperaturabfall zum Oeffnen | 0.50 degC |
| `28` | Temperaturanstieg zum Schliessen | 0.50 degC |
| `29` | Beobachtungszeit Oeffnen | 120 s |
| `2a` | Beobachtungszeit Schliessen | 120 s |
| `2b` | maximaler Software-Fensterzustand | 90 min |

### 8.6 Hardware-Fensterkontakt PE2

Bei `26 = 02` wird PE2 ausgewertet. Der Eingang arbeitet mit Pull-up; die klassische HR20-Hardwarebeschaltung ist Kontakt zwischen PE2 und GND. Die Firmware entprellt/verzoegert Oeffnen und Schliessen getrennt:

| Index | Parameter | Default |
| --- | --- | ---: |
| `2c` | Oeffnungsverzoegerung | 5 s |
| `2d` | Schliessverzoegerung | 5 s |

Im Hardwaremodus gibt es keinen 90-Minuten-Timeout: der physische Kontakt ist massgeblich.

### 8.7 Migration von alten EEPROM-Layouts

Firmware 1.3 verwendet gemeinsam `EE_LAYOUT = 0x16`.

- `0x14` wird als alter Software-Build erkannt; die Werte `26..2a` werden nach `27..2b` verschoben und Modus `01` gesetzt.
- `0x15` wird als alter Hardware-Build erkannt; Enable/Delays werden nach Modus `00/02` und `2c/2d` uebernommen.
- Die Indizes `00..25` bleiben unveraendert.
- Fuer einen Stromausfall waehrend der Migration werden die alten Fensterwerte zuerst im reservierten EEPROM-Bereich zwischengespeichert; `ee_layout` wird erst am Ende auf `0x16` gesetzt.
---

## 9. Schaltzeiten im EEPROM

Die Wochen-Schaltzeiten liegen getrennt von `ee_config` im Array `ee_timers`.

Es gibt **8 Slots pro Tag**.

### 9.1 Timer lesen

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

### 9.2 Timer schreiben

```text
Wabcddd
```

Beispiel: Montag, Slot 0, Komfort (`2`) um 07:00 Uhr.

07:00 = 420 Minuten = `0x1a4`:

```text
W1021a4
```

Danach kontrollieren:

```text
R10
```

### 9.3 Slot deaktivieren

Zeit `fff` bedeutet deaktiviert.

Beispiel Montag Slot 7 deaktiviert mit Energiespar-Typ `1`:

```text
W171fff
```

---

## 10. Solltemperatur direkt setzen

```text
Axx
```

Einheit: 0.5 degC.

Beispiel 20.0 degC:

```text
A28
```

Beispiel 21.0 degC:

```text
A2a
```

Im AUTO-Modus ist das eine manuelle Sollwertaenderung innerhalb des Automatikbetriebs; das AUTO-Symbol bleibt in diesem Fork erhalten.

---

## 11. Betriebsart AUTO / MANU

```text
M00
```

setzt MANU.

```text
M01
```

setzt AUTO.

```text
Mfd
```

laesst die Betriebsart unveraendert und beendet nur einen aktiven Fensterzustand.

Jeder Aufruf von `CTL_change_mode()` setzt den Software-Fensterzustand am Ende zurueck.

---

## 12. Uhr und Datum

### Uhrzeit

```text
Hhhmmss
```

Alle Werte sind **hexadezimal**.

Beispiel 18:30:00:

- 18 dezimal = `12`
- 30 dezimal = `1e`
- 00 = `00`

```text
H121e00
```

### Datum

```text
Yyymmdd
```

Jahr ist der Offset ab 2000 und ebenfalls hexadezimal.

Beispiel 15.09.2026:

- 26 dezimal = `1a`
- 9 = `09`
- 15 dezimal = `0f`

```text
Y1a090f
```

---

## 13. Tastensperre

```text
L00
```

entsperrt.

```text
L01
```

sperrt.

```text
L02
```

fragt nur den Status ab.

---

## 14. Statuszeile `D`

Der Befehl:

```text
D
```

gibt u. a. aus:

- Datum und Uhrzeit,
- Betriebsart,
- `V:` angeforderte Ventilposition,
- `P:` aktuelle kalibrierte Motorposition in Prozent,
- `R:` Anzahl automatischer Close-Re-References seit Boot,
- `I:` gemessene Temperatur,
- `S:` Solltemperatur,
- `B:` Batteriespannung,
- ggf. `E:` Fehlerbits,
- `W` bei aktivem Fensterzustand,
- `L` bei Tastensperre.

---

## 15. Diagnosewerte `Txx`

`Txx` liest **RAM-Diagnosewerte**, nicht EEPROM.

| Index | Bedeutung |
| --- | --- |
| `00` | untere 16 Bit von `sumError` |
| `01` | obere 16 Bit von `sumError` |
| `02` | Integrator-Credit |
| `03` | Credit-Expiration |
| `04` | Fensterzustand / Rest-Timer `CTL_mode_window` |
| `05` | Motor-Pulsdiagnose `motor_diag` |
| `06` | kalibrierter voller Motorweg `MOTOR_PosMax` |
| `07` | aktuelle interne Motorposition `MOTOR_PosAct` |
| `08` | Motor-Overshoot |
| `09` | erfolgreiche automatische Close-Re-References seit Boot |
| `0a` | Positionskorrektur der letzten Close-Re-Reference, signed 16 Bit |
| `0b`/`0c` | Motor-Pulszaehler, falls `DEBUG_MOTOR_COUNTER=1` |

Fuer die Beobachtung des bekannten Positionsdrift-Problems sind besonders interessant:

```text
T06
T07
T09
T0a
```

Fuer Fensterdiagnose:

```text
T04
```

---

## 16. Fehlerbits in der Statuszeile

Wenn die Statuszeile `E:xx` enthaelt, ist `xx` eine Bitmaske.

| Bit | Hex | Bedeutung |
| ---: | ---: | --- |
| 7 | `80` | Batterie LOW |
| 6 | `40` | Batterie WARNING |
| 4 | `10` | RFM Sync, nur bei RFM relevant |
| 3 | `08` | Motorfehler |
| 2 | `04` | Montagefehler |

Mehrere Fehler werden bitweise kombiniert.

Beispiel:

```text
E:48
```

entspricht `0x40 + 0x08`: Batterie-Warnung plus Motorfehler.

---

## 17. EEPROM auf Defaultwerte zuruecksetzen

Beim Einschalten prueft die Firmware die drei Tasten:

```text
PROG + C + AUTO
```

Sind alle drei beim Start gedrueckt, werden die Konfigurationswerte aus ihren Defaultspalten neu geladen und gespeichert.

Das betrifft die Konfigurationswerte. Die Schaltzeiten liegen in einem eigenen EEPROM-Bereich.

---

## 18. Physisches EEPROM-Layout

Im abgesicherten Standalone-Build prueft CI folgende Startadressen:

```text
0x0003  ee_layout
0x0004  ee_timers
0x0084  reservierter Bereich
0x00c0  ee_config
```

`ee_config` besitzt vier Bytes pro Index:

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

Der aktuelle Wert liegt in diesem Build damit physisch bei `0x00ec`.

Direktes Bearbeiten des EEPROM-Images ist moeglich, aber fuer normale Einstellungen nicht empfohlen. `G` und `S` verwenden die Firmware-Struktur und die eingebaute Bereichspruefung.

---

## 19. Welche Werte sollte man nur mit Vorsicht aendern?

Besonders kritisch:

- `05`..`09`: PID-Tuning
- `0f`..`18`: Motorregelung und Endanschlag-/Lichtschranken-Erkennung
- `19`/`1a`: Motorkalibrierung
- `1b`..`21`: Temperatur-ADC-Kalibrierung
- `23`/`24`: Batterie-Schutzschwellen

Leichter nachvollziehbar fuer normale Experimente:

- `01`..`04`: Temperaturstufen
- `0b`..`0d`: Ventilgrenzen
- `26`..`2a`: Software-Fenstererkennung

Vor jeder Aenderung den alten Wert mit `Gxx` notieren und moeglichst immer nur **einen Parameter gleichzeitig** veraendern.

---

## 20. Kurze Praxisbeispiele

Komforttemperatur lesen:

```text
G03
```

Komforttemperatur auf 20.0 degC:

```text
S0328
```

Ventilminimum lesen:

```text
G0b
```

Batterie-Warnschwelle lesen:

```text
G23
```

Fensterzustand pruefen:

```text
T04
```

Fensterzustand beenden, AUTO/MANU unveraendert lassen:

```text
Mfd
```

Status inkl. Fenster-, Motor- und Batteriesituation:

```text
D
```

Firmware-Version:

```text
V
```
