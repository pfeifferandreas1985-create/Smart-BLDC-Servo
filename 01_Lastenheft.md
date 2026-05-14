# Projekt: Smart BLDC Servo – Lastenheft

## 1. Zielbeschreibung
Entwicklung eines intelligenten, Closed-Loop gesteuerten BLDC-Servos auf Basis eines Gimbal-Motors (2208 80T) und ESP32-C3 Super Mini. Das System integriert einen Simple FOC Mini v1.0 Treiber und einen AS5600 Encoder für präzises Positions-, Geschwindigkeits-, und Drehmoment-Feedback mittels Field Oriented Control (FOC).
Es soll sich am Datenbus exakt wie ein Feetech STS/SCS Servomotor ansprechen lassen.

## 2. Hardware-Spezifikationen
*   **Zentrale Logik:** ESP32-C3 Super Mini.
*   **Motor:** BLDC 2208 80T (Gimbal Motor, typischerweise 11 Polpaare).
*   **Treiber:** Simple FOC Mini v1.0 (Unterstützt bis 24V, max 5A).
*   **Sensorik:** AS5600 absoluter Magnet-Encoder (12-Bit, I2C, 4096 Schritte).
*   **Bus-Interface:** 5V Level Shifter + 220Ω Widerstand für 5V Half-Duplex Kommunikation (Feetech-Stil).
*   **Power Management:** Hauptspannung (z.B. 12V); MP1584 Step-Down Converter auf 5V für den ESP32-C3 und die High-Side des Level Shifters.

## 3. Betriebsmodi (Control Modes)
Das System nutzt die SimpleFOC Bibliothek und unterstützt folgende Modi:
1.  **Position Control:** Präzise Anfahrt der Zielposition (Winkel).
2.  **Velocity Control:** Konstante Drehzahl.
3.  **Torque Control (Voltage-based):** Direkte Vorgabe der Spannung/des Drehmoments.

## 4. Software Features & Fail-Safe
*   **SimpleFOC Integration:** Ruckelfreie und leise Steuerung mittels Sinus-Kommutierung.
*   **Closed-Loop:** Laufende Positionskorrektur.
*   **Boot-Safe State:** Pull-Down Widerstand am Enable-Pin, um den Motor während des Bootvorgangs stromlos zu halten.
*   **Kommunikation:** UART über Level-Shifter (Half-Duplex) zum Auslesen und Setzen von Zielwerten.
