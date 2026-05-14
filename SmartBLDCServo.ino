#include <SimpleFOC.h>
#include <Wire.h>

// ==========================================
// PIN KONFIGURATION (ESP32-C3 Super Mini)
// ==========================================
// I2C Pins für AS5600
#define PIN_SDA 8
#define PIN_SCL 9

// Simple FOC Mini PWM Pins
#define PIN_IN1 0
#define PIN_IN2 1
#define PIN_IN3 2
#define PIN_EN  3

// UART Pins für Feetech Half-Duplex Bus (über Level Shifter)
#define PIN_RX 20
#define PIN_TX 21

// ==========================================
// MOTOR & SENSOR KONFIGURATION
// ==========================================
// Gimbal Motor 2208 80T hat typischerweise 11 Polpaare
#define POLE_PAIRS 11

// AS5600 Sensor
MagneticSensorI2C sensor = MagneticSensorI2C(AS5600_I2C);

// BLDC Driver
BLDCDriver3PWM driver = BLDCDriver3PWM(PIN_IN1, PIN_IN2, PIN_IN3, PIN_EN);

// BLDC Motor
BLDCMotor motor = BLDCMotor(POLE_PAIRS);

// ==========================================
// VARIABLEN FÜR FEETECH BUS
// ==========================================
// Target values
float target_value = 0.0;
String command_buffer = "";
unsigned long last_telemetry = 0;

void setup() {
  // 1. Debugging & Web-Dashboard Serial (USB)
  Serial.begin(115200);
  delay(1000);

  // 2. I2C Bus initialisieren (ESP32-C3 Custom Pins)
  Wire.begin(PIN_SDA, PIN_SCL, 400000); // 400 kHz Fast Mode
  sensor.init();
  Serial.println("AS5600 Sensor initialisiert.");

  // 3. Motor & Treiber Setup
  driver.voltage_power_supply = 12.0; // 12V Netzteil
  driver.voltage_limit = 6.0;         // Max 6V an die Spulen (Gimbal Motoren haben hohen Widerstand, aber 6V ist sicher für den Anfang)
  driver.init();
  
  // Link Sensor & Driver to Motor
  motor.linkSensor(&sensor);
  motor.linkDriver(&driver);

  // 4. Control Loop Setup
  motor.voltage_sensor_align = 3.0; // 3V für das initiale Alignement
  motor.foc_modulation = FOCModulationType::SpaceVectorPWM;
  
  // PI Controller für die Position
  motor.controller = MotionControlType::angle;
  motor.P_angle.P = 20.0;
  motor.P_angle.I = 0.0;
  motor.P_angle.D = 0.5;
  motor.P_angle.output_ramp = 10000.0;
  motor.P_angle.limit = 6.0; // Max Voltage
  
  // Velocity Low Pass Filter
  motor.LPF_velocity.Tf = 0.01; // 10ms

  // Motor initialisieren & FOC kalibrieren
  Serial.println("Starte FOC Kalibrierung (Motor nicht blockieren!)...");
  motor.init();
  motor.initFOC();
  Serial.println("FOC Initialisierung abgeschlossen.");

  // 5. Feetech UART Bus (Half-Duplex) Setup
  // Wir verwenden Serial1 für die Kommunikation
  Serial1.begin(1000000, SERIAL_8N1, PIN_RX, PIN_TX);
  Serial.println("Feetech UART Bus lauscht auf 1 Mbps.");

  target_value = motor.sensor_direction * sensor.getAngle();
}

void loop() {
  // 1. FOC Algorithmus (muss so oft wie möglich aufgerufen werden!)
  motor.loopFOC();

  // 2. Motor Bewegung ausführen
  motor.move(target_value);

  // 3. Telemetrie an Web-Dashboard senden (alle 100ms)
  if (millis() - last_telemetry > 100) {
    last_telemetry = millis();
    sendTelemetry();
  }

  // 4. Kommandos vom Web-Dashboard empfangen
  processDashboardCommands();

  // 5. Feetech Bus auslesen (Nicht-blockierend!)
  processFeetechBus();
}

// ==========================================
// WEB DASHBOARD KOMMUNIKATION (JSON via USB)
// ==========================================
void sendTelemetry() {
  // {"pos": 180.5, "rpm": 1200, "volt": 12.0}
  float pos_deg = (motor.shaft_angle / (2.0 * PI)) * 360.0;
  float rpm = (motor.shaft_velocity / (2.0 * PI)) * 60.0;
  float volt = driver.voltage_power_supply; // In real life, measure via ADC or static

  Serial.print("{\"pos\": ");
  Serial.print(pos_deg, 1);
  Serial.print(", \"rpm\": ");
  Serial.print(rpm, 0);
  Serial.print(", \"volt\": ");
  Serial.print(volt, 1);
  Serial.println("}");
}

void processDashboardCommands() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      // Very basic JSON parse for {"mode": "speed", "val": 500}
      if (command_buffer.indexOf("\"mode\":\"speed\"") > 0 || command_buffer.indexOf("\"mode\": \"speed\"") > 0) {
        motor.controller = MotionControlType::velocity;
      } else if (command_buffer.indexOf("\"mode\":\"pos\"") > 0 || command_buffer.indexOf("\"mode\": \"pos\"") > 0) {
        motor.controller = MotionControlType::angle;
      }

      int valIndex = command_buffer.indexOf("\"val\":");
      if (valIndex > 0) {
        float val = command_buffer.substring(valIndex + 6).toFloat();
        if (motor.controller == MotionControlType::velocity) {
          // Input ist U/min, Konvertierung zu rad/s
          target_value = (val / 60.0) * (2.0 * PI);
        } else {
          // Input ist Grad, Konvertierung zu rad
          target_value = (val / 360.0) * (2.0 * PI);
        }
      }

      if (command_buffer.indexOf("\"cmd\":\"zero\"") > 0 || command_buffer.indexOf("\"cmd\": \"zero\"") > 0) {
         sensor.init(); // Re-init setzt den Nullpunkt (bei inkrementell, bei AS5600 bräuchte man Offset)
         motor.sensor_offset = sensor.getAngle();
      }

      command_buffer = "";
    } else {
      command_buffer += c;
    }
  }
}

// ==========================================
// FEETECH KOMMUNIKATIONS-LOGIK (Skeleton)
// ==========================================
void processFeetechBus() {
  // Hier wird das Feetech Protokoll geparst.
  // Das Protokoll basiert auf Paketen: [0xFF, 0xFF, ID, Length, Instruction, Param1, Param2, ..., Checksum]
  
  if (Serial1.available() >= 2) {
    // Einfache Dummy-Logik als Platzhalter für das echte Paket-Parsing
    // Lese Daten aus dem Puffer
    while (Serial1.available()) {
      uint8_t incoming_byte = Serial1.read();
      
      // In einer vollständigen Implementierung würde hier eine State-Machine das 
      // Feetech-Paket auswerten und z.B. einen neuen `target_position` Wert setzen.
      // 
      // Beispiel für ein Write Data (0x03) Kommando auf das Goal Position Register:
      // target_position = (parsed_position / 4096.0) * (2 * PI);
    }
  }
}
