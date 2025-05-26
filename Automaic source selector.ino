#include <LiquidCrystal.h>
const int rs = 18, en = 17, d4 = 16, d5 = 15, d6 = 14, d7 = 13;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Source Monitoring Pins 
const int X = A5, Y = A6, Z = A7;  // Analog pins for EEDC, Solar, and Generator

// Relay Pins for Sources 
const int RELAY_EEDC = 4, RELAY_SOLAR = 5, RELAY_GEN = 6;

// Relay Pins for Loads 
const int LOAD1 = 9, LOAD2 = 10, LOAD3 = 11;

// Generator Control Pin
const int GEN_START = 8;

// Voltage Threshold
const float THRESHOLD = 5.0;


float voltage[3];         
int genStartAttempts = 0;  
bool genError = false;     
unsigned long lcdUpdateTime = 0;
int lcdSourceIndex = 0;  

void setup() {
  lcd.begin(16, 2);
  lcd.print("Initializing...");
  delay(2000);
  lcd.clear();

  // Initialize Relay and Control Pins
  pinMode(RELAY_EEDC, OUTPUT);
  pinMode(RELAY_SOLAR, OUTPUT);
  pinMode(RELAY_GEN, OUTPUT);
  pinMode(GEN_START, OUTPUT);
  pinMode(LOAD1, OUTPUT);
  pinMode(LOAD2, OUTPUT);
  pinMode(LOAD3, OUTPUT);

  // Ensure all relays and loads are off initially
  digitalWrite(RELAY_EEDC, HIGH);
  digitalWrite(RELAY_SOLAR, HIGH);
  digitalWrite(RELAY_GEN, HIGH);
  digitalWrite(GEN_START, HIGH);
  digitalWrite(LOAD1, HIGH);
  digitalWrite(LOAD2, HIGH);
  digitalWrite(LOAD3, HIGH);
}

void loop() {
  
  readVoltages();

  int preferredSource = determinePreferredSource();

 
  switch (preferredSource) {
    case 0:  // No source available
      handleNoSource();
      break;
    case 1:  // EEDC
      switchToEEDC();
      break;
    case 2:  // Solar
      switchToSolar();
      break;
    case 3:  // Generator
      switchToGenerator();
      genStartAttempts = 0;
      genError = false;
      break;
  }

  // Step 4: Update LCD Display
  updateLCD();
}

// Function to read voltages from sources
void readVoltages() {
  voltage[0] = (analogRead(X) / 1023.0 * 5.0) / (0.2);  // EEDC
  voltage[1] = (analogRead(Y) / 1023.0 * 5.0) / 0.2;          // Solar
  voltage[2] = (analogRead(Z) / 1023.0 * 5.0) / 0.2;          // Generator
}


int determinePreferredSource() {
  if (voltage[0] > THRESHOLD) return 1;  // EEDC
  if (voltage[1] > THRESHOLD) return 2;  // Solar
  if (voltage[2] > THRESHOLD) return 3;  // Generator
  return 0;                              // No source available
}

void handleNoSource() {
  // Turn off all relays and loads
  digitalWrite(RELAY_EEDC, HIGH);
  digitalWrite(RELAY_SOLAR, HIGH);
  digitalWrite(RELAY_GEN, HIGH);
  digitalWrite(LOAD1, HIGH);
  digitalWrite(LOAD2, HIGH);
  digitalWrite(LOAD3, HIGH);
  
  if (genStartAttempts < 3 && !genError) {
    lcd.setCursor(0, 0);
    lcd.print("No Source Found ");
    lcd.setCursor(0, 1);
    lcd.print("Starting Gen... ");
    digitalWrite(GEN_START, LOW);
    delay(3000);  // Simulate generator start signal
    digitalWrite(GEN_START, HIGH);

    // Simulate generator failure
    if (voltage[2] <= THRESHOLD) {
      genStartAttempts++;
      if (genStartAttempts == 3) {
        genError = true;
      }
    } else {
      genStartAttempts = 0;
      genError = false;
    }
  }

  if (genError) {
    lcd.setCursor(0, 0);
    lcd.print("Gen Start Failed");
    lcd.setCursor(0, 1);
    lcd.print("Check System    ");
  }
}

// Switch to EEDC and control loads
void switchToEEDC() {
  lcd.setCursor(0, 0);
  lcd.print("Source: EEDC    ");
  digitalWrite(RELAY_EEDC, LOW);
  digitalWrite(RELAY_SOLAR, HIGH);
  digitalWrite(RELAY_GEN, HIGH);

  // Load control logic
  digitalWrite(LOAD1, LOW);
  digitalWrite(LOAD2, LOW);
  digitalWrite(LOAD3, LOW);
  //digitalWrite(LOAD4, HIGH);
  genStartAttempts = 0;
  genError = false;
}

// Switch to Solar and control loads
void switchToSolar() {
  lcd.setCursor(0, 0);
  lcd.print("Source: Solar   ");
  digitalWrite(RELAY_EEDC, HIGH);
  digitalWrite(RELAY_SOLAR, LOW);
  digitalWrite(RELAY_GEN, HIGH);

  // Load control logic
  digitalWrite(LOAD1, LOW);
  digitalWrite(LOAD2, LOW);
  digitalWrite(LOAD3, HIGH);
  genStartAttempts = 0;
  genError = false;
}

// Switch to Generator and control loads
void switchToGenerator() {
  lcd.setCursor(0, 0);
  lcd.print("Source: Generato");
  digitalWrite(RELAY_EEDC, HIGH);
  digitalWrite(RELAY_SOLAR, HIGH);
  digitalWrite(RELAY_GEN, LOW);

  // Load control logic
  digitalWrite(LOAD1, LOW);
  digitalWrite(LOAD2, HIGH);
  digitalWrite(LOAD3, HIGH);
  genStartAttempts = 0;
  genError = false;
}

// Update LCD Display with voltage levels
void updateLCD() {
  static int scrollIndex = 0;                                       // Tracks the current source being displayed
  const char* sources[] = { "EEDC: ", "Solar: ", "Gen: " };         // Labels for sources
  float sourceVoltages[] = { voltage[0], voltage[1], voltage[2] };  // Voltage array

  // Update the LCD every second
  if (millis() - lcdUpdateTime >= 1000) {
    lcd.clear();  // Clear the screen for scrolling effect
    lcd.setCursor(0, 0);
    lcd.print("Voltage Monitor");

    // Display the current source and voltage
    lcd.setCursor(0, 1);
    lcd.print(sources[scrollIndex]);
    lcd.print(sourceVoltages[scrollIndex], 2);
    lcd.print("V");

    // Advance to the next source
    scrollIndex++;
    if (scrollIndex >= 3) {
      scrollIndex = 0;  // Reset to the first source
    }

    lcdUpdateTime = millis();  // Update the timestamp
  }
}
