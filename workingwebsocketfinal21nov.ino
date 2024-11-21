#include <WiFi.h>
#include <WebSocketsServer.h>
#include <Wire.h>

const char* ssid = "Dev";
const char* password = "00000000";

WebSocketsServer webSocket = WebSocketsServer(81); // WebSocket server on port 81

// MPU6050 variables
const int MPU = 0x68; 
int16_t accel_X, accel_Y, accel_Z, gyro_X, gyro_Y, gyro_Z;

// Setup WiFi and WebSocket
void setup() {
  Serial.begin(115200);
  Wire.begin();

  // Connect to WiFi
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
}

// WebSocket events
void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
  if (type == WStype_CONNECTED) {
    Serial.printf("Client [%u] connected.\n", num);
  } else if (type == WStype_DISCONNECTED) {
    Serial.printf("Client [%u] disconnected.\n", num);
  }
}

// Read MPU6050 data and send to WebSocket clients
void loop() {
  webSocket.loop();

  Wire.beginTransmission(MPU);
  Wire.write(0x3B); // First register for accelerometer data
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 14, true);

  // Read accelerometer and gyroscope data
  accel_X = Wire.read() << 8 | Wire.read();
  accel_Y = Wire.read() << 8 | Wire.read();
  accel_Z = Wire.read() << 8 | Wire.read();
  gyro_X = Wire.read() << 8 | Wire.read();
  gyro_Y = Wire.read() << 8 | Wire.read();
  gyro_Z = Wire.read() << 8 | Wire.read();

  // Format sensor data as JSON
  String data = String("{") +
                "\"accelX\":" + accel_X + "," +
                "\"accelY\":" + accel_Y + "," +
                "\"accelZ\":" + accel_Z + "," +
                "\"gyroX\":" + gyro_X + "," +
                "\"gyroY\":" + gyro_Y + "," +
                "\"gyroZ\":" + gyro_Z +
                "}";

  // Send data to all connected clients
  webSocket.broadcastTXT(data);

  delay(100); // Send data every 100ms
}
