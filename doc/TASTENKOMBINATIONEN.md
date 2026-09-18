# OpenHR20: Tasten und Tastenkombinationen

Diese Uebersicht gilt fuer den Standalone-Honeywell-HR20 dieses Forks. `lang` bedeutet ungefaehr 3 Sekunden; die Firmware verwendet `LONG_PRESS_THLD=3` und wertet den Langdruck im Sekundenraster aus.

## Schnelluebersicht

| Eingabe | Kontext | Wirkung |
| --- | --- | --- |
| Drehrad | Hauptanzeige | Solltemperatur in 0.5-degC-Schritten aendern |
| `C` kurz | Haupt-/Infobildschirm | naechster Infobildschirm |
| `AUTO` kurz | Hauptanzeige | AUTO <-> MANU |
| `PROG` kurz | Hauptanzeige | keine eigene Funktion |
| `C` lang | entsperrt | vier Temperatur-Voreinstellungen bearbeiten |
| `PROG` lang | entsperrt | Wochen-Schaltzeiten programmieren |
| `AUTO` lang | entsperrt | Datum und Uhrzeit einstellen |
| `AUTO + C` lang | auch bei Sperre | Tastensperre ein/aus |
| `PROG + AUTO` lang | entsperrt, Fenstermodus `01` | Software-Fenster ein/aus |
| `PROG + C` lang | laufender Betrieb | keine Funktion |
| `PROG + C + AUTO` lang | entsperrt | Servicemenue |
| `PROG + C + AUTO` beim Einschalten | Boot | Konfigurations-Defaults laden |

Kurze Mehrtasten-Kombinationen haben keine eigene Benutzerfunktion. Drehrad plus Taste hat keine definierte Sonderfunktion.

## Hauptanzeige und Infoseiten

Das Drehrad aendert die Solltemperatur. Im AUTO-Modus bleibt AUTO aktiv und das AUTO-Symbol sichtbar.

`C` kurz schaltet bei aktivierter Batterieanzeige durch:

```text
Solltemperatur -> Isttemperatur -> Ventilposition -> Uhrzeit -> Batteriespannung -> Solltemperatur
```

`AUTO` kurz schaltet auf der Hauptanzeige AUTO/MANU um. Auf einer alternativen Infoseite fuehrt eine Bedieneingabe zunaechst zur Hauptanzeige zurueck.

## Lange Einzeltasten

### C lang: Temperaturstufen

```text
C ca. 3 s
```

bearbeitet nacheinander Frostschutz, Energiesparen, Komfort und Superkomfort. Drehrad aendert den Wert, `PROG` speichert und geht weiter, `AUTO` verlaesst das Menue. Noch nicht mit `PROG` bestaetigte Aenderungen werden verworfen.

### PROG lang: Wochenprogramm

```text
PROG ca. 3 s
```

waehlt zuerst Tag/Fallback-Programm. Drehrad waehlt `0` bzw. Montag..Sonntag, `PROG` bestaetigt, `AUTO` beendet. In den acht Slots pro Tag aendert das Drehrad die Uhrzeit in 10-Minuten-Schritten, `C` wechselt Temperaturtyp 0..3, `PROG` speichert den Slot, `AUTO` beendet.

### AUTO lang: Datum/Uhrzeit

```text
AUTO ca. 3 s
```

Reihenfolge: Jahr -> Monat -> Tag -> Stunde -> Minute. Drehrad aendert, `PROG` geht zum naechsten Feld.

## Lange Kombinationen

### AUTO + C: Tastensperre

```text
AUTO + C ca. 3 s
```

schaltet die Tastensperre um. Beim HR20 ohne Schloss-Segment erscheint bei Bedienversuchen im gesperrten Zustand `bloc`. Dieselbe Kombination bleibt zum Entsperren wirksam.

### PROG + AUTO: Software-Fenster

Nur bei EEPROM `26 = 01` (Software):

```text
PROG + AUTO ca. 3 s
```

schaltet `OPEn` ein/aus. AUTO/MANU bleibt unveraendert. Beim Einschalten wird `CTL_mode_window` auf den Timeout aus EEPROM `2b` gesetzt. Bei `26 = 00` oder `02` wird diese Fensterfunktion ignoriert; in Hardwaremodus `02` ist PE2 massgeblich.

### PROG + C

```text
PROG + C ca. 3 s
```

ist derzeit nicht belegt.

### PROG + C + AUTO: Servicemenue

```text
PROG + C + AUTO ca. 3 s
```

oeffnet im laufenden entsperrten Betrieb das Servicemenue.

In der Parameter-Auswahl: Drehrad waehlt den EEPROM-Index, `PROG` geht in die Bearbeitung, `C` ins Watch-Menue, `AUTO` beendet. In der Bearbeitung aendert das Drehrad den Wert, `PROG` speichert, `C` verwirft und wechselt ins Watch-Menue, `AUTO` verwirft und beendet. Im Watch-Menue waehlt das Drehrad `T00`, `T01`, ...; `C` geht zur Parameter-Auswahl, `AUTO` beendet.

Damit kann insbesondere Index `26` direkt am HR20 zwischen `00` (aus), `01` (Software) und `02` (Hardware PE2) umgestellt werden.

## Beim Einschalten: Defaults

```text
PROG + C + AUTO gedrueckt halten und Geraet starten/Batterien einsetzen
```

laedt die Defaultwerte der Konfigurationsparameter. Das ist eine andere Funktion als derselbe Langdruck im laufenden Betrieb. Die Wochen-Schaltzeiten liegen in einem separaten EEPROM-Bereich.

## Motor-Adaption / Kalibrierung

Wenn die Motoradaption aussteht, werden `C` und `PROG` zusaetzlich von der Kalibrierlogik ausgewertet:

| Taste | Wirkung |
| --- | --- |
| `PROG` | automatische Vollkalibrierung erzwingen |
| `C` | manuelle/einseitige Neukalibrierung erzwingen |
| keine | gespeicherte Kalibrierart bzw. gespeicherten Weg wiederverwenden, soweit gueltig |

Diese Kalibrierfunktionen gelten nur im entsprechenden Adaptionszustand.
