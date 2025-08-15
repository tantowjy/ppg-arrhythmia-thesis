#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <chrono>
#include <iomanip>
#include <SdFat.h>
#include <SPI.h>
#include <vector>
#include <Wire.h>
#include "bandpass50.h"
#include "dwtransform_db1.h"
#include "dwtransform_features.h"
#include "heartRate.h"
#include "MAX30105.h"
#include "MODEL_PPG_4C_DWT_MF_CNN_GRU_50.h"
#include "moving_average.h"
#include "normalize_minmax.h"
#include "ppg_morph_feature.h"
#include "reshape_for_cnn.h"
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"

// Prevent conflict with TFLite
#ifdef DEFAULT
  #undef DEFAULT
#endif

// Define SPI pins
#define TFT_SCLK 7
#define TFT_MOSI 9
#define SD_MISO  8

SPIClass mySPI;

// Define pins for ST7735
#define TFT_CS   3
#define TFT_DC   44
#define TFT_RST  2

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// Pin SD Card
#define SD_CS    4

// Define switch button
#define BUTTON_PIN    1
#define NUM_SCREENS   3
#define LONG_PRESS_TIME 2000

unsigned long buttonPressedTime = 0;
bool buttonState = HIGH;
bool lastButtonState = HIGH;
int currentScreen = 0;

// Initial time (could get this from an RTC module or NTP in real use)
int hour = 11;
int minute = 45;
int second = 0;
String date = "Aug 04, 2025";
unsigned long lastTimeUpdate = 0;

// SD Card setup
SdFs sd;
FsFile file;
char filename[40];

// MAX30102
MAX30105 particleSensor;

const byte RATE_SIZE = 4;   //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE];      //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0;          //Time at which the last beat occurred

int beatsPerMinute;
int bpmRaw;
int beatAvg;

long* irBuffer = nullptr;
int irIndex = 0;

unsigned long conditionStartTime = 0;
bool savingConditionActive = false;
bool dataSaved = false;
int dataIndex = 0;

unsigned long displayCounterStart = 0;
int displayCounterValue = 0;

// Struct to hold signal and label
struct SignalData {
  std::vector<double> signalVec;
  String label;
};

// ============= TFLite Start 
// Memory buffer
constexpr int kTensorArenaSize = 2048 * 1024;
uint8_t* tensor_arena = nullptr;

// Create model and interpreter
static tflite::MicroErrorReporter micro_error_reporter;
static tflite::ErrorReporter* error_reporter = &micro_error_reporter;
static const tflite::Model* model = nullptr;
static tflite::MicroInterpreter* interpreter = nullptr;
static TfLiteTensor* dwtmorph_input = nullptr; // dwtmorph_input
static TfLiteTensor* dwtcoeff_input = nullptr; // dwtcoeff_input
static TfLiteTensor* output = nullptr;

const char* label_map[] = {
  "Atrial_Fib",
  "Bradycardia",
  "Normal_Sinus_Rhythm",
  "Tachycardia"
};

const char* label_short[] = {"AF", "BR", "NS", "TC"};

const int label_count = sizeof(label_map) / sizeof(label_map[0]);
const int NUM_CLASSES = 4;

// ============= TFLite End

// Function declaration
void predictionFromSensor();
void predictionTimeFromStorage();
void predictionTestFromStorage();
int prediction(const std::vector<double>& signal);
void prepareFilename();
void showScreen(int screen);
void timeDisplay();
SignalData readSignal(const char* filePath);

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  Serial.begin(115200);

  // Init SPI for TFT
  mySPI.begin(TFT_SCLK, SD_MISO, TFT_MOSI, TFT_CS);

  // TFT init
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(3);
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);

  // Init SD card
  if (!sd.begin(SD_CS, SD_SCK_MHZ(25))) {
    Serial.println("SD init failed!");
    tft.setCursor(10, 10);
    tft.println("SD Card failed!");
    return;
  }
  tft.setCursor(10, 10);
  tft.println("SD Card OK");

  // MAX30102
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30105 not found.");
    tft.setCursor(10, 30);
    tft.println("MAX30102 failed!");
    while (1);
  }
  tft.setCursor(10, 30);
  tft.println("MAX30102 OK");

  byte ledBrightness = 0x1F;
  byte sampleAverage = 1; //Options: 1, 2, 4, 8, 16, 32
  byte ledMode = 2; //Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
  int sampleRate = 50; //Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
  int pulseWidth = 411; //Options: 69, 118, 215, 411
  int adcRange = 16384; //Options: 2048, 4096, 8192, 16384

  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);

  irBuffer = (long*) heap_caps_aligned_alloc(16, 2500 * sizeof(long), MALLOC_CAP_SPIRAM);

  // ============================ TFLite Setup Start 
  // Allocate tensor arena in PSRAM
  tensor_arena = (uint8_t*) heap_caps_aligned_alloc(16, kTensorArenaSize, MALLOC_CAP_SPIRAM);
  if (!tensor_arena) {
    Serial.println("Failed to allocate tensor_arena in PSRAM");
    while (1); // Halt if memory allocation fails
  }

  // Load model
  model = tflite::GetModel(MODEL_PPG_4C_DWT_MF_CNN_GRU_50_tflite);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    Serial.println("Model schema version mismatch.");
    while (1); // Halt if model version is invalid
  }

  // Setup interpreter
  static tflite::AllOpsResolver resolver;
  static tflite::MicroInterpreter static_interpreter(
    model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
  interpreter = &static_interpreter;

  // Allocate tensors
  if (interpreter->AllocateTensors() != kTfLiteOk) {
    Serial.println("\nFailed to allocate tensors.");
    while (1); // Halt on failure
  }

  // Access input and output tensors
  dwtcoeff_input = interpreter->input(0);
  dwtmorph_input = interpreter->input(1);
  output = interpreter->output(0);

  Serial.println("Model initialized successfully.");
  // ============================ TFLite Setup End 

  delay(1000);

  tft.fillScreen(ST7735_BLACK);
}

void loop() {
  buttonState = digitalRead(BUTTON_PIN);

  // Button pressed
  if (lastButtonState == HIGH && buttonState == LOW) {
    buttonPressedTime = millis();
    Serial.println("Button pressed");
  }

  // Button unpressed
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
      tft.fillScreen(ST77XX_BLACK);
      tft.setTextColor(ST77XX_RED);
      tft.setTextSize(1);
      tft.setCursor(30, 45);
      tft.print("Restarting...");
      delay(1500);
      ESP.restart();
    }
  }

  lastButtonState = buttonState;

  // Time based on millis
  if (millis() - lastTimeUpdate >= 1000) {  // each second
    lastTimeUpdate = millis();
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

    Serial.printf("Time: %02d:%02d:%02d\n", hour, minute, second);
    // Serial.print("IR=");
    // Serial.println(irValue);
    // Serial.print("BPM = ");
    // Serial.println(beatAvg);
  }

  if (currentScreen == 0) {
    predictionFromSensor();
    // predictionTimeFromStorage();
  }
  if (currentScreen == 1){
    predictionTimeFromStorage();
    // predictionFromSensor();
  }
  if (currentScreen == 2){
    timeDisplay();
  }

}

void predictionFromSensor() {
  particleSensor.check();

  while (particleSensor.available()) {
    long irValue = particleSensor.getFIFOIR();
    particleSensor.nextSample();

    if (irValue > 5000) {
      if (checkForBeat(irValue)) {
          long delta = millis() - lastBeat;
          lastBeat = millis();

          if (delta > 300 && delta < 2000) { // Filter BPM 30–200
              bpmRaw = 60.0 / (delta / 1000.0);

              // Callibration factor
              float calibrationFactor = 1.57;
              bpmRaw *= calibrationFactor;

              rates[rateSpot++] = (byte)bpmRaw;
              rateSpot %= RATE_SIZE;

              beatAvg = 0;
              for (byte x = 0; x < RATE_SIZE; x++) {
                  beatAvg += rates[x];
              }
              beatAvg /= RATE_SIZE;
          }
      }

      // Show to Display
      tft.fillRect(0, 0, 160, 40, ST77XX_BLACK);
      tft.setTextColor(ST7735_CYAN);

      tft.setTextSize(1);
      tft.setCursor(10, 5);
      tft.print("BPM: ");
      tft.setCursor(35, 5);
      tft.print(bpmRaw);
      tft.setCursor(10, 15);
      tft.print("Avg. BPM: ");
      tft.setCursor(65, 15);
      tft.print(beatAvg);


      if (!savingConditionActive) {
        conditionStartTime = millis();
        displayCounterStart = millis();
        displayCounterValue = 10;
        irIndex = 0;
        savingConditionActive = true;
        dataSaved = false;
        Serial.println("IR > 5000 condition started.");
      }

      if (irIndex < 500) {
        irBuffer[irIndex++] = irValue;

        // Time counter
        unsigned long elapsed = (millis() - displayCounterStart) / 1000;
        int remaining = 10 - elapsed;
        if (remaining < 0) remaining = 0;

        // Tampilkan counter di TFT
        tft.setTextSize(1);
        tft.setCursor(145, 5);
        tft.setTextColor(ST7735_YELLOW);
        tft.print(remaining);
      }

      if (irIndex >= 500 && !dataSaved) {
        // Calculate mean (DC) component
        long sum = 0;
        for (int i = 0; i < irIndex; i++) {
          sum += irBuffer[i];
        }
        long dc = sum / irIndex;

        // Save only AC values (signal - mean)
        prepareFilename();
        file = sd.open(filename, FILE_WRITE);
        if (file) {
          for (int i = 0; i < irIndex; i++) {
            long acValue = irBuffer[i] - dc;
            file.println(acValue);
          }
          file.close();
          Serial.print("AC data saved to: ");
          Serial.println(filename);
          Serial.print("Samples written this session: ");
          Serial.println(irIndex);

          std::vector<double> acValueBuffer;
          acValueBuffer.reserve(irIndex);

          for (int i = 0; i < irIndex; ++i) {
            acValueBuffer.push_back(static_cast<double>(irBuffer[i]));
          }
          
          // prediction
          int predicted_class = prediction(acValueBuffer);
          if (predicted_class != -1) {
            String predLabel;
            if (label_map[predicted_class] == "Atrial_Fib") {
              predLabel = " Atrial Fib";
            } else if (label_map[predicted_class] == "Bradycardia"){
              predLabel = "Bradycardia";
            } else if (label_map[predicted_class] == "Normal_Sinus_Rhythm"){
              predLabel = "Normal Sinus";
            } else if (label_map[predicted_class] == "Tachycardia"){
              predLabel = "Tachycardia";
            }

            Serial.print("Predicted: ");
            Serial.println(label_map[predicted_class]);
            Serial.println();

            // Show to Display
            // tft.fillScreen(ST77XX_BLACK);
            tft.fillRect(0, 30, 160, 70, ST77XX_BLACK);
            tft.setTextColor(ST7735_CYAN);

            tft.setTextSize(1);
            tft.setCursor(35, 55);
            tft.print("Heart Condition");

            // Draw small date
            tft.setTextSize(2);
            tft.setCursor(10, 75);
            tft.print(predLabel);

          }

          dataSaved = true;
        } else {
          Serial.println("Failed to open file.");
        }
        
        savingConditionActive = false;
        irIndex = 0;
      }
    } else {
      if (savingConditionActive && !dataSaved) {
        Serial.println("IR dropped before collecting 500 samples. Data discarded.");
      }
      savingConditionActive = false;
      irIndex = 0;
      dataSaved = false;

      // Show to Display
      tft.fillRect(0, 0, 160, 40, ST77XX_BLACK);
      tft.setTextColor(ST7735_CYAN);

      tft.setTextSize(1);
      tft.setCursor(10, 5);
      tft.print("BPM: ");
      tft.setCursor(35, 5);
      tft.print("--");

      tft.fillRect(0, 30, 160, 70, ST77XX_BLACK);
      tft.setTextColor(ST7735_CYAN);

      tft.setTextSize(1);
      tft.setCursor(35, 55);
      tft.print("Heart Condition");

      // Draw small date
      tft.setTextSize(2);
      tft.setCursor(70, 75);
      tft.print("--");
      
    }
  }
}

void predictionTimeFromStorage(){
  auto start = std::chrono::high_resolution_clock::now();

  predictionTestFromStorage();

  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  std::cout << "Elapsed time: " << duration.count() << " ms" << std::endl;
  
  int elapsed_time = duration.count();

  // Show to Display
  tft.setTextSize(1);
  tft.setCursor(5, 115);
  tft.print("Elapsed time (ms): ");
  tft.setCursor(112, 115);
  tft.print(elapsed_time);

  delay(60000);

  // currentScreen = 2;
}

void predictionTestFromStorage(){
  Serial.println("\nStarting prediction ... \n");

  double accuracy;
  int truePred = 0;
  int totalPrediction = 100;
  int confusionMatrix[NUM_CLASSES][NUM_CLASSES] = {0};

  // Loop through files
  for (int i = 0; i < totalPrediction; i++) {
    float pred_progress = ((i+1)/(float)totalPrediction) * 100;

    tft.fillScreen(ST77XX_BLACK);
    tft.setTextColor(ST7735_CYAN);
    tft.setTextSize(1);

    tft.setCursor(5, 5);
    tft.print("Progress: ");
    tft.setCursor(65, 5);
    tft.print(pred_progress);
    tft.setCursor(105, 5);
    tft.print("%");

    char filePath[64], filePathSort[32];
    snprintf(filePath, sizeof(filePath), "/PPG_DATASET_4CLASS_TEST_50Hz/data_%d.json", i);
    snprintf(filePathSort, sizeof(filePathSort), "data_%d.json", i);

    SignalData data = readSignal(filePath);

    Serial.print("File: ");
    Serial.println(filePath);

    tft.setCursor(5, 35);
    tft.print("File     : ");
    tft.setCursor(70, 35);
    tft.print(filePathSort);

    Serial.print("Expected: ");
    Serial.println(data.label);

    tft.setCursor(5, 45);
    tft.print("Expected : ");
    tft.setCursor(70, 45);
    tft.print(data.label);

    int predicted_class = prediction(data.signalVec);
    tft.setCursor(5, 55);
    tft.print("Predicted: ");
    tft.setCursor(70, 75);
    tft.print(predicted_class);

    if (predicted_class != -1) {
      Serial.print("Predicted: ");
      Serial.println(label_map[predicted_class]);
      Serial.println();

      tft.setCursor(5, 55);
      tft.print("Predicted: ");
      tft.setCursor(70, 55);
      tft.print(label_map[predicted_class]);
    }

    // Mapping label string to index
    int expected_class = -1;
    for (int j = 0; j < NUM_CLASSES; j++) {
      if (data.label == label_map[j]) {
        expected_class = j;
        break;
      }
    }

    if (expected_class != -1 && predicted_class != -1) {
      confusionMatrix[expected_class][predicted_class]++;
      if (expected_class == predicted_class) {
        truePred++;
      }
    }

    delay(100); // Avoid serial flooding
  }

  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_CYAN);
  tft.setTextSize(1);

  accuracy = ((double)truePred / totalPrediction) * 100;
  std::cout << std::fixed << std::setprecision(2);
  std::cout << "Accuracy: " << accuracy << " %\n";

  tft.setCursor(5, 100);
  tft.print("Accuracy     (%) : ");
  tft.setCursor(112, 100);
  tft.print(accuracy);

  // Print confusion matrix
  int cellW = 30;
  int cellH = 20;
  int offsetX = 30;
  int offsetY = 20;

  Serial.println("\nConfusion Matrix:");
  Serial.print("        ");
  for (int i = 0; i < NUM_CLASSES; i++) {
    Serial.print(label_map[i]);
    Serial.print("    ");

    tft.setCursor(offsetX + i * cellW + 5, 5);
    tft.print(label_short[i]);
  }
  Serial.println();
  for (int i = 0; i < NUM_CLASSES; i++) {
    Serial.print(label_map[i]);
    Serial.print("   ");

    tft.setCursor(0, offsetY + i * cellH + 5);
    tft.print(label_short[i]);

    for (int j = 0; j < NUM_CLASSES; j++) {
      Serial.print(confusionMatrix[i][j]);
      Serial.print("           ");

      tft.setCursor(offsetX + j * cellW + 5, offsetY + i * cellH + 5);
      tft.print(confusionMatrix[i][j]);
    }
    Serial.println();
  }
}

int prediction(const std::vector<double>& signal) {
  // Apply Band Pass Filter
  BandpassFilter filter(50.0); // fs = 50 Hz
  std::vector<double> bpf_filtered = filter.apply(signal);

  // Apply Moving Average Filter
  int window_size = 5;
  std::vector<double> ma_filtered = moving_average_filter(bpf_filtered, window_size);

  // Normalize the signal
  std::vector<double> normalized = normalize_signal(ma_filtered);

  // Apply DWT Coefficient
  std::vector<std::vector<double>> cD;
  std::vector<double> dwt_cA;

  get_dwt_coeffs(normalized, 4, cD, dwt_cA);  // Change the level 

  std::vector<float> dwt_cA_float;

  dwt_cA_float.reserve(dwt_cA.size());
  for (double val : dwt_cA) {
      dwt_cA_float.push_back(static_cast<float>(val));
  }

  // Apply DWT features
  std::vector<double> dwt_features;
  dwtransform(normalized, dwt_features);

  // Apply MF - Vector first peak
  double frequency = 50.0;
  std::vector<int> peaks;
  std::vector<FeatureSet> mf_features = extract_morph_features(normalized, frequency, peaks);
  std::vector<double> morph_features = morphological_features(mf_features);

  // Merge features
  std::vector<double> features = {};
  features.insert(features.end(), dwt_features.begin(), dwt_features.end());
  features.insert(features.end(), morph_features.begin(), morph_features.end());

  // Convert to Float
  std::vector<float> features_float;

  features_float.reserve(features.size());
  for (double val : features) {
      features_float.push_back(static_cast<float>(val));
  }

  /* ========================= Run inference ================================== */
  size_t morph_expected = dwtmorph_input->bytes / sizeof(float);
  size_t dwt_expected   = dwtcoeff_input->bytes / sizeof(float);

  if (features_float.size() != morph_expected) {
      Serial.printf("DWT and MF features input size mismatch! Expected %u, got %u\n",
                    (unsigned)morph_expected, (unsigned)features_float.size());
      return -1;
  }

  if (dwt_cA_float.size() != dwt_expected) {
      Serial.printf("DWT input size mismatch! Expected %u, got %u\n",
                    (unsigned)dwt_expected, (unsigned)dwt_cA_float.size());
      return -2;
  }

  

  // Copy vector into input tensor
  for (size_t i = 0; i < dwt_cA_float.size(); ++i) {
      dwtcoeff_input->data.f[i] = dwt_cA_float[i];
  }

  // Salin data Morph
  for (size_t i = 0; i < features_float.size(); ++i) {
      dwtmorph_input->data.f[i] = features_float[i];
  }

  // Run inference
  if (interpreter->Invoke() != kTfLiteOk) {
    Serial.println("Failed to invoke model.");
    return -1; // Error code
  }

  // Find the predicted class
  int predicted_class = 0;
  float max_prob = output->data.f[0];

  for (int i = 1; i < output->dims->data[1]; ++i) {
    float val = output->data.f[i];
    if (val > max_prob) {
      max_prob = val;
      predicted_class = i;
    }
  }

  return predicted_class;
}

void prepareFilename() {
  sprintf(filename, "data_%05d.txt", dataIndex);
  dataIndex++;
}

void showScreen(int screen) {
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_CYAN);
  tft.setTextSize(2);
  tft.setCursor(30, 40);

  if (screen == 0){
    predictionFromSensor();
    // predictionTimeFromStorage();
  } else if (screen == 1) {
    predictionTimeFromStorage();
    // predictionFromSensor();
  } else if (screen == 2) {
    timeDisplay();
  }
}

void timeDisplay(){
  // Format HH:MM:SS
  char timeStr[9];
  sprintf(timeStr, "%02d:%02d:%02d", hour, minute, second);

  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST7735_CYAN);

  // Draw large time
  tft.setTextSize(2);
  tft.setCursor(35, 40);
  tft.print(timeStr);

  // Draw small date
  tft.setTextSize(1);
  tft.setCursor(45, 75);
  tft.print(date);

  delay(1000);
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
