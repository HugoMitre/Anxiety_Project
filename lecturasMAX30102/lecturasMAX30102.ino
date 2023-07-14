/*
  Hardware Connections (Breakoutboard to Arduino):
  -5V = 5V (3.3V is allowed)
  -GND = GND
  -SDA = A4 (or SDA)
  -SCL = A5 (or SCL) 
  -INT = Not connected

  Hardware Connections (ESP8266):
  -5V = 5V (3.3V is allowed)
  -GND = GND
  -SDA = D2 (or SDA)
  -SCL = D1 (or SCL) 
  -INT = Not connected

  Hardware Connections (ESP32):
  -5V = 5V (3.3V is allowed)
  -GND = GND
  -SDA = D21 (or SDA)
  -SCL = D22 (or SCL) 
  -INT = Not connected
 
  The MAX30105 Breakout can handle 5V or 3.3V I2C logic. We recommend powering the board with 5V
  but it will also run at 3.3V.
*/

#include <Wire.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"
#include <ArduinoJson.h>

// WIFI
#include <WiFi.h>
// #include <WiFiMulti.h>
#include <HTTPClient.h>

// REAL TIME (INTERN MODULE)
#include "rtc_datetime.h"
#include <ESP32Time.h>

// WIFI
const char* ssid = "INFINITUM2613_2.4";
const char* password =  "MGVfuQ2nwP";
const char* serverAddress = "192.168.1.77";
const int serverPort = 3000;
const char* apiEndpoint = "/api/reads";

// WIFI CONNECTION
WiFiClient client;

ESP32Time rtc;  // Creat a structure type ESP32Time
// Variables to get 
int year, month, day, hour, minute, second;

MAX30105 particleSensor;

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

void setup()
{
  Serial.begin(115200); // initialize serial communication at 115200 bits per second:
 
  // Initialize sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
  {
    Serial.println(F("MAX30105 was not found. Please check wiring/power."));
    while (1);
  }

  // Serial.println(F("Attach sensor to finger with rubber band. Press any key to start conversion"));
  // while (Serial.available() == 0) ; //wait until user presses a key
  // Serial.read();

  byte ledBrightness = 60; //Options: 0=Off to 255=50mA
  byte sampleAverage = 1; //Options: 1, 2, 4, 8, 16, _32
  byte ledMode = 2; //Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
  byte sampleRate = 800; //Options: 50, 100, 200, 400, 800, _1000, 1600, 3200
  int pulseWidth = 69; //Options: 69, 118, 215, 411
  int adcRange = 16384; //Options: 2048, 4096, 8192, 16384

  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange); //Configure sensor with these settings
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Connecting...");
  
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.print("Successful connection, my IP: ");
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

void loop()
{
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
    //dumping the first 15 sets of samples in the memory and shift the last 75 sets of samples to the top
    for (byte i = 15; i < 100; i++)
    {
      redBuffer[i - 15] = redBuffer[i];
      irBuffer[i - 15] = irBuffer[i];
    }

    //take 25 sets of samples before calculating the heart rate.
    for (byte i = 85; i < 100; i++)
    {
      while (particleSensor.available() == false) //do we have new data?
        particleSensor.check(); //Check the sensor for new data

      redBuffer[i] = particleSensor.getRed();
      irBuffer[i] = particleSensor.getIR();
      particleSensor.nextSample(); //We're finished with this sample so move to next sample

      //send samples and calculation result to terminal program through UART

      Serial.print("TIME: ");
      Serial.print(rtc.getTime() + "." + String(rtc.getMillis()));

      Serial.print(F(", RED: "));
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

      // REQUEST POST TO SAVE THE DATA IN DB
      if(WiFi.status() == WL_CONNECTED) {
        DynamicJsonDocument jsonDocument(1024);

        jsonDocument["time"] = rtc.getTime() + "." + String(rtc.getMillis());
        jsonDocument["red"] = String(redBuffer[i]);
        jsonDocument["ir"] = String(irBuffer[i]);
        jsonDocument["hr"] = String(heartRate);
        jsonDocument["validHR"] = String(validHeartRate);
        jsonDocument["spo2"] = String(spo2);
        jsonDocument["validSpo2"] = String(validSPO2);

        String jsonString;
        serializeJson(jsonDocument, jsonString);

        if (client.connect(serverAddress, serverPort)) {
          
          // POST Request
          String request = "POST " + String(apiEndpoint) + " HTTP/1.1\r\n";
          request += "Host: " + String(serverAddress) + "\r\n";
          request += "Content-Type: application/json\r\n";
          request += "Content-Length: " + String(jsonString.length()) + "\r\n";
          request += "Connection: close\r\n\r\n";
          request += jsonString;

          // send request
          client.print(request);

          // Request response
          while (client.available()) {
            String line = client.readStringUntil('\r');
            Serial.print(line);
          }

          // Close connection
          client.stop();
        } else {
          Serial.println("Error connecting the server");
        }
      }else{
        Serial.println("Error connecting WIFI");
      }
    }

    //After gathering 15 new samples recalculate HR and SP02
    maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
  }
}

