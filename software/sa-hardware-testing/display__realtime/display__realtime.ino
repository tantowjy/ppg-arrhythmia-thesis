/*
    Real-Time Clock Display using NTP Server

    Hardware Testing Information:
    - Board         : Seeed Studio XIAO ESP32S3 Series
    - Display Module: 1.77" 160x128 ST7735 Display
*/

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <WiFi.h>
#include "time.h"

// TFT display pins
#define TFT_CS   3
#define TFT_RST  2
#define TFT_DC   44

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// WiFi credentials
const char* ssid     = "SSID";
const char* password = "PASSWORD";

// NTP settings
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 25200; // UTC+7

void setup() {
    Serial.begin(115200);

    // Init display
    tft.initR(INITR_BLACKTAB);
    tft.setRotation(3);
    tft.fillScreen(ST77XX_BLACK);

    // Setup WiFi
    WiFi.begin(ssid, password);
    Serial.print("\nConnecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        
        // Draw
        tft.fillScreen(ST77XX_BLACK);
        tft.setTextColor(ST77XX_WHITE);
        tft.setTextSize(1);
        tft.setCursor(25, 60);
        tft.print("Connecting to WiFi");
    }
    Serial.println("\nWiFi connected.");

    // Draw 
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(1);
    tft.setCursor(25, 60);
    tft.print("WiFi connected.");

    // Init NTP
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    struct tm timeinfo;
    while (!getLocalTime(&timeinfo)) {
        Serial.println("Waiting for NTP time...");
        
        // Draw
        tft.fillScreen(ST77XX_BLACK);
        tft.setTextColor(ST77XX_WHITE);
        tft.setTextSize(1);
        tft.setCursor(25, 60);
        tft.print("Waiting for NTP time...");
        delay(500);
    }

    // Optionally disable WiFi after time is obtained
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
}

void loop() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time");
        delay(1000);
        return;
    }

    // Format time
    char timeStr[9]; // HH:MM:SS
    strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);

    // Format date
    char dateStr[20]; // e.g., Jun 16, 2025
    strftime(dateStr, sizeof(dateStr), "%b %d, %Y", &timeinfo);

    // Clear screen and draw time
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextColor(ST7735_CYAN);

    // Draw large time
    tft.setTextSize(2);
    tft.setCursor(30, 40);
    tft.print(timeStr);

    // Draw small date
    tft.setTextSize(1);
    tft.setCursor(45, 75);
    tft.print(dateStr);

    delay(1000);
}