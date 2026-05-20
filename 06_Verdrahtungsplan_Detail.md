# SmartBLDCServo – Detaillierter Verdrahtungsplan

> Dieser Plan beschreibt **jede einzelne Verbindung** zwischen den Modulen
> und erklärt exakt, wo jeder Widerstand, Kondensator und die Schutzdiode
> angeschlossen werden muss.

---

## Übersicht der Module (werden NICHT verändert)

| Ref  | Modul                    | Funktion                       |
|------|--------------------------|--------------------------------|
| U1   | ESP32-C3 Super Mini      | Mikrocontroller (MCU)          |
| U2   | SimpleFOC Mini v1.0      | BLDC-Motortreiber (DRV8313)    |
| U3   | MP1584EN Modul           | Step-Down 12V → 5V             |
| U4   | BOB-12009 (BSS138 Board) | Level Shifter 3.3V ↔ 5V       |
| J1   | Schraubklemme 2-pol      | 12V Eingang (VMOT + GND)       |
| J2   | JST-XH 3-pol             | Feetech-Bus (5V, DATA, GND)   |
| J3   | JST-XH 4-pol             | AS5600 Encoder (VCC,GND,SDA,SCL)|

---

## Übersicht aller passiven Bauteile

| Ref   | Typ       | Wert          | Funktion                           | Angeschlossen zwischen            |
|-------|-----------|---------------|------------------------------------|------------------------------------|
| D1    | TVS-Diode | SMBJ15A       | Überspannungsschutz 12V            | VMOT (Kathode) ↔ GND (Anode)      |
| C1    | Elko      | 1000µF/25V    | Bulk-Puffer 12V                    | VMOT ↔ GND                        |
| C2    | Keramik   | 100nF/50V     | HF-Entkopplung SimpleFOC           | VMOT ↔ GND (direkt am U2)         |
| C3    | Keramik   | 10µF/25V      | Eingangspuffer Buck-Converter      | U3 IN+ ↔ U3 IN-                   |
| C4    | Keramik   | 22µF/16V      | Ausgangspuffer Buck-Converter      | U3 OUT+ ↔ U3 OUT-                 |
| C5    | Elko      | 100µF/10V     | MCU-Puffer (Low-ESR)               | U1 5V-Pin ↔ GND                   |
| C6    | Keramik   | 47µF/16V      | MCU-Puffer (X7R, 1206)             | U1 5V-Pin ↔ GND                   |
| C7    | Keramik   | 100nF         | MCU HF-Filter                      | U1 5V-Pin ↔ GND                   |
| C8    | Keramik   | 10µF/10V      | AS5600 Entkopplung                 | J3 Pin1 (VCC) ↔ J3 Pin2 (GND)     |
| R1    | 0603      | 10kΩ          | EN Pull-Down (Treiber aus bei Boot)| U2 EN ↔ GND                        |
| R2    | 0603      | 10kΩ          | nFLT Pull-Up (Fehlerleitung)       | U2 nFLT ↔ 3.3V                    |
| R3    | 0603      | 10kΩ          | nSLP Pull-Up (Treiber aktiv)       | U2 nSLP ↔ 3.3V                    |
| R4    | 0603      | 10kΩ          | nRST Pull-Up (kein Reset)          | U2 nRST ↔ 3.3V                    |
| R5    | 0603      | 4.7kΩ         | I2C SDA Pull-Up                    | SDA-Leitung ↔ 3.3V                |
| R6    | 0603      | 4.7kΩ         | I2C SCL Pull-Up                    | SCL-Leitung ↔ 3.3V                |
| R7    | 0603      | 220Ω          | TX Serienwiderstand (BOB-12009)    | U1 GPIO21 ↔ U4 LV1                |
| R8    | 0603      | 4.7kΩ         | RX Spannungsteiler (Serie)         | TTL_1_Wire ↔ U1 GPIO20            |
| R9    | 0603      | 10kΩ          | RX Spannungsteiler (Pull-Down)     | U1 GPIO20 ↔ GND                   |
| R10   | 0603      | 2.2kΩ         | Bus-Terminierung Pull-Up           | TTL_1_Wire ↔ 5V                   |

---

## TEIL 1: STROMVERSORGUNG

### 1.1 – 12V Eingang und Schutz

```
       J1 (Schraubklemme)
       ┌──────┐
  12V──┤ Pin1 ├──┬──────────────────────────┬──── VMOT-Schiene
       │      │  │                          │
  GND──┤ Pin2 ├──┼──────────────────────────┼──── GND-Schiene
       └──────┘  │                          │
                 │                          │
           ┌─────┴─────┐            ┌───────┴───────┐
           │    D1      │            │     C1        │
           │  SMBJ15A   │            │  1000µF/25V   │
           │  (TVS)     │            │  (Elko)       │
           │            │            │               │
           │ Kathode=VMOT│           │  (+) = VMOT   │
           │ Anode=GND  │            │  (-) = GND    │
           └─────┬─────┘            └───────┬───────┘
                 │                          │
                 └──────────GND─────────────┘
```

**D1 (SMBJ15A):** Direkt parallel an J1. Kathode (Strich-Seite) an 12V/VMOT, Anode an GND.
Zweck: Kappt Spannungsspitzen von regenerativem Motorbremsen.

**C1 (1000µF/25V):** Direkt parallel an J1. Plus an VMOT, Minus an GND.
Zweck: Puffert große Stromspitzen beim Motorstart.

### 1.2 – SimpleFOC Mini Stromversorgung

```
  VMOT-Schiene ───────────────┬──── U2 VCC (Pin 14)
                              │
                         ┌────┴────┐
                         │   C2    │
                         │ 100nF   │
                         │ /50V    │
                         └────┬────┘
                              │
  GND-Schiene ────────────────┴──── U2 GND (Pin 15)
```

**C2 (100nF/50V):** So nah wie möglich an den VCC/GND-Pins des SimpleFOC Mini löten.
Zweck: Filtert hochfrequentes Rauschen des DRV8313 Treibers.

### 1.3 – Buck-Converter (MP1584EN) 12V → 5V

```
  VMOT ──┬──── U3 IN+          U3 OUT+ ──┬──── 5V-Schiene
         │                               │
    ┌────┴────┐                     ┌─────┴────┐
    │   C3    │                     │    C4    │
    │ 10µF    │                     │  22µF    │
    │ /25V    │                     │  /16V    │
    └────┬────┘                     └─────┬────┘
         │                               │
  GND ───┴──── U3 IN-          U3 OUT- ──┴──── GND
```

**C3 (10µF):** Direkt an IN+/IN- des MP1584EN Moduls.
**C4 (22µF):** Direkt an OUT+/OUT- des MP1584EN Moduls.

### 1.4 – ESP32-C3 Spannungsversorgung (5V-Eingang)

```
  5V-Schiene ──┬───────────────────────── U1 Pin "5V" (Pin 8)
               │
          ┌────┴────┐  ┌────┴────┐  ┌────┴────┐
          │   C5    │  │   C6    │  │   C7    │
          │ 100µF   │  │  47µF   │  │ 100nF   │
          │ Elko    │  │ Keramik │  │ Keramik │
          │ /10V    │  │ /16V    │  │         │
          └────┬────┘  └────┬────┘  └────┬────┘
               │            │            │
  GND ─────────┴────────────┴────────────┘
```

**C5, C6, C7:** Alle drei parallel, direkt am 5V-Pin des ESP32-C3 gegen GND.
Zweck: Verhindert MCU-Brownouts bei WiFi/BLE-Spitzenstrom.

### 1.5 – 3.3V Logik (vom ESP32 LDO)

```
  U1 Pin "3V3" (Pin 6) ───── 3.3V-Schiene
                               │
                               ├── R2, R3, R4 (Pull-Ups)
                               ├── R5, R6 (I2C Pull-Ups)
                               ├── U4 LV (BOB-12009 Low-Side)
                               └── J3 Pin1 (AS5600 VCC)
```

**WICHTIG:** Der 3.3V-Ausgang am SimpleFOC Mini (Pin 2) bleibt **UNBESCHALTET!**

---

## TEIL 2: ESP32-C3 ↔ SimpleFOC Mini (Motorsteuerung)

### 2.1 – PWM-Signale (direkte Verbindungen, KEINE Widerstände)

```
  U1 GPIO 0 ─────────────────── U2 IN1  (Phase A)
  U1 GPIO 1 ─────────────────── U2 IN2  (Phase B)
  U1 GPIO 2 ─────────────────── U2 IN3  (Phase C)
```

### 2.2 – Enable mit Pull-Down (R1)

```
  U1 GPIO 3 ──────┬──────────── U2 EN
                   │
              ┌────┴────┐
              │   R1    │
              │  10kΩ   │
              └────┬────┘
                   │
                  GND
```

**R1 (10kΩ):** Ein Bein an die Leitung GPIO3↔EN, anderes Bein an GND.
Zweck: Hält den Motortreiber während des ESP32-Bootvorgangs DEAKTIVIERT.

### 2.3 – Fault-Pin mit Pull-Up (R2)

```
                  3.3V
                   │
              ┌────┴────┐
              │   R2    │
              │  10kΩ   │
              └────┬────┘
                   │
  U1 GPIO 4 ──────┴──────────── U2 nFLT
```

**R2 (10kΩ):** Ein Bein an die Leitung GPIO4↔nFLT, anderes Bein an 3.3V.
Zweck: nFLT ist Open-Drain. Pull-Up hält Signal auf HIGH wenn kein Fehler vorliegt.

### 2.4 – Sleep und Reset permanent aktiv (R3, R4)

```
                  3.3V                3.3V
                   │                   │
              ┌────┴────┐         ┌────┴────┐
              │   R3    │         │   R4    │
              │  10kΩ   │         │  10kΩ   │
              └────┬────┘         └────┬────┘
                   │                   │
              U2 nSLP              U2 nRST
```

**R3 (10kΩ):** Von U2 nSLP (Pin 8) nach 3.3V. Treiber bleibt aus Sleep-Modus.
**R4 (10kΩ):** Von U2 nRST (Pin 6) nach 3.3V. Treiber bleibt aus Reset.

> **ACHTUNG:** nSLP und nRST haben interne 100kΩ Pull-Downs im DRV8313!
> Ohne diese externen 10kΩ Pull-Ups bleibt der Treiber im Schlaf/Reset!

---

## TEIL 3: ESP32-C3 ↔ AS5600 Encoder (I2C über J3)

### 3.1 – Encoder-Steckerbelegung (J3, JST-XH 4-pol)

```
       J3 (4-pol Buchse)
       ┌──────────┐
  VCC──┤  Pin 1   ├──── 3.3V (vom ESP32 LDO)
  GND──┤  Pin 2   ├──── GND
  SDA──┤  Pin 3   ├──── U1 GPIO 8
  SCL──┤  Pin 4   ├──── U1 GPIO 9
       └──────────┘
```

### 3.2 – I2C Pull-Ups (R5, R6) und Entkopplung (C8)

```
         3.3V         3.3V
          │             │
     ┌────┴────┐   ┌────┴────┐
     │   R5    │   │   R6    │
     │  4.7kΩ  │   │  4.7kΩ  │
     └────┬────┘   └────┬────┘
          │             │
  GPIO8 ──┴── SDA       │
                        │
  GPIO9 ────────────────┴── SCL


  J3 Pin1 (VCC) ──┬──── 3.3V
                   │
              ┌────┴────┐
              │   C8    │
              │  10µF   │
              └────┬────┘
                   │
  J3 Pin2 (GND) ──┴──── GND
```

**R5 (4.7kΩ):** Ein Bein an SDA-Leitung (GPIO8 ↔ J3 Pin3), anderes an 3.3V.
**R6 (4.7kΩ):** Ein Bein an SCL-Leitung (GPIO9 ↔ J3 Pin4), anderes an 3.3V.
**C8 (10µF):** Direkt an J3 zwischen Pin1 (VCC) und Pin2 (GND). So nah wie möglich am Stecker.

---

## TEIL 4: FEETECH-BUS (1 Mbps, Split-Path über BOB-12009)

### 4.1 – Gesamtübersicht

```
  3.3V-Seite (ESP32)              5V-Seite (Bus)
  ══════════════════              ═══════════════

  GPIO21 (TX)──[R7 220Ω]──U4 LV1 ── U4 HV1──┬── TTL_1_Wire
                                              │
  GPIO20 (RX)──────────────[R8 4.7kΩ]────────┤
       │                                      │
      [R9 10kΩ]                          [R10 2.2kΩ]
       │                                      │
      GND                                    5V

  U4 LV  = 3.3V
  U4 HV  = 5V
  U4 GND = GND
```

### 4.2 – TX-Pfad: ESP32 → Bus (über BOB-12009 Kanal 1)

```
  U1 GPIO 21 ──[R7 220Ω]──── U4 LV1
                              U4 HV1 ──── TTL_1_Wire (Datenleitung)
```

**R7 (220Ω):** In Serie zwischen GPIO21 und dem LV1-Pin des BOB-12009.
Zweck: Begrenzt Strom und reduziert Überschwinger an der steigenden Flanke.

### 4.3 – RX-Pfad: Bus → ESP32 (Spannungsteiler, NICHT über BOB-12009)

```
  TTL_1_Wire (5V Bus) ──[R8 4.7kΩ]──┬──── U1 GPIO 20
                                      │
                                 [R9 10kΩ]
                                      │
                                     GND
```

**R8 (4.7kΩ):** Von TTL_1_Wire Busleitung zum GPIO20 (in Serie).
**R9 (10kΩ):** Von GPIO20 nach GND (Pull-Down).

Ergebnis: 5V × (10k / (4.7k + 10k)) = **3.4V** → sicher für den 3.3V-toleranten GPIO.
Zweck: Schneller, passiver Pegelwandler ohne MOSFET-Verzögerung für RX.

### 4.4 – Bus-Terminierung (R10)

```
  TTL_1_Wire ──[R10 2.2kΩ]──── 5V (von MP1584EN)
```

**R10 (2.2kΩ):** Direkt am Feetech-Stecker J2, von der Datenleitung nach 5V.
Zweck: Zieht die Busleitung schnell auf 5V zurück (scharfe steigende Flanken bei 1 Mbps).

### 4.5 – BOB-12009 Versorgung und unbeschaltete Pins

```
  U4 LV  ──── 3.3V (vom ESP32)
  U4 HV  ──── 5V (vom MP1584EN)
  U4 GND ──── GND
  U4 LV2 ──── unbeschaltet (offen)
  U4 LV3 ──── unbeschaltet (offen)
  U4 LV4 ──── unbeschaltet (offen)
  U4 HV2 ──── unbeschaltet (offen)
  U4 HV3 ──── unbeschaltet (offen)
  U4 HV4 ──── unbeschaltet (offen)
```

### 4.6 – Feetech-Stecker (J2, JST-XH 3-pol)

```
       J2 (3-pol Buchse)
       ┌──────────┐
   5V──┤  Pin 1   ├──── 5V (Versorgung für externe Servos)
  DATA──┤  Pin 2   ├──── TTL_1_Wire (Datenleitung)
  GND──┤  Pin 3   ├──── GND
       └──────────┘
```

---

## TEIL 5: MOTOR-AUSGÄNGE (SimpleFOC Mini → Motor)

Die drei Phasenausgänge des SimpleFOC Mini gehen direkt zum BLDC-Motor:

```
  U2 OUT1 (Pin 13) ──── Motor Phase A
  U2 OUT2 (Pin 12) ──── Motor Phase B
  U2 OUT3 (Pin 11) ──── Motor Phase C
```

Keine zusätzlichen Bauteile notwendig. Die Motorleitungen sollten möglichst
kurz und mit breiten Leiterbahnen (≥1.5mm) geführt werden.

---

## ZUSAMMENFASSUNG: Alle Verbindungen auf einen Blick

### ESP32-C3 Pin-Belegung (U1)

| ESP32 Pin | Richtung | Verbindung zu           | Passive Bauteile                     |
|-----------|----------|-------------------------|--------------------------------------|
| 5V        | Eingang  | MP1584EN OUT+            | C5 (100µF), C6 (47µF), C7 (100nF) → GND |
| 3V3       | Ausgang  | 3.3V-Schiene             | –                                    |
| GND       | –        | System-GND               | –                                    |
| GPIO 0    | Ausgang  | U2 IN1                   | –                                    |
| GPIO 1    | Ausgang  | U2 IN2                   | –                                    |
| GPIO 2    | Ausgang  | U2 IN3                   | –                                    |
| GPIO 3    | Ausgang  | U2 EN                    | R1 (10kΩ) → GND                     |
| GPIO 4    | Eingang  | U2 nFLT                  | R2 (10kΩ) → 3.3V                    |
| GPIO 8    | Bidir.   | J3 Pin3 (SDA)            | R5 (4.7kΩ) → 3.3V                   |
| GPIO 9    | Ausgang  | J3 Pin4 (SCL)            | R6 (4.7kΩ) → 3.3V                   |
| GPIO 20   | Eingang  | TTL_1_Wire via R8        | R8 (4.7kΩ Serie), R9 (10kΩ) → GND   |
| GPIO 21   | Ausgang  | U4 LV1 via R7            | R7 (220Ω Serie)                      |

### SimpleFOC Mini Pin-Belegung (U2)

| U2 Pin    | Verbindung zu           | Passive Bauteile                     |
|-----------|-------------------------|--------------------------------------|
| VCC (14)  | VMOT (12V-Schiene)      | C2 (100nF) → GND                    |
| GND (15)  | System-GND              | –                                    |
| GND1 (1)  | System-GND              | –                                    |
| GND2 (4)  | System-GND              | –                                    |
| IN1 (3)   | U1 GPIO 0               | –                                    |
| IN2 (5)   | U1 GPIO 1               | –                                    |
| IN3 (7)   | U1 GPIO 2               | –                                    |
| EN (9)    | U1 GPIO 3               | R1 (10kΩ) → GND                     |
| nRST (6)  | –                       | R4 (10kΩ) → 3.3V                    |
| nSLP (8)  | –                       | R3 (10kΩ) → 3.3V                    |
| nFLT (10) | U1 GPIO 4               | R2 (10kΩ) → 3.3V                    |
| 3.3V (2)  | **UNBESCHALTET!**       | –                                    |
| OUT1 (13) | Motor Phase A           | –                                    |
| OUT2 (12) | Motor Phase B           | –                                    |
| OUT3 (11) | Motor Phase C           | –                                    |
