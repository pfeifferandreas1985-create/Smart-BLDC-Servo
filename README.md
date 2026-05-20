# Smart BLDC Servo

Ein intelligenter, kompakter Closed-Loop BLDC-Servo-Controller basierend auf:
- **ESP32-C3 Super Mini**
- **SimpleFOC Mini v1.0 (DRV8313)**
- **AS5600 Magnet-Encoder** (I2C)
- **Feetech STS/SCS Emulation** (Half-Duplex 1-Wire UART, 1 Mbps)

Dieses Projekt ist die Weiterentwicklung des `SmartServoStepperV2` und ersetzt den lauten Stepper-Motor durch einen leisen, effizienten Brushless DC (BLDC) Motor (z. B. Gimbal Motoren).

---

## 🛠 Aktueller Entwicklungsstand

Die Hardware-Dokumentation (Schaltplan-Review, detaillierter Verdrahtungsplan, BOM) ist in den Markdown-Dateien `01` bis `06` im Hauptverzeichnis ausführlich dokumentiert.

### Open-Loop Test Tool (NEU!)
Im Verzeichnis `software/OpenLoopTest/` befindet sich das erste Testprogramm. Dieses Tool dient dazu, die grundlegende Hardware (ESP32-C3 + DRV8313 Treiber + BLDC Motor) sicher zu testen, **bevor** Encoder oder Feetech-Bus angeschlossen werden.

**Funktionen des Test-Tools:**
1. **WLAN-Web-Dashboard:** Steuerung des Motors über eine dunkle, professionelle Web-Oberfläche.
2. **Sicherer Open-Loop Modus:** Der Motor läuft im `velocity_openloop` Modus von SimpleFOC. Das Spannungs-Limit ist extrem konservativ gesetzt (2.5V), damit auch Motoren mit niedrigem Widerstand beim "Blockieren" nicht überhitzen.
3. **Hardware-Schutz:** Der Fehler-Pin (`nFLT`) des DRV8313 wird aktiv in Echtzeit vom ESP32-C3 überwacht. Bei Überstrom/Übertemperatur schaltet der ESP32 den Treiber sofort ab.
4. **Alignment:** Beim Start führt der Code automatisch ein elektrisches Alignment (2 rad/s) aus.

**Nutzung:**
1. Öffnen Sie `OpenLoopTest.ino` in der Arduino IDE und flashen Sie es auf den ESP32-C3 (Achtung: `USB CDC On Boot` muss auf `Enabled` stehen!).
2. Verbinden Sie sich mit dem ESP32 über Ihr Netzwerk.
3. Öffnen Sie `software/OpenLoopTest/gui/index.html` lokal im Browser.
4. Geben Sie die IP des ESP32 ein, klicken Sie auf *MOTOR AKTIVIEREN* und regeln Sie die Geschwindigkeit über den Slider.

---

## 🗂 Verzeichnisstruktur

- `software/` - Enthält Firmware (wie z.B. das OpenLoopTest-Tool)
- `software/OpenLoopTest/gui/` - Web-Dashboard für das Test-Tool
- `01_...` bis `06_...` - Ausführliche Baudokumentationen und Pin-Mappings
- `*.kicad_*` - Die KiCad 10.0 Projektdateien für das Hardware-Design
