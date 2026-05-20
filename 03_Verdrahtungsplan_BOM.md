# Verdrahtungsplan & Bill of Materials (BOM)
**Projekt: Smart BLDC Servo (2208 80T)**

Dieses Dokument enthält den detaillierten Verdrahtungsplan, die exakten Pin-Belegungen aller Module, die Liste der Schutzbeschaltungen sowie die vollständige Stückliste (BOM) unter Einbeziehung aller Schutzschaltungen für maximalen Betriebsschutz.

---

## 1. Bill of Materials (BOM) - Stückliste

| Menge | Bauteil / Modul | Spezifikation / Wert | Funktion / Einsatzzweck |
| :--- | :--- | :--- | :--- |
| 1x | **ESP32-C3 Super Mini** | 3.3V Logik, WLAN/BT | Haupt-Mikrocontroller, Regelkreis & Kommunikation |
| 1x | **Simple FOC Mini v1.0** | 3-PWM, max 24V/5A | BLDC Motortreiber für FOC Steuerung |
| 1x | **Gimbal Motor 2208** | 80T, typisch 11 Polpaare | Der BLDC Motor selbst (Aktor) |
| 1x | **AS5600 Encoder** | I2C, 12-Bit Auflösung | Absoluter Magnet-Encoder für Winkelmessung |
| 1x | **MP1584EN Step-Down** | Einstellbar (auf 5.0V justieren) | Wandelt 12V Motorenspannung auf 5V für die Logik |
| 1x | **BSS138 Transistor** | N-Channel MOSFET | Unidirektionaler Level Shifter für den TX-Pfad |
| 1x | **TVS-Diode** | **SMBJ24A** (oder SMBJ15A bei 12V) | Transientenschutz (Flyback-Schutz) am VMOT-Eingang |
| 1x | Elektrolytkondensator | **1000 µF / 25V** (Low ESR) | VMOT Hauptpufferung am Simple FOC Mini |
| 1x | Elektrolytkondensator | **100 µF / 10V** (oder höher) | MCU 5V-Eingangspufferung gegen Spannungseinbrüche |
| 1x | Keramikkondensator | **47 µF** (X7R, 1206, ≥10V) | MCU 5V-Eingang Logik-Entkopplung |
| 1x | Keramikkondensator | **22 µF** (X7R, 0805/1206) | MP1584EN Buck-Converter Ausgangsglättung |
| 2x | Keramikkondensator | **10 µF** | 1x nahe AS5600 VCC, 1x nahe MP1584EN Eingang |
| 2x | Keramikkondensator | **0.1 µF** | 1x parallel zu VMOT Elko, 1x direkt am ESP32 5V-Pin |
| 2x | Widerstand | **4.7 kΩ** | I2C Fast-Mode Pull-Ups (SDA/SCL) an 3.3V & RX-Teiler (Serie) |
| 4x | Widerstand | **10 kΩ** | 1x EN Pull-down, 2x nSLP/nRST Pull-ups, 1x nFLT Pull-up & RX-Teiler (GND) |
| 1x | Widerstand | **2.2 kΩ** | Feetech 5V-Bus Terminierung (Pull-up gegen 5V) |
| 1x | Magnet | Diametral magnetisiert | Für den AS5600-Sensor (auf Motorwelle kleben) |

---

## 2. Modul-Verdrahtungstabellen (Pinout)

### ⚡ Stromversorgung & Pufferkondensatoren (Power Grid)
| Quelle / Modul | Pin | Ziel / Modul | Pin | Anmerkung |
| :--- | :--- | :--- | :--- | :--- |
| **12V Netzteil** | `V+ (12V)` | Simple FOC Mini | `VMOT` | Hauptleitung. `SMBJ24A` (TVS), `1000µF` (Elko) & `0.1µF` (Keramik) parallel gegen GND schalten! |
| **12V Netzteil** | `GND` | Simple FOC Mini | `GND` | Leistungs-Masse |
| **12V Netzteil** | `V+ (12V)` | MP1584EN | `IN+` | Eingang für Step-Down (lokal mit `10µF` puffern) |
| **12V Netzteil** | `GND` | MP1584EN | `IN-` | Masse für Step-Down |
| MP1584EN | `OUT+ (5V)` | ESP32-C3 | `5V` | ESP32 5V-Pin (lokal mit `100µF` + `47µF` + `100nF` gegen GND puffern!). `22µF` direkt an MP1584EN Ausgang. |
| MP1584EN | `OUT+ (5V)` | Feetech Bus | `5V (Red)` | Spannungsversorgung für externe Busteilnehmer (falls benötigt) |
| MP1584EN | `OUT- (GND)`| System GND | `GND` | Sternpunkt-Masse. Alle GNDs (ESP, Shifter, AS5600) verbinden. |

### 🧠 ESP32-C3 Super Mini -> Simple FOC Mini (Motorsteuerung & Schutz)
| ESP32-C3 Pin | Funktion | Simple FOC Mini Pin | Anmerkung / Externe Beschaltung |
| :--- | :--- | :--- | :--- |
| `GPIO 0` | PWM A | `IN1` | Phase A Steuerung |
| `GPIO 1` | PWM B | `IN2` | Phase B Steuerung |
| `GPIO 2` | PWM C | `IN3` | Phase C Steuerung |
| `GPIO 3` | Enable | `EN` | Schaltet den Treiber an/aus (**10 kΩ Pull-Down** gegen GND zwingend!) |
| `GPIO 4` | Fault | `nFLT` | Fehler-Feedback (**10 kΩ Pull-Up** gegen 3.3V zwingend!) |
| - | - | `nSLP` | Schlafeingang (über **10 kΩ Pull-Up** an 3.3V daueraktiv geschaltet) |
| - | - | `nRST` | Reseteingang (über **10 kΩ Pull-Up** an 3.3V daueraktiv geschaltet) |
| `3.3V` (Ausgang)| Logik-Power | `3.3V` (Pin 2) | **Unbeschaltet lassen!** Interner LDO hat nur 10mA. |
| `GND` | Masse | `GND` | Signal-Masse |

### 👁️ ESP32-C3 Super Mini -> AS5600 (Sensor)
| ESP32-C3 Pin | Funktion | AS5600 Pin | Anmerkung |
| :--- | :--- | :--- | :--- |
| `3.3V` (Ausgang)| Logik-Power | `VCC` | Der AS5600 läuft auf 3.3V (lokal mit `10µF` entkoppeln) |
| `GND` | Masse | `GND` | Masse |
| `GPIO 8` | I2C SDA | `SDA` | Datenleitung (**4.7 kΩ Pull-Up** an 3.3V) |
| `GPIO 9` | I2C SCL | `SCL` | Clock-Leitung (**4.7 kΩ Pull-Up** an 3.3V) |

### 🔌 ESP32-C3 Super Mini -> Feetech Bus (Split-Path Level Shifter)
| ESP32-C3 Pin | Funktion | Signalweg / Level Shifter | Anmerkung / Externe Beschaltung |
| :--- | :--- | :--- | :--- |
| `GPIO 21` | UART TX | → BSS138 Source | **TX-Pfad (3.3V → 5V):** BSS138 Gate an 3.3V. BSS138 Drain geht an die 5V Bus-Datenleitung. |
| `GPIO 20` | UART RX | ← Spannungsteiler | **RX-Pfad (5V → 3.3V):** Von der 5V Bus-Datenleitung über 4.7 kΩ in Serie an GPIO 20. GPIO 20 über 10 kΩ gegen GND schalten. |
| `3.3V` (Ausgang)| Logik-Referenz| BSS138 Gate / VIO | 3.3V Referenzspannung |
| `GND` | Masse | GND | Gemeinsame Masse |

### 🌐 Physische Bus-Terminierung (Am Ausgangsstecker)
| Bus-Leitung | Pin | Beschaltung | Anmerkung |
| :--- | :--- | :--- | :--- |
| `5V (VCC)` | Pin 1 | - | Stromversorgung des Busses |
| `DATA (5V)` | Pin 2 | **2.2 kΩ Pull-Up** gegen 5V | **Bus-Terminierung:** Zwingend erforderlich für scharfe Signalflanken bei 1 Mbps |
| `GND` | Pin 3 | - | Masse-Referenz |

---

## 3. Schutzbeschaltung (Detaillierte Einbauorte)

1. **VMOT Flyback Puffer & TVS (am Simple FOC Mini):**
   * **Bauteile:** `1000 µF / 25V` Low-ESR Elko + `0.1 µF` Keramik + TVS-Diode `SMBJ24A` (parallel).
   * **Einbauort:** Unmittelbar zwischen den Schraubklemmen/Pads von `VMOT` und `GND` des Simple FOC Mini.
   * **Warum?** Verhindert induktive Spannungsspitzen durch die Generatorwirkung des BLDC-Motors bei schnellen Richtungswechseln.

2. **MCU-Glättung (ESP32-C3 Brownout-Schutz):**
   * **Bauteile:** `100 µF` Elko + `47 µF` Keramikkondensator (X7R, 1206) + `100 nF` Keramik (parallel).
   * **Einbauort:** Direkt am `5V`-Pin und dem nahen `GND`-Pin des ESP32-C3 Super Mini.
   * **Warum?** Filtert hochfrequente Störsignale des Treibers heraus und puffert Stromspitzen des ESP32-C3 beim Senden über WLAN oder Bus.

3. **I2C Fast-Mode Pull-Ups (für den AS5600):**
   * **Bauteile:** 2x `4.7 kΩ` Widerstände.
   * **Einbauort:** Einer von `GPIO 8 (SDA)` an `3.3V`, einer von `GPIO 9 (SCL)` an `3.3V`.
   * **Warum?** Die internen Pull-Ups des ESP32-C3 sind zu schwach für 400kHz. Ohne diese Widerstände kommt es zu Sensor-Aussetzern (Rauschen).

4. **Boot-Safe Pull-Down (am Simple FOC Mini):**
   * **Bauteil:** 1x `10 kΩ` Widerstand.
   * **Einbauort:** Zwischen `EN` Pin (Pin 9) des Simple FOC Mini und `GND`.
   * **Warum?** Hält den Motor beim Booten oder Flashen der MCU sicher ausgeschaltet, da der Treiber-Pin Active-HIGH ist und der ESP32-Pin beim Start floaten kann.

5. **nSLP/nRST Treiber-Aktivierung:**
   * **Bauteile:** 2x `10 kΩ` Widerstände.
   * **Einbauort:** Jeweils einer von `nSLP` (Pin 8) und `nRST` (Pin 6) des Simple FOC Mini gegen `3.3V`.
   * **Warum?** Der DRV8313 schaltet sich ohne dieses HIGH-Signal dauerhaft stumm (interner Pull-down).

6. **nFLT Überlastungsschutz:**
   * **Bauteil:** 1x `10 kΩ` Widerstand.
   * **Einbauort:** Zwischen `nFLT` (Pin 10) des Simple FOC Mini und `3.3V`. Zudem direkte Verbindung an `GPIO 4` des ESP32.
   * **Warum?** Hält das Fehlersignal auf HIGH. Zieht der Treiber bei Überlast `nFLT` auf LOW, schaltet die Firmware sofort den Motor ab.
