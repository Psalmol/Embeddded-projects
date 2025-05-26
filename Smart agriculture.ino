/*
............................................................................................................
This code is intended for Esp32.
Tested using Arduino IDE version 2.3.4, ESP board library 3.1.1.
The project and compilation of this code is done by PSALMOL ELECTRONICS, (Akadiri Olalekan Samuel), 
and other online resources.
.............................................................................................................
*/

#define BLYNK_TEMPLATE_ID "------------"
#define BLYNK_TEMPLATE_NAME "----------"
#define BLYNK_AUTH_TOKEN "-------------"
const char* ssid = "-------------------";
const char* pass = "-----------------";

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <ModbusMaster.h>
#include <OneWire.h>
#include <DallasTemperature.h>
BlynkTimer timer;
// RS485 Modbus NPK sensor connections

#define RXD2 16
#define TXD2 17
#define DE_RE 18
int nitrogen = 0, phosphorus = 0, potassium = 0;
#define SLAVE_ID 1
ModbusMaster node;

// DS18B20 temperature sensor
#define ONE_WIRE_BUS 19
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress tempSensor;

// Soil moisture sensor
#define SOIL_MOISTURE_PIN 34
#define PUMP_PIN 22
#define MOISTURE_THRESHOLD 30  
int soilMoisture;
float soilTemp;
String pestStatus;
bool pestDetected;

// Pest detection input (from Raspberry Pi)
#define PEST_DETECT_PIN 35

void preTransmission() {
  digitalWrite(DE_RE, HIGH);
}
void postTransmission() {
  digitalWrite(DE_RE, LOW);
}

void myTimer() {
  Blynk.virtualWrite(V1, nitrogen);
  Blynk.virtualWrite(V2, phosphorus);
  Blynk.virtualWrite(V3, potassium);
  Blynk.virtualWrite(V4, soilMoisture);
  Blynk.virtualWrite(V5, soilTemp);
  Blynk.virtualWrite(V6, pestDetected);
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);

  WiFi.begin(ssid, pass);
  WiFi.setSleep(false);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  pinMode(DE_RE, OUTPUT);
  digitalWrite(DE_RE, LOW);

  node.begin(SLAVE_ID, Serial2);
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);

  sensors.begin();
  sensors.getAddress(tempSensor, 0);
  sensors.setResolution(tempSensor, 9);

  pinMode(SOIL_MOISTURE_PIN, INPUT);
  pinMode(PEST_DETECT_PIN, INPUT_PULLDOWN);
  pinMode(PUMP_PIN, OUTPUT);
  digitalWrite(PUMP_PIN, LOW);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  timer.setInterval(10000L, myTimer);

  Serial.println("Smart Agriculture System Initialized.");
}

void loop() {
  // Read NPK sensor
  uint8_t result = node.readHoldingRegisters(0x001E, 3);
  if (result == node.ku8MBSuccess) {
    nitrogen = node.getResponseBuffer(0);
    phosphorus = node.getResponseBuffer(1);
    potassium = node.getResponseBuffer(2);
  } else {
    Serial.println("Error reading NPK sensor.");
  }

  // Read soil moisture
  int rawMoisture = analogRead(SOIL_MOISTURE_PIN);
  //Serial.println(rawMoisture);
  soilMoisture = map(rawMoisture, 4095, 800, 0, 100);
  soilMoisture = constrain(soilMoisture, 0, 100);

  // Read soil temperature
  sensors.requestTemperatures();
  soilTemp = sensors.getTempC(tempSensor);

  // Read pest detection status
  pestDetected = digitalRead(PEST_DETECT_PIN);
  pestStatus = pestDetected ? "Pest Detected" : "No Pest";

  // Print data to Serial Monitor
  Serial.printf("N: %d, P: %d, K: %d, Moisture: %d, Temp: %.2f, Pest: %s\n",
                nitrogen, phosphorus, potassium, soilMoisture, soilTemp, pestStatus.c_str());

  // Send data to Blynk

  // Water pump control
  if (soilMoisture < MOISTURE_THRESHOLD) {
    Serial.println("Low moisture detected. Activating pump...");
    digitalWrite(PUMP_PIN, HIGH);
    delay(1000);
    digitalWrite(PUMP_PIN, LOW);

    /* rawMoisture = analogRead(SOIL_MOISTURE_PIN);
    soilMoisture = map(rawMoisture, 4095, 800, 0, 100);
    soilMoisture = constrain(soilMoisture, 0, 100);
    Serial.printf("Updated Moisture Level: %d\n", soilMoisture);*/
  }

  Blynk.run();
  timer.run();
}
