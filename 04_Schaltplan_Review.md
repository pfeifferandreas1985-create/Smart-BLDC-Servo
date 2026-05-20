# 04. Schaltplan- & Schutzschaltungs-Review (SmartBLDCServo)

Dieses Dokument enthält das technische Review des Schaltplans und der Schutzschaltungen für das Projekt **SmartBLDCServo** auf Basis des Gimbal-Motors (2208 80T) und des **SimpleFOC Mini v1.0**-Treibers. Die Schaltung wurde im Vergleich zu den Freigabestandards des Vorgängerprojekts **SmartServoStepperV2** geprüft.

---

## 1. Kritische Fehler (Müssen vor dem PCB-Layout behoben werden)

### 1.1 Unbeschaltete nSLP und nRST Pins am SimpleFOC Mini (Fehlende Aktivierung)
*   **Problem:** Der DRV8313-Treiberchip auf dem SimpleFOC Mini besitzt interne **100 kΩ Pull-Down-Widerstände** an den Steuereingängen. Die Pins `nSLP` (Sleep, Pin 8) und `nRST` (Reset, Pin 6) sind **Active-LOW**. Wenn diese Pins in der Schaltung unbeschaltet (floating) bleiben, zieht der Chip sie auf GND und verbleibt dauerhaft im Sleep- und Reset-Zustand. Der Treiber wird keine PWM-Signale annehmen und der Motor bleibt stromlos.
*   **Lösung:** Verbinden Sie sowohl `nSLP` als auch `nRST` mit **3.3V** (entweder direkt oder jeweils über einen **10 kΩ Pull-Up-Widerstand**). Dadurch wird der Treiber permanent aktiviert.
*   **Hinweis:** Der `3.3V` Pin auf dem SimpleFOC Mini Header (Pin 2) ist der Ausgang eines chipinternen LDO-Reglers (max. 10 mA). Sie können die Pins `nSLP`/`nRST` entweder mit diesem Pin 2 oder direkt mit der 3.3V-Schiene des ESP32-C3 verbinden (empfohlen, da stabiler).

### 1.2 Fehlende Glättungs- und Pufferkondensatoren an den Logikschienen (5V & 3.3V)
*   **Problem:** In der aktuellen BOM und dem Verdrahtungsplan fehlen jegliche Abblockkondensatoren an den Spannungsreglern und MCU-Eingängen. BLDC-Motoren erzeugen durch das schnelle Schalten der Phasen starke Störungen (Rippel) auf der Versorgungsspannung. Ohne Pufferung wird der ESP32-C3 beim Anlauf des Motors durch Spannungsabfälle (Brownouts) abstürzen oder unvorhersehbares Verhalten zeigen.
*   **Lösung (analog zum SmartServoStepperV2-Standard):**
    *   **MP1584EN Ausgang (5V Rail):** Platzieren Sie mindestens einen **22 µF Keramikkondensator** (X7R, 1206) direkt am Ausgang des Reglers.
    *   **ESP32-C3 5V Input:** Platzieren Sie eine Kombination aus **100 µF Elko** + **47 µF Keramikkondensator** (X7R, 1206, ≥16V) + **100 nF Keramikkondensator** so nah wie möglich am 5V-Pin des ESP32.
    *   **AS5600 Sensor (3.3V Rail):** Platzieren Sie einen **10 µF Keramikkondensator** direkt an den VCC- und GND-Pins des Sensors.

---

## 2. Dringende Empfehlungen zur Stabilitäts-Optimierung

### 2.1 Feetech-Bus Level Shifter: Bidirektional vs. Unidirektionaler Split-Pfad (1 Mbps Stabilität)
*   **Problem:** Die Schaltung sieht den kombinierten 1-Wire UART mit dem bidirektionalen Level Shifter BSS138 (`LV1` zu `HV1` mit 220 Ω Schutzwiderstand) vor. Bei der hohen Übertragungsrate des Feetech-Busses von **1 Mbps** führt die hohe Gate-Kapazität und der langsame Pull-Up-Übergang des BSS138 zu einer starken Verrundung der Signalflanken. Dies führt im Betrieb häufig zu Frame-Fehlern.
*   **Lösung aus SmartServoStepperV2 V1.6:**
    Nutzen Sie die freien GPIOs des ESP32-C3 und teilen Sie den Pfad in einen unidirektionalen TX-Pfad und einen RX-Spannungsteiler-Pfad auf. Diese werden erst auf der 5V-Seite zusammengeführt:
    1.  **TX-Pfad:** ESP32 TX (`GPIO 21`) → Gate/Source des BSS138 (Pegelwandlung 3.3V → 5V) → 5V-Bus.
    2.  **RX-Pfad:** 5V-Bus → Spannungsteiler (**4.7 kΩ** in Serie, **10 kΩ** gegen GND) → ESP32 RX (`GPIO 20`).
    3.  **Terminierung:** Ein starker **2.2 kΩ Pull-Up-Widerstand** vom 5V-Bus gegen 5V sorgt für scharfe Signalflanken.
*   **Vorteil:** Durch diese Trennung muss der Level Shifter nur unidirektional arbeiten, was die Signalqualität bei 1 Mbps massiv erhöht. Da beim ESP32-C3 die GPIOs 4, 5, 6, 7 und 10 ungenutzt sind, steht dieser Änderung nichts im Weg.

### 2.2 Fehlender TVS-Schutz am VMOT-Eingang (Flyback-Überspannungsschutz)
*   **Problem:** Der 1000 µF Elko schützt vor kurzzeitigen Rückspeisungen, fängt jedoch keine hochenergetischen transienten Überspannungen ab, die entstehen, wenn der Motor blockiert oder abrupt gestoppt wird. Diese Spannungsspitzen können den SimpleFOC Mini sowie den MP1584 Buck-Converter zerstören.
*   **Lösung:** Schalten Sie eine **TVS-Diode** (Transienten-Absorptionsdiode) vom Typ **SMBJ24A** (bei 12V-24V Betrieb) oder **SMBJ15A** (bei reinem 12V Betrieb) parallel zum 1000 µF Kondensator direkt an den VMOT-Eingang (Kathode an VMOT, Anode an GND).

---

## 3. Optionale Komfort-Features

### 3.1 Nutzung des Fehler-Pins (nFLT)
*   **Vorteil:** Der SimpleFOC Mini hat einen Open-Drain Ausgang `nFLT` (Pin 10), der im Fehlerfall (Übertemperatur, Überstrom am DRV8313) auf LOW gezogen wird.
*   **Empfehlung:** Verbinden Sie `nFLT` über einen **10 kΩ Pull-Up-Widerstand** gegen 3.3V mit einem freien ESP32 GPIO (z.B. `GPIO 4`). Ihre Firmware kann dadurch Fehler sofort erkennen, den Motor stoppen und eine Fehlermeldung über den Bus senden.

---

## 4. Korrekt und sinnvoll umgesetzte Schaltungsteile (Best Practice)

*   ✔ **Boot-Safe Enable State:** Der **10 kΩ Pull-Down-Widerstand** am `EN`-Pin (Pin 9) des SimpleFOC Mini ist absolut korrekt. Da der Treiber-Enable-Pin Active-HIGH is, hält dieser Widerstand den Motor beim Booten der MCU sicher stromlos, solange die GPIOs schwimmen.
*   ✔ **I2C Pull-Ups:** Die beiden **4.7 kΩ Pull-Up-Widerstände** an SDA/SCL gegen 3.3V sind für den stabilen Betrieb des AS5600 im 400 kHz Fast Mode ideal dimensioniert.
*   ✔ **Softwareseitige Sicherheitslimits:** Die Reduzierung des Spannungslimits im Code (`driver.voltage_limit = 6.0;` bei `driver.voltage_power_supply = 12.0;`) ist sehr sinnvoll für Gimbal-Motoren, da diese einen hohen Phasenwiderstand aufweisen und bei vollen 12V schnell überhitzen würden.

---

## 5. Empfohlenes optimiertes Pin-Mapping

Unter Berücksichtigung der Optimierungen ergibt sich folgende Empfehlung für die Pin-Belegung des ESP32-C3:

| Pin | Funktion | Richtung | Anbindung / Hardware-Schutz |
| :--- | :--- | :--- | :--- |
| **GPIO 0** | PWM Phase A | Ausgang | Direkt an `IN1` des SimpleFOC |
| **GPIO 1** | PWM Phase B | Ausgang | Direkt an `IN2` des SimpleFOC |
| **GPIO 2** | PWM Phase C | Ausgang | Direkt an `IN3` des SimpleFOC |
| **GPIO 3** | Driver Enable | Ausgang | An `EN` des SimpleFOC (mit 10 kΩ Pull-Down an GND) |
| **GPIO 4** | Driver Fault (nFLT)| Eingang | An `nFLT` des SimpleFOC (mit 10 kΩ Pull-Up an 3.3V) |
| **GPIO 8** | I2C SDA | Bidirektional | An AS5600 SDA (mit 4.7 kΩ Pull-Up an 3.3V) |
| **GPIO 9** | I2C SCL | Ausgang | An AS5600 SCL (mit 4.7 kΩ Pull-Up an 3.3V) |
| **GPIO 20**| Feetech Bus RX | Eingang | Über 5V→3.3V Spannungsteiler (4.7 kΩ / 10 kΩ) an den Bus |
| **GPIO 21**| Feetech Bus TX | Ausgang | Über Level Shifter (BSS138) an den 5V-Bus |
