/*
    Time Clock Display with Reset Button

    Hardware Testing Information:
    - Board         : Seeed Studio XIAO ESP32S3 Series
    - Display Module: 1.77" 160x128 ST7735 Display
*/

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

// Reset button
#define RESET_BUTTON_PIN 1 // IO1

// Define pins for ST7735
#define TFT_CS   3
#define TFT_RST  2
#define TFT_DC   44

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// Initial time (you could get this from an RTC module or NTP in real use)
int hour = 11;
int minute = 45;
int second = 0;
String date = "Jul 10, 2025";

void setup() {
    Serial.begin(115200);
    pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);

    tft.initR(INITR_BLACKTAB);
    tft.setRotation(3);
    tft.fillScreen(ST77XX_BLACK);
}

void loop() {
    // Reset Button
    if (digitalRead(RESET_BUTTON_PIN) == LOW) {
        delay(50); // Debounce
        if (digitalRead(RESET_BUTTON_PIN) == LOW) {
            Serial.println("Resetting ESP32...");
            ESP.restart(); // Software reset
        }
    }

    delay(100);

    // Format HH:MM:SS
    char timeStr[9];
    sprintf(timeStr, "%02d:%02d:%02d", hour, minute, second);

    // Clear and draw the screen
    uint16_t textColor = tft.color565(4, 21, 74);
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextColor(ST77XX_CYAN);

    // Draw large time
    tft.setTextSize(2);
    tft.setCursor(30, 40);
    tft.print(timeStr);

    // Draw small date
    tft.setTextSize(1);
    tft.setCursor(45, 75);
    tft.print(date);

    // Wait one second
    delay(900);

    // Increment time
    second++;
    if (second >= 60) {
        second = 0;
        minute++;
        if (minute >= 60) {
            minute = 0;
            hour++;
            if (hour >= 24) {
                hour = 0;
            }
        }
    }
}