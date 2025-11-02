/**
 * TLA2528_AnalogReadSerial.ino
 * 
 * Reads analog input on a TLA2528 pin and prints values to Serial Monitor.
 * Compatible with Arduino Serial Plotter for visualization.
 * 
 * Platform Support:
 * - ESP32: Uses default Wire
 * - Arduino Nano R4: Auto-detects, uses Wire1 (Qwiic connector)
 * - Other Arduino: Uses Wire on A4/A5
 */

#include <Wire.h>
#include <TLA2528.h>

// I2C Address
const uint8_t TLA2528_ADDRESS = 0x10;

// Pin Definitions
const uint8_t POT_PIN = 3;  // Potentiometer analog input

// Auto-detect platform and select I2C bus
#if defined(ARDUINO_UNOR4_WIFI) || defined(ARDUINO_UNOR4_MINIMA)
  TLA2528 expander(Wire1);
  #define I2C_BUS Wire1
  #define BOARD_NAME "Arduino Nano R4"
#else
  TLA2528 expander;
  #define I2C_BUS Wire
  #define BOARD_NAME "Default (Wire)"
#endif

void setup() {
  Serial.begin(115200);

  #if defined(ARDUINO_UNOR4_WIFI) || defined(ARDUINO_UNOR4_MINIMA)
    // Wait for Serial on Nano R4
    for (auto startNow = millis() + 2500; !Serial && millis() < startNow; delay(500));
  #else
    delay(1000);
  #endif

  Serial.println("\n=== TLA2528 Analog Read Serial ===");
  Serial.print("Platform: ");
  Serial.println(BOARD_NAME);

  // Initialize I2C
  I2C_BUS.begin();
  I2C_BUS.setClock(400000); // Optional: 400 kHz

  Serial.println("Initializing TLA2528...");
  if (!expander.begin(TLA2528_ADDRESS)) {
    Serial.println("Failed to initialize TLA2528!");
    Serial.println("- Check wiring, address, pull-ups");
    while (1);  // Halt if chip not found
  }

  // Configure pin as analog input
  expander.pinMode(POT_PIN, IO_ANALOG);

  Serial.println("Reading analog values...");
}

void loop() {
  // Read 12-bit analog value (0-4095)
  int analogValue = expander.analogRead(POT_PIN);

  // Print to Serial Monitor/Plotter
  Serial.println(analogValue);

  // Delay for readable output
  delay(100);
}
