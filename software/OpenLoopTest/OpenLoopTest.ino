#include <Arduino.h>
#include <WiFi.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <SimpleFOC.h>

// --- PINS (ESP32-C3) ---
#define PIN_IN1 0
#define PIN_IN2 1
#define PIN_IN3 2
#define PIN_EN  3
#define PIN_nFLT 4

// --- WIFI CONFIG ---
const char* st_ssid = "FRITZ!Box 6660 Cable UE";
const char* st_pass = "28370714691864306613";
IPAddress local_ip(192, 168, 178, 78);
IPAddress gateway(192, 168, 178, 1);
IPAddress subnet(255, 255, 255, 0);

WebSocketsServer webSocket = WebSocketsServer(81);

// --- MOTOR (Open-Loop) ---
BLDCMotor motor = BLDCMotor(11);           // 11 Polpaare für Gimbal
BLDCDriver3PWM driver = BLDCDriver3PWM(PIN_IN1, PIN_IN2, PIN_IN3, PIN_EN);

float targetVelocity = 0.0;   // rad/s
bool isKilled = true;
bool driver_fault = false;

unsigned long lastTele = 0;

// --- Open-Loop Parameter (für Gimbal Motor) ---
const float OPENLOOP_VOLTAGE = 2.5;   // Start mit 2.5V (sanft, aber spürbar)
const float MAX_VELOCITY = 30.0;      // Max ~5 U/s (300 RPM) - reicht für Test

void handleWebSocket(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    if (type == WStype_TEXT) {
        JsonDocument doc; 
        deserializeJson(doc, payload);
        
        if (doc["cmd"].is<const char*>()) {
            String cmd = doc["cmd"].as<String>();
            if (cmd == "kill") { 
                isKilled = true; 
                targetVelocity = 0;
                Serial.println("Kill command received");
            }
            else if (cmd == "enable") { 
                if (digitalRead(PIN_nFLT) == HIGH) {
                    isKilled = false; 
                    driver_fault = false;
                    Serial.println("Enable command received");
                } else {
                    Serial.println("Cannot enable - driver fault!");
                }
            }
        }
        
        if (doc["val"].is<long>()) {
            long rpm = doc["val"];
            // RPM to rad/s (Open-Loop: Begrenzung auf MAX_VELOCITY)
            float newTarget = (float(rpm) * 2.0 * PI) / 60.0;
            targetVelocity = constrain(newTarget, -MAX_VELOCITY, MAX_VELOCITY);
            Serial.print("Setpoint: ");
            Serial.print(rpm);
            Serial.println(" RPM");
        }
    }
}

void setup() {
    delay(2000);
    Serial.begin(115200);
    Serial.println("\n=== SmartBLDCServo Open-Loop Test ===");
    
    // --- HARDWARE PINS ---
    pinMode(PIN_nFLT, INPUT_PULLUP);
    
    // --- DRIVER INIT ---
    driver.voltage_power_supply = 12.0;
    driver.voltage_limit = OPENLOOP_VOLTAGE;  // Wichtig: auf 2.5V limitieren!
    driver.init();
    motor.linkDriver(&driver);
    
    // --- MOTOR OPEN-LOOP CONFIG ---
    motor.voltage_limit = OPENLOOP_VOLTAGE;
    motor.controller = MotionControlType::velocity_openloop;
    
    // Wichtige Open-Loop Parameter für Gimbal Motor
    motor.velocity_limit = MAX_VELOCITY;           // rad/s max
    motor.voltage_sensor_align = 2.0;              // Spannung für Alignment
    
    // Phase Resistance für bessere Spannungsberechnung (optional)
    motor.phase_resistance = 10.0;  // Ohm (typisch für Gimbal)
    
    motor.init();
    Serial.println("Motor initialized");
    
    // --- OPEN-LOOP ALIGNMENT (wichtig!) ---
    // Bei Open-Loop muss der Motor wissen, wo Phase A ist
    Serial.println("Performing open-loop alignment...");
    motor.zero_electric_angle = 0;  // Start mit 0
    motor.electrical_angle = 0;
    
    // Kleinen Puls geben, um den Motor zu alignen
    motor.move(2.0);  // 2 rad/s ganz langsam
    delay(1000);
    motor.move(0);
    delay(500);
    
    Serial.println("Motor ready! Send 'enable' via WebSocket to start");
    
    // Motor sicherheitshalber AUSSCHALTEN bevor WLAN verbindet!
    // Sonst zieht er konstant Strom, falls das WLAN lange braucht.
    motor.disable();
    isKilled = true;
    
    // --- WIFI ---
    WiFi.mode(WIFI_STA);
    WiFi.config(local_ip, gateway, subnet);
    WiFi.begin(st_ssid, st_pass);
    
    Serial.print("Connecting to WiFi");
    int wifi_timeout = 0;
    while (WiFi.status() != WL_CONNECTED && wifi_timeout < 20) {
        delay(500);
        Serial.print(".");
        wifi_timeout++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi connected");
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\nWiFi connection FAILED! Check SSID/Password.");
    }
    
    // --- WEBSOCKET ---
    webSocket.begin();
    webSocket.onEvent(handleWebSocket);
    
    Serial.println("Setup complete - waiting for commands");
}

void loop() {
    webSocket.loop();
    
    // --- HARDWARE FAULT CHECK ---
    bool fault = (digitalRead(PIN_nFLT) == LOW);
    if (fault && !driver_fault) {
        driver_fault = true;
        isKilled = true;
        targetVelocity = 0;
        motor.disable();
        Serial.println("!!! DRV8313 FAULT DETECTED !!!");
        Serial.println("Check: Overcurrent, Overtemperature, or Undervoltage");
    }
    
    // --- MOTOR CONTROL ---
    if (isKilled) {
        if (motor.enabled) {
            motor.move(0);
            motor.disable();
            Serial.println("Motor disabled (killed)");
        }
    } else {
        if (!motor.enabled) {
            motor.enable();
            Serial.println("Motor enabled - starting movement");
        }
        
        // Open-Loop move
        motor.loopFOC();
        motor.move(targetVelocity);
        
        // Kleines Debugging (alle 2 Sekunden)
        static unsigned long lastDebug = 0;
        if (millis() - lastDebug > 2000) {
            lastDebug = millis();
            float currentRPM = (motor.shaft_velocity * 60.0) / (2.0 * PI);
            Serial.print("Target: ");
            Serial.print((targetVelocity * 60.0) / (2.0 * PI));
            Serial.print(" RPM | Actual: ");
            Serial.print(currentRPM);
            Serial.print(" RPM | Voltage: ");
            Serial.println(motor.voltage.q);
        }
    }
    
    // --- TELEMETRY (alle 100ms für WebUI) ---
    if (millis() - lastTele > 100) { 
        lastTele = millis();
        JsonDocument out; 
        
        // Simulierte Position (für UI)
        static float fake_pos = 0;
        if (!isKilled && abs(targetVelocity) > 0.1) {
            fake_pos += (targetVelocity * 0.1);
            if (fake_pos > 2*PI) fake_pos -= 2*PI;
            if (fake_pos < 0) fake_pos += 2*PI;
        }
        
        out["pos"] = (fake_pos / (2*PI)) * 4096;
        out["rps"] = abs(targetVelocity / (2*PI));
        out["tmc"] = driver_fault ? "ERR" : "OK";
        out["k"] = isKilled ? 1 : 0;
        out["sp"] = (targetVelocity * 60.0) / (2.0 * PI);
        
        String json;
        serializeJson(out, json);
        webSocket.broadcastTXT(json);
    }
}
