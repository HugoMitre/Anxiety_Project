#ifndef RTC_DATETIME_H
#define RTC_DATETIME_H

#include <Arduino.h> // O cualquier otra biblioteca necesaria

void extractTimeFromJson(const String& json, int& year, int& month, int& day, int& hour, int& minute, int& second);

#endif // RTC_DATETIME_H