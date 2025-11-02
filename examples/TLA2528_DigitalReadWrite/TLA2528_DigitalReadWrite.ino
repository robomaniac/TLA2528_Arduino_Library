/**
 * TLA2528_DigitalReadWrite.ino
 * 
 * Simple digital I/O example with button and LED control.
 * 
 * Hardware Setup:
 * - GPIO0: Button with pull-up resistor (active LOW)
 * - GPIO1: LED with 220Ω resistor
 * - GPIO2: PWM LED with 220Ω resistor  
 * - GPIO3: Potentiometer (0-3.3V)
 * 
 * Platform Support:
 * - ESP32: Uses default Wire on any I2C pins
 * - Arduino Nano R4: Auto-detects, uses Wire1 (Qwiic connector, 3.3V)
 * - Other Arduino: Uses Wire on A4 (SDA) / A5 (SCL)
 */

#include <Wire.h>
#include <TLA2528.h>

const uint8_t TLA2528_ADDRESS = 0x10;
const uint8_t BUTTON_PIN = 0;
const uint8_t LED_PIN = 1;
const uint8_t PWM_LED_PIN = 2;
const uint8_t POT_PIN = 3;

// Auto-detect platform and use appropriate I2C bus
#if defined(ARDUINO_UNOR4_WIFI) || defined(ARDUINO_UNOR4_MINIMA)
  TLA2528 expander(Wire1);
  #define I2C_BUS Wire1
  #define BOARD_NAME "Arduino Nano R4"
#else
  TLA2528 expander;
  #define I2C_BUS Wire
  #define BOARD_NAME "Default (Wire)"
#endif

uint32_t lastReadTime = 0;
const uint32_t READ_INTERVAL = 50;
int currentBrightness = 0;

// Button state tracking for toggle
bool lastButtonState = HIGH;
bool ledState = LOW;

void setup() {
  Serial.begin(115200);
  
  #if defined(ARDUINO_UNOR4_WIFI) || defined(ARDUINO_UNOR4_MINIMA)
    for (auto startNow = millis() + 2500; !Serial && millis() < startNow; delay(500));
  #else
    delay(2000);
  #endif
  
  Serial.println("\n=== TLA2528 Digital Read/Write ===");
  Serial.print("Platform: ");
  Serial.println(BOARD_NAME);
  
  I2C_BUS.begin();
  I2C_BUS.setClock(400000);
  
  Serial.println("Connecting to TLA2528...");
  
  if (!expander.begin(TLA2528_ADDRESS)) {
    Serial.println("ERROR: TLA2528 not detected!");
    Serial.println("\nTroubleshooting:");
    Serial.println("- Check I2C connections and address");
    Serial.println("- Verify 4.7kΩ pull-up resistors on SDA/SCL");
    #if defined(ARDUINO_UNOR4_WIFI) || defined(ARDUINO_UNOR4_MINIMA)
      Serial.println("- Nano R4: Use Qwiic connector or A4/A5 with Wire");
    #endif
    while (1) delay(1000);
  }
  
  Serial.println("✓ Connected!");
  
  expander.pinMode(BUTTON_PIN, INPUT);
  expander.pinMode(LED_PIN, OUTPUT);
  expander.pinMode(PWM_LED_PIN, OUTPUT);
  expander.pinMode(POT_PIN, IO_ANALOG);
  
  expander.setPWMConfig(100, 256);
  
  expander.digitalWrite(LED_PIN, LOW);
  expander.digitalWrite(PWM_LED_PIN, LOW);
  
  Serial.println("✓ Setup complete!");
  Serial.println("\nOperation:");
  Serial.println("- Press button → Toggle LED");
  Serial.println("- Turn pot → Change PWM brightness");
}

void loop() {
  uint32_t now = millis();
  
  if (now - lastReadTime >= READ_INTERVAL) {
    lastReadTime = now;
    
    // Button toggle logic (detect press on falling edge)
    bool buttonState = expander.digitalRead(BUTTON_PIN);
    
    if (buttonState == LOW && lastButtonState == HIGH) {
      // Button just pressed - toggle LED
      ledState = !ledState;
      expander.digitalWrite(LED_PIN, ledState);
      
      Serial.print("Button pressed - LED ");
      Serial.println(ledState ? "ON" : "OFF");
    }
    
    lastButtonState = buttonState;
    
    // Potentiometer controls PWM brightness
    int sensor = expander.analogRead(POT_PIN);
    if (sensor < 0) sensor = 0;
    
    int targetBrightness = map(sensor, 0, 4095, 0, expander.getPWMMaxValue());
    currentBrightness += (targetBrightness - currentBrightness) / 4;
    
    expander.analogWrite(PWM_LED_PIN, currentBrightness);
  }
  
  expander.update();
}