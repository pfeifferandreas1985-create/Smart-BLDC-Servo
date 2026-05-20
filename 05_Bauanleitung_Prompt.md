# 05. Bauanleitung & Detail-Prompt für Schaltplanerstellung

Dieses Dokument enthält einen präzisen, detaillierten Prompt, der das gesamte Schaltungsdesign und die Verdrahtung so beschreibt, dass eine externe Person (oder eine KI wie Claude/GPT) den Schaltplan und das PCB-Layout fehlerfrei und exakt nachbauen kann.

---

## Kopierbarer Prompt für Schaltplan und Layout (Deutsch)

```text
Erstelle einen vollständigen, professionellen Schaltplan für einen intelligenten Closed-Loop BLDC-Servomotor (Projektname: SmartBLDCServo). Die Schaltung basiert auf einem ESP32-C3 Super Mini Mikrocontroller, einem Simple FOC Mini v1.0 Treiberboard (DRV8313), einem AS5600 Magnetsensor (I2C) und einem 1 Mbps Feetech-kompatiblen Half-Duplex Eindraht-Bus.

Befolge exakt die folgenden Verdrahtungsvorgaben und Schutzbeschaltungen:

### 1. STROMVERSORGUNG & SPANNUNGSGLÄTTUNG (POWER GRID)
- HAUPTVERSORGUNG (12V):
  * Eingangsklemmen für 12V Hauptspannung (VMOT) und GND.
  * Schalte direkt parallel am 12V-Eingang des Simple FOC Mini folgende Bauteile zum Schutz vor Flyback-Spannungsspitzen:
    1. Eine TVS-Diode vom Typ SMBJ24A (oder SMBJ15A) parallel (Kathode an VMOT, Anode an GND).
    2. Einen 1000 µF / 25V Low-ESR Elektrolytkondensator.
    3. Einen 0.1 µF (100 nF) Keramikkondensator.

- DEZENTRALE SPANNUNGSREGELUNG (12V auf 5V):
  * Nutze einen MP1584EN Buck-Converter.
  * Eingang: VMOT (12V) und GND (lokal gepuffert mit einem 10 µF Keramikkondensator).
  * Ausgang: Auf 5.0V einstellen. Schalte einen 22 µF Keramikkondensator direkt parallel an den 5V-Ausgang.
  * Die 5V-Schiene versorgt den ESP32-C3 "5V" Pin und die High-Side (HV) des Feetech-Busses.

- MCU-GLÄTTUNG & ENTSTATTUNG (ESP32-C3 5V Input):
  * Schalte direkt am 5V-Eingangspin des ESP32-C3 Super Mini eine Kondensatorbatterie gegen GND, um MCU-Brownouts zu verhindern:
    1. Einen 100 µF Elektrolytkondensator (Low ESR).
    2. Einen 47 µF Keramikkondensator (X7R, Bauform 1206, Nennspannung ≥16V).
    3. Einen 100 nF Keramikkondensator zur HF-Filterung.

- LOGIKSPANNUNG (3.3V):
  * Die 3.3V-Logikspannung wird vom internen LDO-Regler des ESP32-C3 erzeugt (3.3V Pin des ESP32).
  * Der 3.3V-Ausgangspin des Simple FOC Mini bleibt komplett UNBESCHALTET.
  * Schalte nahe am VCC/GND-Eingang des AS5600 Encoders einen 10 µF Keramikkondensator gegen GND.

---

### 2. SIGNALVERDRAHTUNG (PIN-MAPPING)

- ESP32-C3 zu SIMPLE FOC MINI (DRV8313):
  * ESP32 GPIO 0 -> Simple FOC IN1 (PWM Phase A)
  * ESP32 GPIO 1 -> Simple FOC IN2 (PWM Phase B)
  * ESP32 GPIO 2 -> Simple FOC IN3 (PWM Phase C)
  * ESP32 GPIO 3 -> Simple FOC EN (Treiber-Aktivierung). Schalte einen 10 kΩ Widerstand als Pull-Down gegen GND (sichert, dass der Treiber beim Booten aus bleibt).
  * ESP32 GPIO 4 -> Simple FOC nFLT (Fehler-Ausgang). Schalte einen 10 kΩ Widerstand als Pull-Up gegen 3.3V.
  * Simple FOC nSLP (Sleep-Eingang) -> Über einen 10 kΩ Widerstand als Pull-Up permanent an 3.3V anschließen.
  * Simple FOC nRST (Reset-Eingang) -> Über einen 10 kΩ Widerstand als Pull-Up permanent an 3.3V anschließen.
  * Verbinde die Signal-Masse (GND) des Simple FOC Mini mit der System-GND.

- ESP32-C3 zu AS5600 ENCODER (I2C):
  * ESP32 GPIO 8 -> AS5600 SDA. Schalte einen 4.7 kΩ Widerstand als Pull-Up gegen 3.3V.
  * ESP32 GPIO 9 -> AS5600 SCL. Schalte einen 4.7 kΩ Widerstand als Pull-Up gegen 3.3V.

- ESP32-C3 zu FEETECH 1-WIRE BUS (Split-Path Level Shifter für 1 Mbps):
  Führe den UART1-Sende- und Empfangspfad auf der 3.3V-Seite getrennt und führe sie erst auf der 5V-Busseite zusammen:
  * TX-PFAD (3.3V auf 5V):
    * Verwende einen N-Channel MOSFET BSS138.
    * ESP32 GPIO 21 (UART TX) an Source des BSS138 anschließen.
    * Verbinde das Gate des BSS138 direkt mit 3.3V.
    * Verbinde den Drain des BSS138 mit der externen 5V-Feetech-Datenleitung (TTL_1_Wire).
  * RX-PFAD (5V auf 3.3V):
    * Führe das Signal von der 5V-Datenleitung (TTL_1_Wire) über einen Spannungsteiler an den ESP32 RX-Pin.
    * Schalte einen 4.7 kΩ Widerstand in Serie von der 5V-Datenleitung zu ESP32 GPIO 20 (UART RX).
    * Schalte einen 10 kΩ Widerstand von ESP32 GPIO 20 gegen GND.
  * BUS-TERMINIERUNG:
    * Schalte einen starken 2.2 kΩ Widerstand am Ausgangsstecker des Feetech-Busses als Pull-Up von der Datenleitung gegen 5V (sorgt für scharfe steigende Flanken bei 1 Mbps).

### 3. FERTIGUNGS- & LAYOUT-HINWEISE
- Platziere alle Entkopplungskondensatoren (100 nF) so nah wie physisch möglich an den VCC-Pins des jeweiligen ICs (ESP32-C3, AS5600).
- Halte die I2C-Leitungen (SDA/SCL) so kurz wie möglich und führe sie räumlich getrennt von den lauten Motor-Phasenleitungen (Phasen A, B, C) des Simple FOC Mini.
- Erstelle großflächige GND-Flächen auf Top- und Bottom-Layer für ein optimales Wärmemanagement.
```
