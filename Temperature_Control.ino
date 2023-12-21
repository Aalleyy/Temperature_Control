#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define DHTPIN 7          // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11     // DHT sensor type

DHT dht(DHTPIN, DHTTYPE);

// Define pins for 16x2 LCD, relay, fans, and heating elements
const int relayHeaterPin = 2;
const int relayFanPin = 3;
const int currentSensorPin = A1;    // Analog pin for the current sensor
const int contrast = 0;             // Adjust the contrast for your LCD

// LCD configuration
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Define parameters
int targetTemp = 25;               // Set your initial desired target temperature
const int maxTemp = 35;            // Maximum allowable temperature
const int minTemp = 15;            // Minimum allowable temperature
const float maxCurrent = 2.0;      // Maximum allowable current (adjust based on your components)
const float ACS_SENSITIVITY = 0.185;  // Sensitivity factor for ACS712 sensor (adjust according to your module)

// Variables for power and energy calculation
float voltage = 12.0;  // A constant voltage supply of 12V
unsigned long previousMillis = 0;
unsigned long interval = 1000;  // Update every 1 second
float accumulatedEnergy = 0.0;

// Function to read and calculate the DC current
float measureCurrent() {
  int sensorValue = analogRead(currentSensorPin);
  float voltage = (sensorValue / 1023.0) * 5.0;
  float current = voltage / ACS_SENSITIVITY;  // ACS712 sensitivity factor
  return current;
}

void setup() {
  // Initialize LCD
  lcd.begin(16, 2);
  lcd.setBacklight(HIGH);
  lcd.setContrast(contrast);

  // Initialize DHT sensor
  dht.begin();

  // Initialize relay, fans, heating elements, and current sensor pins
  pinMode(relayHeaterPin, OUTPUT);
  pinMode(relayFanPin, OUTPUT);

  Serial.begin(9600);  // Initialize serial communication
  Serial.println("Enter the desired target temperature: ");
}

void loop() {
  // Read temperature and humidity
  float temperature = dht.readTemperature();

  // Read current for temperature control system
  float currentTempControl = measureCurrent();

  // Calculate power and energy consumption for temperature control system
  float powerTempControl = voltage * currentTempControl;
  unsigned long currentMillis = millis();
  unsigned long elapsedTime = currentMillis - previousMillis;
  float energyTempControl = (powerTempControl / 1000.0) * (elapsedTime / 3600000.0);  // Convert from Wh to kWh

  // Update accumulated energy for temperature control system
  accumulatedEnergy += energyTempControl;
  previousMillis = currentMillis;

  // Display temperature, current, and voltage
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temperature);
  lcd.print(" C  ");

  lcd.setCursor(0, 1);
  lcd.print("I: ");
  lcd.print(currentTempControl);
  lcd.print("A ");

  lcd.setCursor(8, 1);
  lcd.print("V: ");
  lcd.print(voltage);
  lcd.print("V ");

  // Delay for stability
  delay(3000);  // Display temperature current, and voltage for 3 seconds

  // Display energy consumption and power
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Power: ");
  lcd.print(powerTempControl);
  lcd.print("W ");
  
  lcd.setCursor(0, 1);
  lcd.print("Energy: ");
  lcd.print(accumulatedEnergy);
  lcd.print("kWh ");

  

  // Check temperature and control relay, fans, and heating elements
  if (temperature < targetTemp) {
    digitalWrite(relayHeaterPin, HIGH);  // Turn on heating element relay
    digitalWrite(relayFanPin, LOW);      // Turn off fan relay
  } else if (temperature > targetTemp) {
    digitalWrite(relayHeaterPin, LOW);   // Turn off heating element relay
    digitalWrite(relayFanPin, HIGH);      // Turn on fan relay
  } else {
    digitalWrite(relayHeaterPin, LOW);   // Turn off heating element relay
    digitalWrite(relayFanPin, LOW);      // Turn off fan relay
  }

  // Check for safety limits for temperature control system
  if (temperature == targetTemp){
    lcd.clear();
    lcd.print("Temperature Set");
  } else if (temperature > maxTemp || currentTempControl > maxCurrent) {
    // Implement safety measures for high temperature or current
    // (e.g., turn off heating element, turn off fan, display warning)
    lcd.clear();
    lcd.print("High Temp/Current Warning");
  } else if (temperature < minTemp) {
    // Implement safety measures for low temperature
    // (e.g., turn off fan, display warning)
    lcd.clear();
    lcd.print("Low Temp Warning");
  }

  // Check if there's new data from the user
  if (Serial.available() > 0) {
    targetTemp = Serial.parseInt();  // Read user input for target temperature
    Serial.println("Target temperature updated to: " + String(targetTemp) + " C");
    lcd.clear();
    lcd.print("Target Temp Updated");
  }

  // Delay for stability
  delay(3000);
}
