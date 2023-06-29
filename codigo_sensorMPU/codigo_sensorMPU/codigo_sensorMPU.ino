#include "I2Cdev.h"
#include "MPU6050.h"
#include "Wire.h"
 
MPU6050 S;
// S es el objeto definido

int ax, ay, az;
float g = 9.81;
float rax,ray,raz;

void setup(){
  Serial.begin(9600);    //Iniciando USART
  Wire.begin();          //Iniciando comunicación I2C  
  S.initialize();    //Iniciando el sensor

  if (S.testConnection()) Serial.println("Sensor iniciado correctamente");
  else Serial.println("Error al iniciar el sensor");
}
 

void loop() {
  // Lectura de las aceleraciones
  S.getAcceleration(&ax, &ay, &az);
  // parametros por referencia
  rax = ax*(2*g)/32767;
  ray = ay*(2*g)/32767;
  raz = az*(2*g)/32767;

  //Mostrar las lecturas separadas por un [tab]
  Serial.print("a[x y z]:\t");
  // g[x y z] -> son las variables del giroscopio
  Serial.print(ax); Serial.print("\t");
  Serial.print(ay); Serial.print("\t");
  Serial.print(az); Serial.print("\t \t");
  Serial.print("m/s^2 [x y z]:\t");
  Serial.print(rax); Serial.print("\t");
  Serial.print(ray); Serial.print("\t");
  Serial.print(raz); Serial.print("\t \t");
  Serial.print("\n");
  
  delay(100);

}

// el valor que entrega es entre -32768 a 32767, siendo el mayor 32767 (positivos), pq es de 16 bits
// como manda rango de             -2g a   2g (o la g que este configurada)
