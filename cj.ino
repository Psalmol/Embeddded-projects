#include <ModbusMaster.h>

// Define RS485 transceiver control pins
#define RE 13  // Receiver enable pin
#define DE 14  // Driver enable pin

// Use Hardware Serial2 on the ESP32
// (adjust the RX/TX pins if needed; here, RX=16, TX=17)
#define MODBUS_BAUD 9600

// Create a ModbusMaster instance
ModbusMaster node;

// RS485 direction control functions
void preTransmission() {
  digitalWrite(DE, HIGH);  // Enable transmission
  digitalWrite(RE, HIGH);  // Disable reception (if RE is active LOW, reverse logic accordingly)
}

void postTransmission() {
  digitalWrite(DE, LOW);  // Disable transmission
  digitalWrite(RE, LOW);  // Enable reception
}

void setup() {
  Serial.begin(9600);                              // For debugging output
  Serial2.begin(MODBUS_BAUD, SERIAL_8N1, 16, 17);  // RS485 serial port

  // Initialize RS485 control pins
  pinMode(RE, OUTPUT);
  pinMode(DE, OUTPUT);
  digitalWrite(DE, LOW);  // Set default state to receive
  digitalWrite(RE, LOW);

  // Initialize the Modbus communication
  // Set the slave address to 3 (as indicated on the product nameplate)
  node.begin(3, Serial2);
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);

  Serial.println("ModbusMaster initialized for Soil NPK Sensor (Address 3).");
}

void loop() {
  /*int N, P, K;
  uint8_t result = node.readHoldingRegisters(0x001E, 3);
  Serial.print("result: ");
  Serial.println(result);
  //if (result == node.ku8MBSuccess) {
  // Serial.print("Response buffer length: ");

  //Serial.println(node.getResponseBufferLength());  // if supported, or count bytes manually
  for (uint8_t i = 0; i < 8; i++) {
    Serial.print("Register ");
    Serial.print(0x001E + i, HEX);
    Serial.print(": ");
    Serial.println(node.getResponseBuffer(i));
  }
  N = node.getResponseBuffer(0);
  P = node.getResponseBuffer(1);
  K = node.getResponseBuffer(2);  //
  //} else {
  //Serial.print("Modbus read error: 0x");
  // Serial.println(result, HEX);


  // Wait a bit before sending the next request
  delay(2000);*/
  scanRegisters();
}

void scanRegisters() {
  Serial.println("Scanning Holding Registers:");
  for (uint16_t addr = 0x0000; addr < 0x0030; addr++) {
    uint8_t res = node.readHoldingRegisters(addr, 1);
    if (res == node.ku8MBSuccess) {
      uint16_t val = node.getResponseBuffer(0);
      if (val != 0) {
        Serial.print("HReg 0x");
        Serial.print(addr, HEX);
        Serial.print(" = ");
        Serial.println(val);
      }
    } else {
      // Optionally print error for each address:
      // Serial.print("HReg 0x"); Serial.print(addr, HEX); Serial.print(" error: "); Serial.println(res, HEX);
    }
    delay(50);
  }

  Serial.println("Scanning Input Registers:");
  for (uint16_t addr = 0x0000; addr < 0x0030; addr++) {
    uint8_t res = node.readInputRegisters(addr, 1);
    if (res == node.ku8MBSuccess) {
      uint16_t val = node.getResponseBuffer(0);
      if (val != 0) {
        Serial.print("IReg 0x");
        Serial.print(addr, HEX);
        Serial.print(" = ");
        Serial.println(val);
      }
    } else {
      // Optionally print error for each address:
      // Serial.print("IReg 0x"); Serial.print(addr, HEX); Serial.print(" error: "); Serial.println(res, HEX);
    }
    delay(50);
  }
}
