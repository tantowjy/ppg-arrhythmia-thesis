/*
    Multi Screen Display Switching with Tactile Button

    Hardware Testing Information:
    - Board : Seeed Studio XIAO ESP32S3 Series
*/

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

// Define pins for ST7735
#define TFT_CS   3
#define TFT_RST  2
#define TFT_DC   44

#define BUTTON_PIN    1
#define NUM_SCREENS   3
#define LONG_PRESS_TIME 2000

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

unsigned long buttonPressedTime = 0;
bool buttonState = HIGH;
bool lastButtonState = HIGH;
int currentScreen = 0;

void setup() {
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    Serial.begin(115200);

    // TFT init
    tft.initR(INITR_BLACKTAB);
    tft.setRotation(3);
    tft.fillScreen(ST77XX_BLACK);

    Serial.println("ESP32 Booted. Initializing display...");
    showScreen(currentScreen);
}

void loop() {
    buttonState = digitalRead(BUTTON_PIN);

    if (lastButtonState == HIGH && buttonState == LOW) {
        buttonPressedTime = millis();
        Serial.println("Button pressed");
    }

    if (lastButtonState == LOW && buttonState == HIGH) {
        unsigned long pressDuration = millis() - buttonPressedTime;
        Serial.print("Button released after ");
        Serial.print(pressDuration);
        Serial.println(" ms");

        if (pressDuration < LONG_PRESS_TIME) {
            currentScreen = (currentScreen + 1) % NUM_SCREENS;
            Serial.print("Short press: Switching to screen ");
            Serial.println(currentScreen + 1);
            showScreen(currentScreen);
        } else {
            Serial.println("Long press: Restarting ESP32...");
            tft.fillScreen(ST77XX_RED);
            tft.setTextColor(ST77XX_WHITE);
            tft.setTextSize(2);
            tft.setCursor(20, 60);
            tft.print("Restarting...");
            delay(1500);
            ESP.restart();
        }
    }

    lastButtonState = buttonState;
}

void showScreen(int screen) {
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextColor(ST77XX_CYAN);
    tft.setTextSize(2);
    tft.setCursor(25, 60);

    switch(screen) {
        case 0:
            Serial.println("Displaying: Screen 1");
            tft.print("Display 1");
            break;
        case 1:
            Serial.println("Displaying: Screen 2");
            tft.print("Display 2");
            break;
        case 2:
            Serial.println("Displaying: Screen 3");
            tft.print("Display 3");
            break;
        default:
            Serial.println("Error: Invalid screen number");
            break;
    }
}
