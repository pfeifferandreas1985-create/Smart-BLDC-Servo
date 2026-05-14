# Verdrahtungsplan & Bill of Materials (BOM)
**Projekt: Smart BLDC Servo (2208 80T)**

Dieses Dokument enthält den detaillierten Verdrahtungsplan, die exakten Pin-Belegungen aller Module, die Liste der Schutzbeschaltungen sowie die vollständige Stückliste (BOM).

---

## 1. Bill of Materials (BOM) - Stückliste

| Menge | Bauteil / Modul | Spezifikation / Wert | Funktion / Einsatzzweck |
| :--- | :--- | :--- | :--- |
| 1x | **ESP32-C3 Super Mini** | 3.3V Logik, WLAN/BT | Haupt-Mikrocontroller, Regelkreis & Kommunikation |
| 1x | **Simple FOC Mini v1.0** | 3-PWM, max 24V/5A | BLDC Motortreiber für FOC Steuerung |
| 1x | **Gimbal Motor 2208** | 80T, typisch 11 Polpaare | Der BLDC Motor selbst (Aktor) |
| 1x | **AS5600 Encoder** | I2C, 12-Bit Auflösung | Absoluter Magnet-Encoder für Winkelmessung |
| 1x | **MP1584 Step-Down** | Einstellbar (auf 5.0V justieren) | Wandelt 12V Motorenspannung auf 5V für die Logik |
| 1x | **Level Shifter (Bi-Dir)**| 4-Kanal (BSS138 Basis) | Pegelwandler zwischen 3.3V (ESP32) und 5V (Bus) |
| 1x | Elektrolytkondensator | **1000 µF / 25V** (Low ESR) | VMOT Pufferung (Flyback-Schutz) am Simple FOC Mini |
| 1x | Keramikkondensator | **0.1 µF** | Hochfrequenz-Filterung parallel zum 1000µF Elko |
| 2x | Widerstand | **4.7 kΩ** | I2C Pull-Up Widerstände (SDA/SCL) für 400kHz Mode |
| 1x | Widerstand | **10 kΩ** | Boot-Safe Pull-Down Widerstand am EN-Pin |
| 1x | Widerstand | **220 Ω** | Kurzschluss-/Strombegrenzung für den Feetech-Bus |
| 1x | Magnet | Diametral magnetisiert | Für den AS5600 Sensor (auf Motorwelle kleben) |

---

## 2. Modul-Verdrahtungstabellen (Pinout)

### ⚡ Stromversorgung (Power Routing)
| Quelle / Modul | Pin | Ziel / Modul | Pin | Anmerkung |
| :--- | :--- | :--- | :--- | :--- |
| **12V Netzteil** | `V+ (12V)` | Simple FOC Mini | `VMOT` | Motorenstromversorgung (Kondensatoren parallel hier anlegen!) |
| **12V Netzteil** | `GND` | Simple FOC Mini | `GND` | Gemeinsame Masse |
| **12V Netzteil** | `V+ (12V)` | MP1584 Step-Down | `IN+` | Eingang für Spannungsregler |
| **12V Netzteil** | `GND` | MP1584 Step-Down | `IN-` | Masse für Spannungsregler |
| MP1584 Step-Down | `OUT+ (5V)` | ESP32-C3 Super Mini | `5V` | ESP32-C3 generiert intern 3.3V daraus |
| MP1584 Step-Down | `OUT+ (5V)` | Level Shifter | `HV` (High Voltage) | 5V Referenz für den Bus |
| MP1584 Step-Down | `OUT- (GND)`| System GND | `GND` | Alle GNDs (ESP, Shifter, AS5600) verbinden |

### 🧠 ESP32-C3 Super Mini -> Simple FOC Mini (Motorsteuerung)
| ESP32-C3 Pin | Funktion | Simple FOC Mini Pin | Anmerkung |
| :--- | :--- | :--- | :--- |
| `GPIO 0` | PWM A | `IN1` | Phase A Steuerung |
| `GPIO 1` | PWM B | `IN2` | Phase B Steuerung |
| `GPIO 2` | PWM C | `IN3` | Phase C Steuerung |
| `GPIO 3` | Enable | `EN` | Schaltet den Treiber an/aus |
| `GND` | Masse | `GND` | Muss verbunden sein |

### 👁️ ESP32-C3 Super Mini -> AS5600 (Sensor)
| ESP32-C3 Pin | Funktion | AS5600 Pin | Anmerkung |
| :--- | :--- | :--- | :--- |
| `3.3V` (Ausgang) | Power | `VCC` | Der AS5600 läuft auf 3.3V |
| `GND` | Masse | `GND` | |
| `GPIO 8` | I2C SDA | `SDA` | Datenleitung (4.7kΩ Pull-Up an 3.3V erforderlich) |
| `GPIO 9` | I2C SCL | `SCL` | Clock-Leitung (4.7kΩ Pull-Up an 3.3V erforderlich) |
| - | - | `DIR` | Optional: An GND für CW, an 3.3V für CCW Laufrichtung |

### 🔌 ESP32-C3 Super Mini -> Level Shifter (Feetech Bus)
| ESP32-C3 Pin | Funktion | Level Shifter | Anmerkung |
| :--- | :--- | :--- | :--- |
| `3.3V` (Ausgang) | Power | `LV` (Low Voltage) | 3.3V Referenzspannung für den ESP32 |
| `GND` | Masse | `GND` | |
| `GPIO 20` | UART RX | `LV1` | Empfangsleitung (direkt mit LV1 verbunden) |
| `GPIO 21` | UART TX | `LV1` | **Achtung:** Über `220Ω Widerstand` an denselben Pin `LV1` anschließen! |

### 🌐 Level Shifter -> Feetech Bus (Extern)
| Level Shifter | Funktion | Externer Bus | Anmerkung |
| :--- | :--- | :--- | :--- |
| `HV1` | UART (5V) | `DATA` Leitung | `HV1` geht direkt als Single-Wire-Bus nach draußen |
| `HV` | Power | `5V` (Extern) | Wenn andere Servos am Bus sind, 5V-GND Level mitschleifen |

*Hinweis: Da RX und TX bereits auf der LV-Seite (3.3V) über den 220Ω Widerstand auf `LV1` zusammengeführt werden, wird nur ein Kanal (`LV1` zu `HV1`) des Level Shifters benötigt.*

---

## 3. Schutzbeschaltung (Zwingend erforderlich!)

1. **VMOT Flyback Puffer (am Simple FOC Mini):**
   * **Bauteile:** `1000 µF / 25V` Elko + `0.1 µF` Keramikkondensator (Parallel).
   * **Einbauort:** Direkt zwischen den Anschlüssen `VMOT` und `GND` am Simple FOC Mini.
   * **Warum?** Fängt gefährliche Spannungsspitzen ab, wenn der BLDC Motor abrupt bremst.

2. **I2C Fast-Mode Pull-Ups (für den AS5600):**
   * **Bauteile:** 2x `4.7 kΩ` Widerstände.
   * **Einbauort:** Einer verbindet `GPIO 8 (SDA)` mit `3.3V`, der andere verbindet `GPIO 9 (SCL)` mit `3.3V`.
   * **Warum?** Die internen Pull-Ups des ESP32-C3 sind zu schwach für 400kHz. Ohne diese Widerstände kommt es zu Sensor-Aussetzern (Rauschen).

3. **Boot-Safe Pull-Down (am Simple FOC Mini):**
   * **Bauteil:** 1x `10 kΩ` Widerstand.
   * **Einbauort:** Zwischen `EN` Pin des Simple FOC Mini und `GND`.
   * **Warum?** Der Simple FOC Treiber ist "Active High". Während der ESP32-C3 bootet, schwimmen die Pins. Dieser Widerstand zwingt den Motor auf "Aus", bis der ESP komplett hochgefahren ist.

4. **Bus-Kurzschluss-Schutz (Feetech 1-Wire):**
   * **Bauteil:** 1x `220 Ω` Widerstand.
   * **Einbauort:** Auf der 3.3V LV-Seite zwischen dem ESP32-C3 TX-Pin und dem Knotenpunkt zur RX-Leitung vor dem Level Shifter. (ESP32 TX -> 220Ω Widerstand -> Verbindung mit ESP32 RX -> Level Shifter LV-Eingang).
   * **Warum?** Begrenzt den Strom und verhindert einen Kurzschluss, wenn der ESP32 sendet (TX = HIGH), während gleichzeitig ein anderes Gerät auf dem Bus sendet.
