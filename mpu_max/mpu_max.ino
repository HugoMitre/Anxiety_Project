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
#include <ArduinoJson.h>

// REAL TIME (INTERN MODULE)
#include "rtc_datetime.h"
#include <ESP32Time.h>


MPU6050 gyro; // MPU6050 object
MAX30105 particleSensor; // MAX30105 object

int16_t AccX, AccY, AccZ, Temp, GyroX, GyroY, GyroZ;
const int numLect = 200;
int gyroData[numLect], acelData[numLect], aceGData[numLect];

WiFiClient client;

ESP32Time rtc;  // Creat a structure type ESP32Time
// Variables to get 
int year, month, day, hour, minute, second;

// wifi 
const char* ssid = "Visitas";
const char* password =  "Cimat2023";
const char* serverAddress = "10.13.200.62";
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
byte readLED = 13; //Blinks with each data read

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
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Conectando...");
  
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.print("Conectado con éxito, mi IP es: ");
  Serial.print(WiFi.localIP());

  // GETTING REAL DATETIME
  String jsonBody;
  String response;

  if (client.connect("worldtimeapi.org", 80)) {
    // Enviar la solicitud GET a la API
    client.println("GET /api/timezone/America/Mexico_City HTTP/1.1\r\n");
    client.println("Host: worldtimeapi.org\r\n");
    client.println("Connection: close");
    client.println();
    
    delay(1000);
    
    // Leer la respuesta de la API
    while (client.available()) { 
      response = client.readString();
    }
    
    int startIndex = response.indexOf('{');
    int endIndex = response.lastIndexOf('}');
    
    // Extraer el cuerpo JSON de la respuesta
    jsonBody = response.substring(startIndex, endIndex + 1);
    extractTimeFromJson(jsonBody, year, month, day, hour, minute, second);

    client.stop();
  } else {
    Serial.println("Error al establecer la conexión con la API");
  }

  rtc.setTime(second,minute,hour,day,month,year);
}



void loop() {
  // Read from MPU6050
  int16_t gyroX, gyroY, gyroZ;
  int16_t accelX, accelY, accelZ;
  const float alpha =0.2;
  float fgX, fgY, fgZ;
  float faX, faY, faZ;

  gyro.getRotation(&gyroX, &gyroY, &gyroZ);
  gyro.getAcceleration(&accelX, &accelY, &accelZ);

  //datos convertidos de aceleracion m/s^2 a cm/s^2 
  float aX = accelX * 9.81 / 16384.0;
  float aY = accelY * 9.81 / 16384.0;
  float aZ = accelZ * 9.81 / 16384.0;

  // datos giro filtrados
  fgX = alpha * gyroX + (1 - alpha) * fgX;
  fgY = alpha * gyroY + (1 - alpha) * fgY;
  fgZ = alpha * gyroZ + (1 - alpha) * fgZ;

  faX = alpha * aX + (1 - alpha) * faX;
  faY = alpha * aY + (1 - alpha) * faY;
  faZ = alpha * aZ + (1 - alpha) * faZ;

  Serial.print("MPU6050 Gyro: ");
  Serial.print("X = "); Serial.print(fgX);
  Serial.print(", Y = "); Serial.print(fgY);
  Serial.print(", Z = "); Serial.println(fgZ);

  Serial.print("MPU6050 Accel: ");
  Serial.print("X = "); Serial.print(faX);
  Serial.print(", Y = "); Serial.print(faY);
  Serial.print(", Z = "); Serial.println(faZ);  // falta el filtro de ambos datos 

  // Read from MAX30105
  uint32_t redValue = particleSensor.getRed();
  uint32_t irValue = particleSensor.getIR();

  Serial.print("MAX30105 Red Value: ");
  Serial.print(redValue);
  Serial.print(", IR Value: ");
  Serial.println(irValue);

  // PETICION POST PARA ALMACENAR LOS DATOS EN LA BD
  if(WiFi.status() == WL_CONNECTED) {
    // Tamaño máximo del JSON, ajusta según tus necesidades
    DynamicJsonDocument jsonDocument(1024);
    
    jsonDocument["time"] = rtc.getTime() + "." + String(rtc.getMillis());
    jsonDocument["red"] = String(redValue);
    jsonDocument["ir"] = String(irValue);
    // jsonDocument["hr"] = heartRate;
    // jsonDocument["spo2"] = spo2;
    jsonDocument["gyroX"] = fgX;
    jsonDocument["gyroY"] = fgY;
    jsonDocument["gyroZ"] = fgZ;
    //agregar datos de aceleracion 
    jsonDocument["accelX"] = faX;
    jsonDocument["accelY"] = faY;
    jsonDocument["accelZ"] = faZ;

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
}