/*
    Real-Time BPM

    Hardware Testing Information:
    - Board          : Seeed Studio XIAO ESP32S3 Series
    - Sensor Module  : MAX30102
    - Display Module : 1.77" 160x128 ST7735 Display
*/

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"

// Define pins for ST7735
#define TFT_CS   3
#define TFT_RST  2
#define TFT_DC   44

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

MAX30105 particleSensor;

const byte RATE_SIZE = 4;   //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE];      //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0;          //Time at which the last beat occurred

float beatsPerMinute;
int beatAvg;

// Initial time (you could get this from an RTC module or NTP in real use)
int hour = 14;
int minute = 35;
int second = 0;
String date = "Jul 15, 2025";

void setup() {
    Serial.begin(115200);
    Serial.println("Initializing...");

    // Display
    tft.initR(INITR_BLACKTAB);
    tft.setRotation(3);
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextColor(ST77XX_CYAN);
    tft.setTextSize(1);
    tft.setCursor(25, 60);
    tft.println("Initializing...");

    // Initialize sensor
    if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
    {
        Serial.println("MAX30105 was not found. Please check wiring/power. ");
        while (1);
    }
    Serial.println("Place your index finger on the sensor with steady pressure.");

    particleSensor.setup(); //Configure sensor with default settings
    particleSensor.setPulseAmplitudeRed(0x1A); //Turn Red LED to low to indicate sensor is running
    particleSensor.setPulseAmplitudeGreen(0); //Turn off Green LED

    // Display
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextColor(ST77XX_CYAN);
    tft.setTextSize(1);
    tft.setCursor(25, 60);
    tft.println("System Ready");
}

void loop() {
    long irValue = particleSensor.getIR();

    static unsigned long lastPrintTime = 0;

    if (checkForBeat(irValue) == true) {
        // We sensed a beat!
        long delta = millis() - lastBeat;
        lastBeat = millis();

        beatsPerMinute = 60 / (delta / 1000.0);

        if (beatsPerMinute < 255 && beatsPerMinute > 20) {
            rates[rateSpot++] = (byte)beatsPerMinute; // Store this reading
            rateSpot %= RATE_SIZE;

            // Calculate average BPM
            beatAvg = 0;
            for (byte x = 0 ; x < RATE_SIZE ; x++) {
                beatAvg += rates[x];
            }
            beatAvg /= RATE_SIZE;
        }
    }

    // Print BPM every 1 second
    if (millis() - lastPrintTime >= 1000) {
        lastPrintTime = millis();

        // Format HH:MM:SS
        char timeStr[9];
        sprintf(timeStr, "%02d:%02d:%02d", hour, minute, second);

        // Clear and draw the screen
        // uint16_t textColor = tft.color565(4, 21, 74);
        tft.fillScreen(ST77XX_BLACK);
        tft.setTextColor(ST7735_CYAN);

        // Draw time
        tft.setTextSize(1);
        tft.setCursor(5, 5);
        tft.print(date);

        tft.setTextSize(1);
        tft.setCursor(110, 5);
        tft.print(timeStr);

        // Draw BPM
        tft.setTextSize(2);
        tft.setCursor(10, 40);
        tft.print("BPM");

        Serial.print("IR=");
        Serial.println(irValue);

        if (irValue < 50000) {
            Serial.print(" -- ");

            tft.setTextSize(4);
            tft.setCursor(45, 70);
            tft.print(" -- ");
        } else {
            Serial.print("BPM = ");
            Serial.print(beatAvg);

            tft.setTextSize(4);
            tft.setCursor(65, 70);
            tft.print(beatAvg);
        }

        Serial.println();

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
}