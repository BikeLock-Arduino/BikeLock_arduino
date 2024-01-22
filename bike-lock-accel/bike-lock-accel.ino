#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

#define ACCEL_FILTER 0.95

typedef struct {
  float x, y, z;
} vec3_t;

Adafruit_MPU6050 mpu;
vec3_t accel;

long deadline = 0;

void setup() {
  Serial.begin(9600);

  // Try to initialize!
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  // Serial.println("Accelerometer successfully configured");
  delay(100);

  accel.x = 0;
  accel.y = 0;
  accel.z = 0;

  pinMode(4, OUTPUT);
}

void loop() {
  // Calculate pos and orientation
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // Time interval in seconds
  const float deltaTime = 0.1; // Adjust based on your loop rate

  vec3_t newAccel;
  newAccel.x = a.acceleration.x;
  newAccel.y = a.acceleration.y;
  newAccel.z = a.acceleration.z;

  vec3_t jerk;
  jerk.x = (newAccel.x - accel.x) / deltaTime;
  jerk.y = (newAccel.y - accel.y) / deltaTime;
  jerk.z = (newAccel.z - accel.z) / deltaTime;

  float jerkMagSq = jerk.x * jerk.x + jerk.y * jerk.y + jerk.z * jerk.z;

  Serial.print("Jerk:");
  Serial.println(jerkMagSq);

  if (jerkMagSq > 80000) {
    // Movement was detected
    digitalWrite(4, HIGH);
    deadline = millis() + 1000;
    
    accel.x = 0;
    accel.y = 0;
    accel.z = 0;
  } else {
    accel = newAccel;
  }

  if (millis() > deadline) {
    digitalWrite(4, LOW);
  }

  delay(100);
}
