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
#include <stdio.h>
#include <WiFi.h> 
#include "spo2_algorithm.h"
#include <ArduinoJson.h>

MPU6050 gyro; // MPU6050 object
MAX30105 particleSensor; // MAX30105 object

int16_t AccX, AccY, AccZ, Temp, GyroX, GyroY, GyroZ;
const int numLect = 200;
int gyroData[numLect], acelData[numLect], aceGData[numLect];

WiFiClient client;

// wifi 
const char* ssid = "Visitas";
const char* password =  "Cimat2023";
const char* serverAddress = "10.13.200.44";
const int serverPort = 3000;
const char* apiEndpoint = "/api/reads";

#define MAX_BRIGHTNESS 255

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)
//Arduino Uno doesn't have enough SRAM to store 100 samples of IR led data and red led data in 32-bit format
//To solve this problem, 16-bit MSB of the sampled data will be truncated. Samples become 16-bit data.
uint16_t irBuffer[100]; //infrared LED sensor data
uint16_t redBuffer[100];  //red LED sensor data
#else
uint32_t irBuffer[100]; //infrared LED sensor data
uint32_t redBuffer[100];  //red LED sensor data
#endif

int32_t bufferLength; //data length
int32_t spo2; //SPO2 value
int8_t validSPO2; //indicator to show if the SPO2 calculation is valid
int32_t heartRate; //heart rate value
int8_t validHeartRate; //indicator to show if the heart rate calculation is valid

byte pulseLED = 11; //Must be on PWM pin
byte readLED = 6; //Blinks with each data read

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

  byte ledBrightness = 255; //Options: 0=Off to 255=50mA - 60
  byte sampleAverage = 1; //Options: 1, 2, 4, 8, 16, 32 - 1 - checar con todos - 32
  byte ledMode = 2; //Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green - 2
  byte sampleRate = 400; //Options: 50, 100, 200, 400, 800, 1000, 1600, 3200 - 400 - 1600
  int pulseWidth = 69; //Options: 69, 118, 215, 411 - 69
  int adcRange = 16384; //Options: 2048, 4096, 8192, 16384 - 16384

  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange); //Configure sensor with these settings

  // INICIALIZACIÓN Y CONEXION AL WIFI
  WiFi.begin(ssid, password);

  Serial.print("Conectando...");
  
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.print("Conectado con éxito, mi IP es: ");
  Serial.print(WiFi.localIP());
}



void loop() {
  // Read from MPU6050
  int16_t gyroX, gyroY, gyroZ;
  gyro.getRotation(&gyroX, &gyroY, &gyroZ);

  Serial.print("MPU6050 Gyro: ");
  Serial.print("X = "); Serial.print(gyroX);
  Serial.print(", Y = "); Serial.print(gyroY);
  Serial.print(", Z = "); Serial.println(gyroZ);

  // Read from MAX30105
  uint32_t redValue = particleSensor.getRed();
  uint32_t irValue = particleSensor.getIR();

  Serial.print("MAX30105 Red Value: ");
  Serial.print(redValue);
  Serial.print(", IR Value: ");
  Serial.println(irValue);

  // codigo de Luis
  /*
  bufferLength = 100; //buffer length of 100 stores 4 seconds of samples running at 25sps

  //read the first 100 samples, and determine the signal range
  for (byte i = 0 ; i < bufferLength ; i++)
  {
    while (particleSensor.available() == false) //do we have new data?
      particleSensor.check(); //Check the sensor for new data

    redBuffer[i] = particleSensor.getRed();
    irBuffer[i] = particleSensor.getIR();
    particleSensor.nextSample(); //We're finished with this sample so move to next sample

    Serial.print(redBuffer[i]);
    Serial.print(F(","));
    Serial.println(irBuffer[i]);
  }

  //calculate heart rate and SpO2 after first 100 samples (first 4 seconds of samples)
  maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);

  //Continuously taking samples from MAX30102.  Heart rate and SpO2 are calculated every 1 second
  while (1)
  {
    //dumping the first 25 sets of samples in the memory and shift the last 75 sets of samples to the top
    for (byte i = 25; i < 100; i++)
    {
      redBuffer[i - 25] = redBuffer[i];
      irBuffer[i - 25] = irBuffer[i];
    }

    //take 25 sets of samples before calculating the heart rate.
    for (byte i = 75; i < 100; i++)
    {
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
      delay(2000);
    }

    //After gathering 25 new samples recalculate HR and SP02
    maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
  }*/


  delay(50); // Delay between readings

}