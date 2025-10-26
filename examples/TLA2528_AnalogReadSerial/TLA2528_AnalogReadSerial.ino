/**
 * TLA2528_AnalogReadSerial.ino
 * 
 * Reads analog input on GPIO3 and prints values to Serial Monitor.
 * Compatible with Arduino Serial Plotter for visualization.
 * 
 * Hardware:
 * - Potentiometer connected to GPIO3
 * - I2C connections with 4.7kΩ pull-up resistors
 */

#include <Wire.h>
#include <TLA2528.h>

const uint8_t TLA2528_ADDRESS = 0x10;
const uint8_t POT_PIN = 3;  // Potentiometer on GPIO3

TLA2528 expander;

void setup() {
  Serial.begin(115200);
  delay(1000);  // Wait for Serial Monitor
  
  Serial.println("TLA2528 Analog Read Serial");
  
  Wire.begin();
  
  if (!expander.begin(TLA2528_ADDRESS)) {
    Serial.println("Failed to initialize TLA2528!");
    while (1);  // Halt if chip not found
  }
  
  // Configure pin as analog input
  expander.pinMode(POT_PIN, IO_ANALOG);
  
  Serial.println("Reading analog values...");
}

void loop() {
  // Read 12-bit analog value (0-4095)
  int analogValue = expander.analogRead(POT_PIN);
  
  // Print value to Serial Monitor/Plotter
  Serial.println(analogValue);
  
  // Delay for readable output
  delay(100);
}
