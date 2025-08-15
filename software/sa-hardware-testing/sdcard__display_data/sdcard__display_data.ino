/*
    Read Data from SD Card Storage and Display on The Screen

    Hardware Testing Information:
    - Board           : Seeed Studio XIAO ESP32S3 Series
    - Micro SD Module : Sandisk Ultra A1 64 GB, exFAT
    - Display Module  : 1.77" 160x128 ST7735 Display
*/

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <SdFat.h>

// Pin TFT
#define TFT_CS   3
#define TFT_DC   44
#define TFT_RST  2

// Pin SD Card
#define SD_CS    4

// Pin SPI
#define TFT_SCLK 7
#define TFT_MOSI 9
#define SD_MISO  8

// TFT object
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// SD object
SdFat sd;
File dataFile;

SPIClass mySPI;

void setup() {
    Serial.begin(115200);

    // Init SPI for TFT (optional if using default)
    mySPI.begin(TFT_SCLK, SD_MISO, TFT_MOSI, TFT_CS);

    // Init TFT
    tft.initR(INITR_BLACKTAB);
    tft.setRotation(3);
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextColor(ST77XX_CYAN);
    tft.setTextSize(1);

    // Init SD card
    if (!sd.begin(SD_CS, SD_SCK_MHZ(25))) {
        Serial.println(F("SD card init failed!"));
        tft.fillScreen(ST77XX_BLACK);
        tft.setCursor(10, 10);
        tft.println("SD card failed!");
        return;
    }
    Serial.println(F("SD card ready"));
    tft.fillScreen(ST77XX_BLACK);
    tft.setCursor(10, 10);
    tft.println("SD OK");

    // Open data.txt
    dataFile = sd.open("data.txt");
    if (!dataFile) {
        Serial.println(F("Failed to open data.txt"));
        tft.fillScreen(ST77XX_BLACK);
        tft.setCursor(10, 10);
        tft.println("Failed to open data.txt");
        return;
    }

    // Read and display each row of numbers
    int x = 10;
    tft.fillScreen(ST77XX_BLACK);
    while (dataFile.available()) {
        String line = dataFile.readStringUntil('\n');
        line.trim(); // delete space & newline
        Serial.println(line);
        tft.setCursor(x, 10);
        tft.println(line);
        x += 15; // change row
        delay(1000);
    }
    dataFile.close();
}

void loop() {
    // tidak ada loop
}