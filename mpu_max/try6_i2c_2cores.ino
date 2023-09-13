/* 
 MPU6050 Connections (ESP32):
  -Vin = 3.3 V
  -GND = GND
  -SDA = D21
  -SCL = D22
  -INT = Not connected

 MAX30102 Connections (ESP32):
  -Vin = 3.3 V
  -GND = GND
  -SDA = D21
  -SCL = D22
  -INT = Not connected

*/
#include <Wire.h>
#include "MPU6050.h" // adafruit library
#include "MAX30105.h" // sparkfun library 
#include <WiFi.h> 
#include "spo2_algorithm.h" // include in sparkfun library 
#include <ArduinoJson.h> // benoit b library

MPU6050 gyro;
MAX30105 particleSensor;

extern TwoWire Wire1; // Create a separate I2C instance for MAX30105

WiFiClient client;

byte pulseLED = 11; //Must be on PWM pin
byte readLED = 6; //Blinks with each data read

// wifi 
const char* ssid = "Visitas";
const char* password =  "Cimat2023";
const char* serverAddress = "10.13.200.44";
const int serverPort = 3000;
const char* apiEndpoint = "/api/reads";

#define MAX_BRIGHTNESS 255

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)
uint16_t irBuffer[100]; //infrared LED sensor data
uint16_t redBuffer[100];  //red LED sensor data
#else
uint32_t irBuffer[100]; //infrared LED sensor data
uint32_t redBuffer[100];  //red LED sensor data
#endif

TaskHandle_t mpuTaskHandle = NULL;
TaskHandle_t maxTaskHandle = NULL;

int32_t bufferLength; //data length
int32_t spo2; //SPO2 value
int8_t validSPO2; //indicator to show if the SPO2 calculation is valid
int32_t heartRate; //heart rate value
int8_t validHeartRate; //indicator to show if the heart rate calculation is valid

DynamicJsonDocument jsonDocument(1024); // Declare it globally

// MPU6050 task
void mpuTask(void *parameter) {
  (void) parameter;
  int16_t gyroX, gyroY, gyroZ;
  int16_t aX, aY, aZ;
  const float alpha = 0.2; // Smoothing factor (adjust as needed)
  float filteredAccelX = 0.0, filteredAccelY = 0.0, filteredAccelZ = 0.0;
  float filteredGyroX = 0.0, filteredGyroY = 0.0, filteredGyroZ = 0.0;

  for (;;) {
    //gyro.setFullScaleAccelRange(MPU6050_ACCEL_FS_2);
    gyro.getRotation(&gyroX, &gyroY, &gyroZ);
    gyro.getAcceleration(&aX, &aY, &aZ);

    float accelX = aX * 9.81 / 16384.0; // 16384 LSB/g for +/- 2g range
    float accelY = aY * 9.81 / 16384.0;
    float accelZ = aZ * 9.81 / 16384.0;

    // Apply low-pass filter
    filteredGyroX = alpha * accelX + (1 - alpha) * filteredGyroX;
    filteredGyroY = alpha * accelY + (1 - alpha) * filteredGyroY;
    filteredGyroZ = alpha * accelZ + (1 - alpha) * filteredGyroZ;
    
    filteredAccelX = alpha * accelX + (1 - alpha) * filteredAccelX;
    filteredAccelY = alpha * accelY + (1 - alpha) * filteredAccelY;
    filteredAccelZ = alpha * accelZ + (1 - alpha) * filteredAccelZ;

    Serial.print("MPU6050 Gyro: ");
    Serial.print("X = "); Serial.print(filteredGyroX);
    Serial.print(", Y = "); Serial.print(filteredGyroY);
    Serial.print(", Z = "); Serial.println(filteredGyroZ);

    Serial.print("MPU6050 Accel (m/s^2): ");
    Serial.print("X = "); Serial.print(filteredAccelX);
    Serial.print(", Y = "); Serial.print(filteredAccelY);
    Serial.print(", Z = "); Serial.println(filteredAccelZ);
    
    // Print stack usage
    UBaseType_t mpuStackUsage = uxTaskGetStackHighWaterMark(mpuTaskHandle);
    Serial.print("MPU6050 Task Stack Usage: ");
    Serial.println(mpuStackUsage);

    vTaskDelay(500 / portTICK_PERIOD_MS); // Delay between readings
  }
}

// MAX30105 task
void maxTask(void *parameter) {

  (void) parameter;
  for (;;) {
    bufferLength = 100;
    // Read the first 100 samples and determine the signal range
    for (byte i = 0; i < bufferLength; i++) {
      while (particleSensor.available() == false)
        particleSensor.check();

      // Store samples in redBuffer and irBuffer
      redBuffer[i] = particleSensor.getRed();
      irBuffer[i] = particleSensor.getIR();
      particleSensor.nextSample();

      Serial.print(redBuffer[i]);
      Serial.print(F(","));
      Serial.println(irBuffer[i]);
    }

    // Calculate heart rate and SpO2 after first 100 samples
    maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);

    // Continuously taking samples from MAX30105
    while (1) 
    {
      particleSensor.clearFIFO(); // Clear FIFO
      particleSensor.nextSample(); // Prepare for next sample
      // Your MAX30105 code here
      //dumping the first 25 sets of samples in the memory and shift the last 75 sets of samples to the top
      for (byte i = 25; i < 100; i++){
        redBuffer[i - 25] = redBuffer[i];
        irBuffer[i - 25] = irBuffer[i];
      }

      //take 25 sets of samples before calculating the heart rate.
      for (byte i = 75; i < 100; i++){
        while (particleSensor.available() == false) //do we have new data?
          particleSensor.check(); //Check the sensor for new data

        digitalWrite(readLED, !digitalRead(readLED)); //Blink onboard LED with every data read

        redBuffer[i] = particleSensor.getRed();
        irBuffer[i] = particleSensor.getIR();
        particleSensor.nextSample(); //We're finished with this sample so move to next sample

        //send samples and calculation result to terminal program through UART

        Serial.print(F("RED: "));
        Serial.print(redBuffer[i], DEC);
        Serial.print(F(", IR: "));
        Serial.print(irBuffer[i], DEC);

        // Serial.print(F("red="));
        // Serial.print(redBuffer[i], DEC);
        // Serial.print(F(", ir="));
        // Serial.print(irBuffer[i], DEC);

        Serial.print(F(", HR: "));
        Serial.print(heartRate, DEC);
        // Serial.print(F(","));

        Serial.print(F(", HRvalid: "));
        Serial.print(validHeartRate, DEC);
        // Serial.print(F(","));

        Serial.print(F(", SPO2: "));
        Serial.print(spo2, DEC);
        // Serial.print(F(","));

        Serial.print(F(", SPO2Valid: "));
        Serial.println(validSPO2, DEC);
        // Serial.println(F(";"));

        // PETICION POST PARA ALMACENAR LOS DATOS EN LA BD
        if(WiFi.status() == WL_CONNECTED) {
          // Tamaño máximo del JSON, ajusta según tus necesidades
        DynamicJsonDocument jsonDocument(1024);

        jsonDocument["red"] = String(redBuffer[i]);
        jsonDocument["ir"] = String(irBuffer[i]);
        jsonDocument["hr"] = String(heartRate);
        jsonDocument["validHR"] = String(validHeartRate);
        jsonDocument["spo2"] = String(spo2);
        jsonDocument["validSpo2"] = String(validSPO2);

        String jsonString;
        serializeJson(jsonDocument, jsonString);

        if (client.connect(serverAddress, serverPort)) {
          
          // Crea la solicitud POST
          String request = "POST " + String(apiEndpoint) + " HTTP/1.1\r\n";
          request += "Host: " + String(serverAddress) + "\r\n";
          request += "Content-Type: application/json\r\n";
          request += "Content-Length: " + String(jsonString.length()) + "\r\n";
          request += "Connection: close\r\n\r\n";
          request += jsonString;

          // Envía la solicitud al servidor
          client.print(request);

          // Lee y muestra la respuesta del servidora
          while (client.available()) {
            String line = client.readStringUntil('\r');
            Serial.print(line);
          }

          // Cierra la conexión
          client.stop();
        } else {
          Serial.println("Error al conectar al servidor");
        }
      }else{
        Serial.println("Error en la conexión WIFI");
      }
      delay(50); // cambio a 50 por problemas de sincronizacion
    }

      //After gathering 25 new samples recalculate HR and SP02
      maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
      // ... Read MAX30105 data, calculations, and JSON POST
      // Print stack usage
      UBaseType_t maxStackUsage = uxTaskGetStackHighWaterMark(maxTaskHandle);
      Serial.print("MAX30105 Task Stack Usage: ");
      Serial.println(maxStackUsage);

      vTaskDelay(2000 / portTICK_PERIOD_MS); // Delay for 2 seconds
    }
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin();

  gyro.initialize();
  particleSensor.begin(Wire, I2C_SPEED_FAST);

  xTaskCreatePinnedToCore(mpuTask, "MPUTask", 4096, NULL, 1, &mpuTaskHandle, 0); // Run on Core 0
  xTaskCreatePinnedToCore(maxTask, "MAXTask", 4096, NULL, 1, &maxTaskHandle, 1); // Run on Core 1

}

void loop() {
  // Main loop can be kept empty since tasks are running on separate cores
}
