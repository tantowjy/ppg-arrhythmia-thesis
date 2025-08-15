/*
    Read Data from SD Card Storage

    Hardware Testing Information:
    - Board           : Seeed Studio XIAO ESP32S3 Series
    - Micro SD Module : Sandisk Ultra A1 64 GB, exFAT
*/

#include <SdFat.h>
#include <ArduinoJson.h>
#include <vector>

// SD Card setup
SdFs sd;
FsFile file;

// Struct to hold signal and label
struct SignalData {
    std::vector<double> signalVec;
    String label;
};

// Function declaration
SignalData readSignal(const char* filePath);

void setup() {
    Serial.begin(115200);
    while (!Serial);

    // Init SD card
    if (!sd.begin(SdSpiConfig(4, DEDICATED_SPI))) {
        Serial.println("SD init failed!");
        return;
    }
}

void loop() {
    // Loop through files data_0.json to data_9.json
    for (int i = 0; i < 10; i++) {
        char filePath[64];
        snprintf(filePath, sizeof(filePath), "/PPG_DATASET_4CLASS_TEST_50Hz/data_%d.json", i);

        SignalData data = readSignal(filePath);

        Serial.print("File: ");
        Serial.println(filePath);

        Serial.print("Label: ");
        Serial.println(data.label);

        Serial.println("Signal:");
        for (double v : data.signalVec) {
            Serial.print(v, 3);
            Serial.print(" ");
        }
        Serial.println("\n");

        delay(10000); // Avoid serial flooding
    }
}

// Reads signal and label from JSON file
SignalData readSignal(const char* filePath) {
    SignalData result;

    file = sd.open(filePath);
    if (!file) {
        Serial.print("Failed to open file: ");
        Serial.println(filePath);
        return result;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
        Serial.print("deserializeJson() failed for ");
        Serial.print(filePath);
        Serial.print(": ");
        Serial.println(error.f_str());
        return result;
    }

    JsonArray signal = doc[0];
    for (double v : signal) {
        result.signalVec.push_back(v);
    }

    result.label = doc[1].as<String>();

    return result;
}