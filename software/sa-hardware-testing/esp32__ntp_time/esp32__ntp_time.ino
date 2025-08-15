/*
    ESP32 Real-Time using NTP Server

    Hardware Testing Information:
    - Board : Seeed Studio XIAO ESP32S3 Series
*/

#include <WiFi.h>
#include "time.h"

const char* ssid     = "SSID";
const char* password = "PASSWORD";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 25200;  // UTC+7

void setup(){
    Serial.begin(115200);

    // Connect to Wi-Fi
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected.");
    
    // Init and get the time
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    printLocalTime();

    //disconnect WiFi as it's no longer needed
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
}

void loop(){
    delay(1000);
    printLocalTime();
}

void printLocalTime(){
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo)){
        Serial.println("Failed to obtain time");
        return;
    }

    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}