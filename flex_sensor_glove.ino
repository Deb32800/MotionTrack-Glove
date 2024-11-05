#include <SoftwareSerial.h>
#include <SerialCommand.h>
#include <Wire.h> 
#include <math.h> 

const int MPU = 0x68; // I2C address of the MPU-6050
int16_t accel_X, accel_Y, accel_Z, Tmp, gyro_X, gyro_Y, gyro_Z; // 16-bit integers
SerialCommand sCmd;

// ESP32 pin assignments for flex sensors
const int THUMB_PIN = 36; 
const int INDEX_PIN = 32;  
const int MIDDLE_PIN = 33; 
const int RING_PIN = 34; 
const int PINKY_PIN = 35; 

const float Vin = 5; // Input Voltage of Arduino
const float R_DIV = 14000; // Measured resistance of R2
const float NOMINAL_RES = 26000; // resistance when straight
const float BENT_RES = 60000.0; // resistance at 90 deg

// Acceleration calibration correction
int accel_Xcal = 1;
int accel_Ycal = 1;
int accel_Zcal = 1;

// Gyro calibration correction
int gyro_Xcal = 0;
int gyro_Ycal = 0;
int gyro_Zcal = 0;

void setup() 
{
  // Set the flex sensor pins as input
  pinMode(THUMB_PIN, INPUT);
  pinMode(PINKY_PIN, INPUT);
  pinMode(MIDDLE_PIN, INPUT);
  pinMode(INDEX_PIN, INPUT);
  pinMode(RING_PIN, INPUT);

  // Begin serial communication
  Serial.begin(9600);
  calibrate();
}

void loop() 
{
  // Read the output of each flex sensor
  flexSensor(THUMB_PIN, "Thumb", "Left");
  flexSensor(INDEX_PIN, "Index", "Left");
  flexSensor(RING_PIN, "Ring", "Left");
  flexSensor(MIDDLE_PIN, "Middle", "Left");
  flexSensor(PINKY_PIN, "Pinky", "Left");

  // Read accel and gyro data
  accel_and_gyro("Left");
}

// Read the output voltage from specified pin
// Calculate the resistance of the flex sensor and determine the angle bent
// Print the angle bent through the serial port
void flexSensor(int PIN, String finger, String hand) {
  int voltage_read = analogRead(PIN);
  float Vout = voltage_read * Vin / 1023.0;
  float flex_resistance = R_DIV * (Vin / Vout - 1.0);

  // Linearly map the flex resistance measured to its corresponding angle
  float angle = map(flex_resistance, NOMINAL_RES, BENT_RES, 0, 90.0);
  
  // Angle correction for the thumb                  
  if (finger == "Thumb") {
    angle = angle - 50;
  }

  // Print to serial communication with the hand and finger identifiers
  Serial.println(hand + ":" + finger + ":" + String(angle));
}

// Read the accel and gyro data from MPU-6050
// Read the accel and gyro data from MPU-6050
void accel_and_gyro(String hand) {
  Wire.beginTransmission(MPU); // Start transmission
  Wire.write(0x3B); // Address of the first register for accelerometer data
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 14, true); // Request 14 registers

  // Read accel data (2 bytes each)
  accel_X = (Wire.read() << 8 | Wire.read()) + accel_Xcal; 
  accel_Y = (Wire.read() << 8 | Wire.read()) + accel_Ycal; 
  accel_Z = (Wire.read() << 8 | Wire.read()) + accel_Zcal; 

  // Read gyroscope data (2 bytes each)
  gyro_X = (Wire.read() << 8 | Wire.read()) + gyro_Xcal; 
  gyro_Y = (Wire.read() << 8 | Wire.read()) + gyro_Ycal; 
  gyro_Z = (Wire.read() << 8 | Wire.read()) + gyro_Zcal;  

  // Print gyro data
  Serial.println(hand + ":RotateX:" + String(gyro_X));
  Serial.println(hand + ":RotateY:" + String(gyro_Y));
  Serial.println(hand + ":RotateZ:" + String(gyro_Z));
}

// Calibrate accel and gyro from the initial readings of the data
// Ensure that sensors are stationary at the start for better accuracy
void calibrate() {
    Wire.beginTransmission(MPU);
    Wire.write(0x3B); // Address of the first register for accelerometer data
    Wire.endTransmission(false);
    Wire.requestFrom(MPU, 14, true); // Request 14 registers

    // Read initial accelerometer data (2 bytes each)
    accel_X = (Wire.read() << 8 | Wire.read());  
    accel_Y = (Wire.read() << 8 | Wire.read()); 
    accel_Z = (Wire.read() << 8 | Wire.read());
  
    // Read initial gyroscope data (2 bytes each)
    gyro_X = (Wire.read() << 8 | Wire.read());
    gyro_Y = (Wire.read() << 8 | Wire.read()); 
    gyro_Z = (Wire.read() << 8 | Wire.read());

    // Accel correction
    accel_Xcal = -accel_X - 15000; // Adjust based on your calibration needs
    accel_Ycal = -accel_Y;          // Adjust based on your calibration needs
    accel_Zcal = -accel_Z;          // Adjust based on your calibration needs
    
    // Gyro correction
    gyro_Xcal = -gyro_X;            // Adjust based on your calibration needs
    gyro_Ycal = -gyro_Y - 750;      // Adjust based on your calibration needs
    gyro_Zcal = -gyro_Z;            // Adjust based on your calibration needs
}
