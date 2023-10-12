/* 
 MPU6050 Connections (ESP32):
  -Vin = 3.3 V
  -GND = GND
  -SDA = D21
  -SCL = D22
 
 MAX30102 Connections (ESP32):
  -Vin = 3.3 V
  -GND = GND
  -SDA = D21
  -SCL = D22
  -INT = Not connected

*/

#include <Wire.h> // I2C library
#include "MPU6050.h" // MPU6050 library by adafruit
#include "MAX30105.h" // MAX30105 library by sparkfun
#include <freertos/semphr.h> // library to worok with semaphores for tasks
//#include <ArduinoJson.h>

MPU6050 gyro; // MPU6050 object
MAX30105 particleSensor; // MAX30105 object

WiFiClient client;

// wifi 
const char* ssid = "Visitas";
const char* password =  "Cimat2023";
const char* serverAddress = "10.13.200.62";
const int serverPort = 3000;
const char* apiEndpoint = "/api/reads";

// tasks definition 
TaskHandle_t mpuTaskHandle = NULL;
TaskHandle_t maxTaskHandle = NULL;
// semaphores definition 
SemaphoreHandle_t gyroMutex;
SemaphoreHandle_t maxMutex;

// shared variables to hold data for MPU6050
float filteredAccelX, filteredAccelY, filteredAccelZ;
float filteredGyroX, filteredGyroY , filteredGyroZ;

// semaphore mutex definition 
SemaphoreHandle_t mutex = xSemaphoreCreateMutex();

void mpuTask(void *parameter){
  (void) parameter;
  int16_t gyroX, gyroY, gyroZ;
  int16_t aX, aY, aZ;
  const float alpha = 0.2; // Smoothing factor
  
  //gyro.initialize();

  for(;;){
    if (xSemaphoreTake(gyroMutex, portMAX_DELAY) == pdTRUE){
      gyro.getRotation(&gyroX, &gyroY, &gyroZ);
      gyro.getAcceleration(&aX, &aY, &aZ);
      xSemaphoreGive(gyroMutex);

      float accelX = aX * 9.81 / 16384.0; // 16384 LSB/g for +/- 2g range
      float accelY = aY * 9.81 / 16384.0;
      float accelZ = aZ * 9.81 / 16384.0;
      
      // Apply low-pass filter
      filteredGyroX = alpha * gyroX + (1 - alpha) * filteredGyroX;
      filteredGyroY = alpha * gyroY + (1 - alpha) * filteredGyroY;
      filteredGyroZ = alpha * gyroZ + (1 - alpha) * filteredGyroZ;
      
      filteredAccelX = alpha * accelX + (1 - alpha) * filteredAccelX;
      filteredAccelY = alpha * accelY + (1 - alpha) * filteredAccelY;
      filteredAccelZ = alpha * accelZ + (1 - alpha) * filteredAccelZ;
    
      Serial.print("MPU6050 Gyro: ");
      Serial.print("X = "); Serial.print(filteredGyroX);
      Serial.print(", Y = "); Serial.print(filteredGyroY);
      Serial.print(", Z = "); Serial.println(filteredGyroZ);

      Serial.print("MPU6050 Accel (cm/s^2): ");
      Serial.print("X = "); Serial.print(filteredAccelX);
      Serial.print(", Y = "); Serial.print(filteredAccelY);
      Serial.print(", Z = "); Serial.println(filteredAccelZ);

      size_t stackHighWatermark = uxTaskGetStackHighWaterMark(NULL);
      Serial.print("Stack high watermark: ");
      Serial.println(stackHighWatermark);
    }
    vTaskDelay(pdMS_TO_TICKS(100)); // Delay between readings
  }
}

void maxTask( void *parameter){
  (void) parameter;
  particleSensor.setup();
  int16_t red, ir, green;

  for(;;){
    if (xSemaphoreTake(maxMutex, portMAX_DELAY) == pdTRUE){
      red = particleSensor.getRed();
      ir = particleSensor.getIR();
      green = particleSensor.getGreen();
      xSemaphoreGive(maxMutex);

      Serial.print("MAX data ");
      Serial.print("R = ");Serial.println(red);
      Serial.print("IR = ");Serial.println(ir);
      Serial.print("G = "); Serial.println(green);
      size_t stackHighWatermark = uxTaskGetStackHighWaterMark(NULL);
      Serial.print("Stack high watermark: ");
      Serial.println(stackHighWatermark);Serial.println("\n");
    }
  }
  
}


void setup() {
  Serial.begin(115200);
  Wire.begin(); // Initialize I2C bus

  // Initialize MPU6050
  gyro.initialize();
  if (gyro.testConnection()) {
    Serial.println("MPU6050 connected and initialized!");
  } else {
    Serial.println("MPU6050 connection failed!");
    while (1);
  }

  // Initialize MAX30105
  if (particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30105 connected and initialized!");
  } else {
    Serial.println("MAX30105 connection failed!");
    while (1);
  }

  gyroMutex = xSemaphoreCreateMutex();
  maxMutex = xSemaphoreCreateMutex();

  xTaskCreatePinnedToCore(mpuTask, "MPU Task", 4096, NULL, 2, &mpuTaskHandle, 0); // task on core 0, priority 2
  xTaskCreatePinnedToCore(maxTask, "MAX Task", 4096, NULL, 1, &maxTaskHandle, 1); // task on core 1, priority 1
  
}



void loop() {
  
}