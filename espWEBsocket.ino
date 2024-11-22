#include <WiFi.h>
#include <WebSocketsServer.h>
#include <Wire.h>
#include <math.h>

// WiFi credentials
const char* ssid = "TP-Link_BFA4";
const char* password = "80664444";

// WebSocket server on port 81
WebSocketsServer webSocket = WebSocketsServer(81);

// MPU6050 I2C address
const int MPU = 0x68; 
int16_t accel_X, accel_Y, accel_Z, gyro_X, gyro_Y, gyro_Z;

// ESP32 pin assignments for flex sensors
const int THUMB_PIN = 36; 
const int INDEX_PIN = 32;  
const int MIDDLE_PIN = 33; 
const int RING_PIN = 34; 
const int PINKY_PIN = 35; 

const float Vin = 5; // Input Voltage of ESP32
const float R_DIV = 14000; // Measured resistance of R2
const float NOMINAL_RES = 26000; // resistance when straight
const float BENT_RES = 60000.0; // resistance at 90 deg

// Calibration variables
int accel_Xcal = 1, accel_Ycal = 1, accel_Zcal = 1;
int gyro_Xcal = 0, gyro_Ycal = 0, gyro_Zcal = 0;

// Setup WiFi and WebSocket
void setup() {
  Serial.begin(115200);
  Wire.begin();

  // Setup WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");
  Serial.println("IP Address: " + WiFi.localIP().toString());

  // Initialize MPU6050
  Wire.beginTransmission(MPU);
  Wire.write(0x6B); // Power management register
  Wire.write(0);    // Wake up MPU6050
  Wire.endTransmission(true);

  // Start WebSocket server
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  Serial.println("WebSocket server started on port 81.");

  // Set the flex sensor pins as input
  pinMode(THUMB_PIN, INPUT);
  pinMode(PINKY_PIN, INPUT);
  pinMode(MIDDLE_PIN, INPUT);
  pinMode(INDEX_PIN, INPUT);
  pinMode(RING_PIN, INPUT);

  // Calibrate sensors
  calibrate();
}

// WebSocket events
void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
  if (type == WStype_CONNECTED) {
    Serial.printf("Client [%u] connected.\n", num);
  } else if (type == WStype_DISCONNECTED) {
    Serial.printf("Client [%u] disconnected.\n", num);
  }
}

// Main loop
void loop() {
  webSocket.loop(); // Handle WebSocket clients

  // Collect data
  String data = "{";

  // Add flex sensor data
  data += getFlexSensorData(THUMB_PIN, "Thumb", "Left");
  data += ",";
  data += getFlexSensorData(INDEX_PIN, "Index", "Left");
  data += ",";
  data += getFlexSensorData(MIDDLE_PIN, "Middle", "Left");
  data += ",";
  data += getFlexSensorData(RING_PIN, "Ring", "Left");
  data += ",";
  data += getFlexSensorData(PINKY_PIN, "Pinky", "Left");
  data += ",";

  // Add MPU6050 data
  data += getMPUData("Left");
  data += "}";

  // Send data to all WebSocket clients
  webSocket.broadcastTXT(data);

  delay(100); // Send data every 100ms
}

// Read flex sensor data
String getFlexSensorData(int PIN, String finger, String hand) {
  int voltage_read = analogRead(PIN);
  float Vout = voltage_read * Vin / 4095.0; // ESP32 ADC resolution
  float flex_resistance = R_DIV * (Vin / Vout - 1.0);

  // Map flex resistance to angle
  float angle = map(flex_resistance, NOMINAL_RES, BENT_RES, 0, 90.0);
  
  // Angle correction for the thumb
  if (finger == "Thumb") {
    angle = angle - 50;
  }

  return "\"" + hand + "_" + finger + "\":" + String(angle);
}

// Read MPU6050 data
String getMPUData(String hand) {
  Wire.beginTransmission(MPU); 
  Wire.write(0x3B); 
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 14, true);

  // Read accelerometer data
  accel_X = (Wire.read() << 8 | Wire.read()) + accel_Xcal; 
  accel_Y = (Wire.read() << 8 | Wire.read()) + accel_Ycal; 
  accel_Z = (Wire.read() << 8 | Wire.read()) + accel_Zcal; 

  // Read gyroscope data
  gyro_X = (Wire.read() << 8 | Wire.read()) + gyro_Xcal; 
  gyro_Y = (Wire.read() << 8 | Wire.read()) + gyro_Ycal; 
  gyro_Z = (Wire.read() << 8 | Wire.read()) + gyro_Zcal;

  // Format as JSON
  return "\"accelX\":" + String(accel_X) + "," +
         "\"accelY\":" + String(accel_Y) + "," +
         "\"accelZ\":" + String(accel_Z) + "," +
         "\"gyroX\":" + String(gyro_X) + "," +
         "\"gyroY\":" + String(gyro_Y) + "," +
         "\"gyroZ\":" + String(gyro_Z);
}

// Calibrate sensors
void calibrate() {
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 14, true);

  // Read initial data for calibration
  accel_Xcal = -((Wire.read() << 8 | Wire.read())); 
  accel_Ycal = -((Wire.read() << 8 | Wire.read())); 
  accel_Zcal = -((Wire.read() << 8 | Wire.read())); 
  gyro_Xcal = -((Wire.read() << 8 | Wire.read())); 
  gyro_Ycal = -((Wire.read() << 8 | Wire.read())); 
  gyro_Zcal = -((Wire.read() << 8 | Wire.read())); 
}
