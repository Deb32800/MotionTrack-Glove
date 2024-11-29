#include <Wire.h>
#include <WiFi.h>
#include <WebSocketsServer.h>
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"

// WiFi credentials
const char* ssid = "Dev";
const char* password = "00000000";

// WebSocket server
WebSocketsServer webSocket = WebSocketsServer(81);

// MPU6050 instance
MPU6050 mpu;

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

// Calibration values for MPU6050
int accel_Xcal = 1, accel_Ycal = 1, accel_Zcal = 1;
int gyro_Xcal = 0, gyro_Ycal = 0, gyro_Zcal = 0;

int16_t ax, ay, az, gx, gy, gz; // Accelerometer and Gyroscope data

// Setup for DMP
bool dmpReady = false;
uint8_t mpuIntStatus;
uint8_t devStatus;
uint16_t packetSize;
uint8_t fifoBuffer[64];

Quaternion q;
VectorInt16 aa, aaReal, aaWorld;
VectorFloat gravity;
float euler[3];
float ypr[3];

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

  String data = String(hand) + ":" + String(finger) + ":" + String(angle);
  webSocket.broadcastTXT(data); // Send data to WebSocket clients
}

// Initialize MPU6050
void setupMPU() {
  mpu.initialize();
  if (mpu.testConnection()) {
    Serial.println("MPU6050 connection successful!");
  } else {
    Serial.println("MPU6050 connection failed.");
  }

  devStatus = mpu.dmpInitialize();
  mpu.setXGyroOffset(221);
  mpu.setYGyroOffset(76);
  mpu.setZGyroOffset(-85);
  mpu.setZAccelOffset(1788);

  if (devStatus == 0) {
    mpu.CalibrateAccel(10);
    mpu.CalibrateGyro(10);
    mpu.PrintActiveOffsets();
    mpu.setDMPEnabled(true);
    Serial.println("DMP Ready!");
    packetSize = mpu.dmpGetFIFOPacketSize();
    dmpReady = true;
  } else {
    Serial.print("DMP Initialization failed with code ");
    Serial.println(devStatus);
  }
}

// Read accelerometer and gyroscope data
void accel_and_gyro(const char* hand) {
  if (dmpReady && mpu.dmpGetCurrentFIFOPacket(fifoBuffer)) {
    // Get quaternion data
    mpu.dmpGetQuaternion(&q, fifoBuffer);
    // Calculate gravity vector
    mpu.dmpGetGravity(&gravity, &q);
    // Get yaw, pitch, and roll
    mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);

    // Convert from radians to degrees
    float yaw = ypr[0] * 180.0 / M_PI;
    float pitch = ypr[1] * 180.0 / M_PI;
    float roll = ypr[2] * 180.0 / M_PI;

    // Print Yaw, Pitch, and Roll data to Serial Monitor
    Serial.print("Hand: ");
    Serial.println(hand);
    Serial.print("Yaw: ");
    Serial.println(yaw);
    Serial.print("Pitch: ");
    Serial.println(pitch);
    Serial.print("Roll: ");
    Serial.println(roll);

    // Broadcast data over WebSocket
    webSocket.broadcastTXT(String(hand) + ":RotateX:" + String(yaw));
    webSocket.broadcastTXT(String(hand) + ":RotateY:" + String(pitch));
    webSocket.broadcastTXT(String(hand) + ":RotateZ:" + String(roll));
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);  // Use custom SDA, SCL pins (ESP32)
  
  pinMode(THUMB_PIN, INPUT);
  pinMode(INDEX_PIN, INPUT);
  pinMode(MIDDLE_PIN, INPUT);
  pinMode(RING_PIN, INPUT);
  pinMode(PINKY_PIN, INPUT);

  connectToWiFi();
  
  // Initialize MPU6050 and DMP
  setupMPU();

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
