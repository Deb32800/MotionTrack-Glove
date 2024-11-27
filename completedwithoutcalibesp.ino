#include <WiFi.h>
#include <WebSocketsServer.h>
#include <Wire.h>
#include <math.h>

// WiFi credentials
const char* ssid = "Dev";
const char* password = "00000000";

// WebSocket server
WebSocketsServer webSocket = WebSocketsServer(81);

// MPU6050 I2C address
const int MPU = 0x68; 

// Flex sensor pins
const int THUMB_PIN = 36; 
const int INDEX_PIN = 32;  
const int MIDDLE_PIN = 33; 
const int RING_PIN = 34; 
const int PINKY_PIN = 35; 

// Sensor constants
const float Vin = 5.0;
const float R_DIV = 14000;
const float NOMINAL_RES = 26000;
const float BENT_RES = 60000.0;

// Calibration values
int accel_Xcal = 1, accel_Ycal = 1, accel_Zcal = 1;
int gyro_Xcal = 0, gyro_Ycal = 0, gyro_Zcal = 0;

int16_t accel_X, accel_Y, accel_Z, gyro_X, gyro_Y, gyro_Z;

// Connect to WiFi
void connectToWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");
  Serial.println("IP Address: " + WiFi.localIP().toString());
}

// WebSocket event handler
void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
  if (type == WStype_CONNECTED) {
    Serial.printf("Client [%u] connected.\n", num);
  } else if (type == WStype_DISCONNECTED) {
    Serial.printf("Client [%u] disconnected.\n", num);
  }
}

// Flex sensor reading
void flexSensor(int PIN, const char* finger, const char* hand) {
  int voltage_read = analogRead(PIN);
  float Vout = voltage_read * Vin / 4095.0; // For 12-bit ADC
  float flex_resistance = R_DIV * (Vin / Vout - 1.0);
  float angle = map(flex_resistance, NOMINAL_RES, BENT_RES, 0, 90.0);

  if (strcmp(finger, "Thumb") == 0) {
    angle -= 50; // Thumb-specific correction
  }

  String data = String(hand) + ":" + String(finger) + ":" + String(angle);
  webSocket.broadcastTXT(data); // Send data to WebSocket clients
}

// MPU6050 data reading
void accel_and_gyro(const char* hand) {
  Wire.beginTransmission(MPU);
  Wire.write(0x3B); // Address of the first register for accelerometer data
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 14, true);

  accel_X = (Wire.read() << 8 | Wire.read()) + accel_Xcal; 
  accel_Y = (Wire.read() << 8 | Wire.read()) + accel_Ycal; 
  accel_Z = (Wire.read() << 8 | Wire.read()) + accel_Zcal; 

  gyro_X = (Wire.read() << 8 | Wire.read()) + gyro_Xcal; 
  gyro_Y = (Wire.read() << 8 | Wire.read()) + gyro_Ycal; 
  gyro_Z = (Wire.read() << 8 | Wire.read()) + gyro_Zcal;  

  // Broadcast data
  webSocket.broadcastTXT(String(hand) + ":RotateX:" + String(gyro_X));
  webSocket.broadcastTXT(String(hand) + ":RotateY:" + String(gyro_Y));
  webSocket.broadcastTXT(String(hand) + ":RotateZ:" + String(gyro_Z));
}

// MPU6050 calibration
void calibrate() {
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 14, true);

  accel_X = (Wire.read() << 8 | Wire.read());
  accel_Y = (Wire.read() << 8 | Wire.read());
  accel_Z = (Wire.read() << 8 | Wire.read());

  gyro_X = (Wire.read() << 8 | Wire.read());
  gyro_Y = (Wire.read() << 8 | Wire.read());
  gyro_Z = (Wire.read() << 8 | Wire.read());

  accel_Xcal = -accel_X;
  accel_Ycal = -accel_Y;
  accel_Zcal = -accel_Z;

  gyro_Xcal = -gyro_X;
  gyro_Ycal = -gyro_Y;
  gyro_Zcal = -gyro_Z;
}

void setup() {
  Serial.begin(115200);
  Wire.begin();

  pinMode(THUMB_PIN, INPUT);
  pinMode(INDEX_PIN, INPUT);
  pinMode(MIDDLE_PIN, INPUT);
  pinMode(RING_PIN, INPUT);
  pinMode(PINKY_PIN, INPUT);

  connectToWiFi();
  calibrate();

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  Serial.println("WebSocket server started on port 81.");
}

void loop() {
  webSocket.loop();

  // Read flex sensors
  flexSensor(THUMB_PIN, "Thumb", "Left");
  flexSensor(INDEX_PIN, "Index", "Left");
  flexSensor(MIDDLE_PIN, "Middle", "Left");
  flexSensor(RING_PIN, "Ring", "Left");
  flexSensor(PINKY_PIN, "Pinky", "Left");

  // Read accelerometer and gyroscope
  accel_and_gyro("Left");

  delay(100); // Adjust based on the desired update rate
}
