/**
 * TLA2528_AnalogReadSerial.ino
 * 
 * Reads analog input on a TLA2528 pin and prints values to Serial Monitor.
 * Compatible with Arduino Serial Plotter for visualization.
 * 
 * Platform Support:
 * - ESP32: Uses default Wire on any I2C pins
 * - Arduino Nano R4: Auto-detects, uses Wire1 (Qwiic connector)
 * - Other Arduino: Uses Wire on A4 (SDA) / A5 (SCL)
 */

#include <Wire.h>
#include <TLA2528.h>

const uint8_t TLA2528_ADDRESS = 0x10;
const uint8_t POT_PIN = 3;  // Potentiometer analog input

// Auto-detect platform and choose I2C bus
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
    for (auto startNow = millis() + 2500; !Serial && millis() < startNow; delay(500));
  #else
    delay(2000);
  #endif

  Serial.println("\n=== TLA2528 Analog Read Serial ===");
  Serial.print("Platform: ");
  Serial.println(BOARD_NAME);

  I2C_BUS.begin();
  I2C_BUS.setClock(400000);  // 400 kHz for fast I2C

  Serial.println("Connecting to TLA2528...");

  if (!expander.begin(TLA2528_ADDRESS)) {
    Serial.println("ERROR: TLA2528 not detected!");
    Serial.println("- Check I2C wiring and address");
    Serial.println("- Ensure 4.7kΩ pull-ups on SDA/SCL");
    while (1) delay(1000);
  }

  // Configure potentiometer pin as analog input
  expander.pinMode(POT_PIN, IO_ANALOG);

  Serial.println("✓ Setup complete! Reading analog values...");
}

void loop() {
  // Read 12-bit analog value (0-4095)
  int analogValue = expander.analogRead(POT_PIN);

  // Print to Serial Monitor/Plotter
  Serial.println(analogValue);

  // Small delay for readable output
  delay(100);
}
