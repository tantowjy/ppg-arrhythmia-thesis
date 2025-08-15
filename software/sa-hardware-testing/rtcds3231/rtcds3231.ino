/*
    Real-Time Clock (RTC)

    Hardware Testing Information:
    - Board   : Seeed Studio XIAO ESP32S3 Series
    - Module  : RTC DS3231
*/

#include <Wire.h>
#include "RTClib.h"

RTC_DS3231 rtc;

void setup() {
    Serial.begin(115200);
    delay(1000);

    // Use default I2C pins (GPIO 21 for SDA, GPIO 22 for SCL)
    Wire.begin();  // No parameters needed for default

    if (!rtc.begin()) {
        Serial.println("Couldn't find RTC. Check connections!");
        while (1);
    }

    if (rtc.lostPower()) {
        Serial.println("RTC lost power, setting time to compile time.");
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }

    Serial.println("RTC is running.");
}

void loop() {
    DateTime now = rtc.now();

    Serial.print("Time: ");
    Serial.print(now.year()); Serial.print('/');
    Serial.print(now.month()); Serial.print('/');
    Serial.print(now.day()); Serial.print(" ");
    Serial.print(now.hour()); Serial.print(':');
    Serial.print(now.minute()); Serial.print(':');
    Serial.println(now.second());

    delay(1000);
}