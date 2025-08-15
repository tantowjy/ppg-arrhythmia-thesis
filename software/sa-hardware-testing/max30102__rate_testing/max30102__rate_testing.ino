/*
    Sampling Rate Data Acquisition

    Hardware Testing Information:
    - Board         : Seeed Studio XIAO ESP32S3 Series
    - Sensor Module : MAX30102
*/

#include <Wire.h>
#include "MAX30105.h"

MAX30105 particleSensor;

void setup() {
    Serial.begin(115200);
    while(!Serial); //We must wait for Teensy to come online
    Serial.println("Max sample rate example");

    // Initialize sensor
    if (particleSensor.begin(Wire, I2C_SPEED_FAST) == false) //Use default I2C port, 400kHz speed
    {
        Serial.println("MAX30105 was not found. Please check wiring/power. ");
        while (1);
    }

    //Setup to sense up to 18 inches, max LED brightness
    byte ledBrightness = 0x1F;  //Options: 0=Off to 255=50mA
    byte sampleAverage = 16;    //Options: 1, 2, 4, 8, 16, 32
    byte ledMode = 2;           //Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
    int sampleRate = 1600;      //Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
    int pulseWidth = 411;       //Options: 69, 118, 215, 411
    int adcRange = 16384;       //Options: 2048, 4096, 8192, 16384

    particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange); //Configure sensor with these settings
}

void loop() {
    byte samplesTaken = 0;
    long startTime = micros();

    while(samplesTaken < 10) {
        particleSensor.check(); //Check the sensor, read up to 3 samples
        while (particleSensor.available()) //do we have new data?
        {
            samplesTaken++;
            particleSensor.getFIFOIR();
            particleSensor.nextSample(); //We're finished with this sample so move to next sample
        }
    }

    long endTime = micros();

    Serial.print("samples[");
    Serial.print(samplesTaken);

    Serial.print("] Hz[");
    Serial.print((float)samplesTaken / ((endTime - startTime) / 1000000.0), 2);
    Serial.print("]");

    Serial.println();
}