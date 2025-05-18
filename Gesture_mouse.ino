/*
 * Gesture Controlled Mouse with Click using ESP32, MPU6050, and Flex Sensor
 */

#include <Wire.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include "I2Cdev.h"
#include "MPU6050.h"

// WiFi credentials
const char* ssid = "Jobi";
const char* password = "12345678";

// UDP
WiFiUDP udp;
IPAddress receiverIP(192, 168, 154, 61);  // <- Replace with your computer's IP
const int receiverPort = 4210;

// MPU6050
MPU6050 mpu;
int16_t ax, ay, az, gx, gy, gz;

// Flex Sensor
#define FLEX_SENSOR_PIN 34
#define FLEX_CLICK_THRESHOLD 2900
unsigned long lastClickTime = 0;
unsigned long debounceDelay = 500;

// Vcc for Flex Sensor via GPIO15
#define FLEX_VCC_PIN 15

// Movement smoothing
float smoothingFactor = 0.8;
float lastX = 0, lastY = 0;

// Sensitivity
float sensitivity = 2.5;

void setup() {
  Serial.begin(115200);

  // Flex sensor setup
  pinMode(FLEX_SENSOR_PIN, INPUT);
  pinMode(FLEX_VCC_PIN, OUTPUT);
  digitalWrite(FLEX_VCC_PIN, HIGH); // supply constant high to flex sensor

  // MPU6050 setup
  Wire.begin();
  Serial.println("Initializing MPU6050...");
  mpu.initialize();
  Serial.println(mpu.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");

  // Optional: Adjust sensitivity ranges
  mpu.setFullScaleGyroRange(MPU6050_GYRO_FS_1000);
  mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_2);

  // WiFi
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" CONNECTED");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Start UDP
  udp.begin(receiverPort);

  delay(1000); // let everything settle
}

void loop() {
  // Get accelerometer data
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  float accX = ax / 16384.0;  // normalize
  float accY = ay / 16384.0;

  float moveX = accY * sensitivity;  // horizontal movement
  float moveY = accX * sensitivity;  // vertical movement

  // Apply smoothing
  moveX = moveX * (1 - smoothingFactor) + lastX * smoothingFactor;
  moveY = moveY * (1 - smoothingFactor) + lastY * smoothingFactor;

  lastX = moveX;
  lastY = moveY;

  // Send movement if not zero
  if (abs(moveX) > 0.05 || abs(moveY) > 0.05) {
    char buffer[50];
    sprintf(buffer, "%.2f,%.2f", moveX, moveY);
    udp.beginPacket(receiverIP, receiverPort);
    udp.write((uint8_t*)buffer, strlen(buffer));
    udp.endPacket();
    Serial.print("Move: ");
    Serial.println(buffer);
  }

  // Flex sensor for click
  int flexValue = analogRead(FLEX_SENSOR_PIN);
  Serial.print("Flex: "); Serial.println(flexValue);

  if (flexValue > FLEX_CLICK_THRESHOLD && millis() - lastClickTime > debounceDelay) {
    udp.beginPacket(receiverIP, receiverPort);
    udp.print("CLICK");
    udp.endPacket();
    Serial.println("CLICK sent");
    lastClickTime = millis();
  }

  delay(20);
}
