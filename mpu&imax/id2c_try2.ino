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


//Import All reqired libaries for I2C, MAX30102 Sensor, MPU6050 sensor
#include "MPU6050.h"
#include <stdio.h>
#include <WiFi.h>
#include <Wire.h> // used for both MPU6050  MAX30105 
#include "MAX30105.h"
#include "spo2_algorithm.h"
#include <ArduinoJson.h>

MPU6050 gyro;


const int MPU6050_addr = 0x68; // MPU6050 sensor configured on address 0x68 for I2C
int16_t AccX, AccY, AccZ, Temp, GyroX, GyroY, GyroZ;
const int numLect = 200;
int gyroData[numLect], acelData[numLect], aceGData[numLect];

WiFiClient client;
MAX30105 particleSensor;

const int MAX30102_adrr = 0x57;
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

  // Initialize sensors
  if(!Wire.begin(0x68))
  {
    Serial.println(F("MPU6050 was not found. Please check wiring/power."));
    while (1);
  }

  if (!Wire.begin(0x57))
  {
    Serial.println(F("MAX30105 was not found. Please check wiring/power."));
    while (1);
  }
  
  Wire.begin();
  Wire.beginTransmission(MPU6050_addr);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);

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
  Wire.beginTransmission(MPU6050_addr);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU6050_addr, 14);
  AccX = Wire.read() << 8 | Wire.read();
  AccY = Wire.read() << 8 | Wire.read();
  AccZ = Wire.read() << 8 | Wire.read();
  Temp = Wire.read() << 8 | Wire.read();
  GyroX = Wire.read() << 8 | Wire.read();
  GyroY = Wire.read() << 8 | Wire.read();
  GyroZ = Wire.read() << 8 | Wire.read();


}