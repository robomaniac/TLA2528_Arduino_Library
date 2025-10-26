/**
 * TLA2528_DigitalReadWrite.ino
 * 
 * Digital input (button) controls digital output (LED).
 * Demonstrates basic GPIO functionality with debouncing.
 * 
 * Hardware:
 * - Button on GPIO0 with external 10kΩ pull-up resistor
 * - LED with 220Ω resistor on GPIO1
 * - I2C connections with 4.7kΩ pull-up resistors
 */

#include <Wire.h>
#include <TLA2528.h>

const uint8_t TLA2528_ADDRESS = 0x10;
const uint8_t BUTTON_PIN = 0;  // Requires external pull-up resistor!
const uint8_t LED_PIN = 1;

TLA2528 expander;

// Debouncing variables
uint32_t lastDebounceTime = 0;
bool lastButtonState = HIGH;
bool buttonState = HIGH;
const uint32_t debounceDelay = 50;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("TLA2528 Digital Read/Write");
  
  Wire.begin();
  
  if (!expander.begin(TLA2528_ADDRESS)) {
    Serial.println("Failed to initialize TLA2528!");
    while (1) delay(100);
  }
  
  // Configure pin modes
  expander.pinMode(BUTTON_PIN, INPUT);
  expander.pinMode(LED_PIN, OUTPUT);
  
  Serial.println("Press button to control LED");
}

void loop() {
  // Read button state with debouncing
  bool reading = expander.digitalRead(BUTTON_PIN);
  
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      
      // Button pressed (active LOW with pull-up)
      if (buttonState == LOW) {
        Serial.println("Button pressed - LED ON");
        expander.digitalWrite(LED_PIN, HIGH);
      } else {
        Serial.println("Button released - LED OFF");
        expander.digitalWrite(LED_PIN, LOW);
      }
    }
  }
  
  lastButtonState = reading;
  
  // Commit changes to device
  expander.update();
  
  delay(10);  // Small delay for stability
}
