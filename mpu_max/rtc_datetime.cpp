#include "rtc_datetime.h"
#include <ArduinoJson.h>

void extractTimeFromJson(const String& json, int& year, int& month, int& day, int& hour, int& minute, int& second) {
  // Buscar la posición de la cadena "datetime"
  int datetimeIndex = json.indexOf("datetime\":\"");
  const char* datetime_str;

  if (datetimeIndex != -1) {
    // Obtener el índice del inicio del valor de datetime
    int valueIndex = datetimeIndex + 11;

    // Buscar el índice del siguiente "
    int closingQuoteIndex = json.indexOf("\"", valueIndex);

    if (closingQuoteIndex != -1) {
      // Extraer la subcadena que contiene el valor de datetime
      String datetimeValue = json.substring(valueIndex, closingQuoteIndex);
      
      // Convertir el valor de String a const char*
      datetime_str = datetimeValue.c_str();
    }
  }
  
  // Splitting the datetime string into separated components
  char* date_str = strtok((char*)datetime_str, "T");
  char* time_str = strtok(NULL, "T");
  char* _ = strtok(time_str, ".");
  
  // Getting date components and cast into integer
  year = atoi(strtok(date_str, "-"));
  month = atoi(strtok(NULL, "-"));
  day = atoi(strtok(NULL, "-"));
  
  // Getting time components and cast into integers
  hour = atoi(strtok(time_str, ":"));
  minute = atoi(strtok(NULL, ":"));
  second = atoi(strtok(NULL, ":"));
}