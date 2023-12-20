#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

#define DHTPIN 7          // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11     // DHT sensor type

DHT dht(DHTPIN, DHTTYPE);

// Define pins for 16x2 LCD, relay, fans, current sensor, and heating elements
const int relayHeaterPin = 2;
const int relayFanPin = 3;
const int fanPin = 4;
const int heaterPin = 5;
const int currentSensorPin = A1;  // Analog pin for the temperature control system current sensor
const int acs712Pin = A3;  // Analog input pin connected to the ACS712 sensor
const int contrast = 0;          // Adjust the contrast for your LCD

// LCD configuration
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Define parameters
const int targetTemp = 25;       // Set your desired target temperature
const int tempTolerance = 1;     // Set temperature tolerance
const int maxTemp = 35;          // Maximum allowable temperature
const int minTemp = 15;          // Minimum allowable temperature
const float maxCurrent = 2.0;    // Maximum allowable current (adjust based on your components)
const int samples = 500;  // Number of samples to take for each ACS712 reading
const float voltageRef = 1.5;  // Reference voltage for the ACS712 sensor
const float acsSensitivity = 0.185;  // Sensitivity factor for ACS712 5A sensor (in V/A)

// Variables for power and energy calculation
float voltage = 220.0;  // Assuming a constant voltage of 220V
unsigned long previousMillis = 0;
unsigned long interval = 1000;  // Update every 1 second
float accumulatedEnergy = 0.0;

int rawValue = 0;

// Function to read and calculate the AC current
float measureACCurrent() {
  float sum = 0;
  for (int i = 0; i < samples; i++) {
    rawValue = analogRead(acs712Pin);
    float voltage = (rawValue / 1023.0) * voltageRef;
    float current = (voltage - voltageRef / 2) / acsSensitivity;
    sum += sq(current);
    delay(1);
  }
  return sqrt(sum / samples);
}

void setup() {
  // Initialize LCD
  lcd.begin(16, 2);
  lcd.setBacklight(LOW);
  lcd.setContrast(contrast);

  // Initialize DHT sensor
  dht.begin();

  // Initialize relay, fans, heating elements, and current sensor pins
  pinMode(relayHeaterPin, OUTPUT);
  pinMode(relayFanPin, OUTPUT);
  pinMode(fanPin, OUTPUT);
  pinMode(heaterPin, OUTPUT);

  Serial.begin(9600);  // Initialize serial communication
}

void loop() {
  // Read temperature and humidity
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Read current for temperature control system
  float currentTempControl = readCurrent();

  // Read AC current using ACS712 sensor
  float currentACS712 = measureACCurrent();

  // Calculate power and energy consumption for temperature control system
  float powerTempControl = voltage * currentTempControl;
  unsigned long currentMillis = millis();
  unsigned long elapsedTime = currentMillis - previousMillis;
  float energyTempControl = (powerTempControl / 1000.0) * (elapsedTime / 3600000.0);  // Convert from Wh to kWh

  // Update accumulated energy for temperature control system
  accumulatedEnergy += energyTempControl;
  previousMillis = currentMillis;

  // Display values on LCD for temperature control system
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temperature);
  lcd.print(" C  ");

  lcd.setCursor(0, 1);
  lcd.print("Humidity: ");
  lcd.print(humidity);
  lcd.print("% ");

  // Display power, voltage, current, and energy consumption for temperature control system
  lcd.setCursor(0, 2);
  lcd.print("Power: ");
  lcd.print(powerTempControl);
  lcd.print("W ");

  lcd.setCursor(0, 3);
  lcd.print("Energy: ");
  lcd.print(accumulatedEnergy);
  lcd.print("kWh ");

  // Check temperature and control relay, fans, and heating elements
  if (temperature < targetTemp - tempTolerance) {
    digitalWrite(relayHeaterPin, HIGH);  // Turn on heating element relay
    digitalWrite(relayFanPin, LOW);      // Turn off fan relay
    digitalWrite(fanPin, LOW);            // Turn off fan
    digitalWrite(heaterPin, HIGH);        // Turn on heating element
  } else if (temperature > targetTemp + tempTolerance) {
    digitalWrite(relayHeaterPin, LOW);   // Turn off heating element relay
    digitalWrite(relayFanPin, HIGH);      // Turn on fan relay
    digitalWrite(fanPin, HIGH);           // Turn on fan
    digitalWrite(heaterPin, LOW);         // Turn off heating element
  } else {
    digitalWrite(relayHeaterPin, LOW);   // Turn off heating element relay
    digitalWrite(relayFanPin, LOW);      // Turn off fan relay
    digitalWrite(fanPin, LOW);           // Turn off fan
    digitalWrite(heaterPin, LOW);         // Turn off heating element
  }

  // Check for safety limits for temperature control system
  if (temperature > maxTemp || currentTempControl > maxCurrent) {
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

  // Display AC current using ACS712 sensor
  lcd.setCursor(0, 4);
  lcd.print("AC Current: ");
  lcd.print(currentACS712);
  lcd.print("A ");

  // Delay for stability
  delay(1000);
}

float readCurrent() {
  // Read current from ACS712 sensor for temperature control system
  int sensorValue = analogRead(currentSensorPin);
  float current = (sensorValue - 512.0) / 1024.0 * 5.0;  // Assumes ACS712 with 5A range
  return current;
}
