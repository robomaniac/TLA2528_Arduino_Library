/**
 * TLA2528_Basic_Example.ino
 * 
 * Comprehensive demonstration of TLA2528 library features:
 * - Digital input/output with button and LED
 * - 12-bit analog reading from potentiometer
 * - Software PWM for LED brightness control
 * - Activity indicator with blink
 * 
 * Hardware Setup:
 * - GPIO0: Button with 10kΩ pull-up resistor (INPUT)
 * - GPIO1: Status LED with 220Ω resistor (OUTPUT)
 * - GPIO2: PWM LED with 220Ω resistor (PWM OUTPUT)
 * - GPIO3: Potentiometer (ANALOG INPUT)
 * - GPIO4: Blink LED with 220Ω resistor (OUTPUT)
 * 
 * Note: TLA2528 has NO internal pull-up resistors!
 */

#include <Wire.h>
#include <TLA2528.h>

// I2C Address
const uint8_t TLA2528_ADDRESS = 0x10;

// Pin Definitions
const uint8_t BUTTON_PIN = 0;   // Button input (needs external pull-up)
const uint8_t LED_PIN = 1;       // Status LED output
const uint8_t PWM_LED_PIN = 2;   // PWM controlled LED
const uint8_t POT_PIN = 3;       // Potentiometer analog input
const uint8_t BLINK_PIN = 4;     // Activity blink LED

// Timing Constants
const uint32_t DEBOUNCE_DELAY = 20;     // Button debounce time
const uint32_t BLINK_INTERVAL = 200;    // Blink rate

// Auto-detect platform and use correct I2C bus
#if defined(ARDUINO_UNOR4_WIFI) || defined(ARDUINO_UNOR4_MINIMA)
  TLA2528 expander(Wire1);
  #define I2C_BUS Wire1
  #define BOARD_NAME "Arduino Nano R4"
#else
  TLA2528 expander;
  #define I2C_BUS Wire
  #define BOARD_NAME "Default (Wire)"
#endif

// State Variables
struct {
  uint32_t lastDebounceTime = 0;
  bool lastButtonReading = HIGH;
  bool buttonState = HIGH;
  bool ledState = false;

  uint32_t lastBlinkTime = 0;
  bool blinkState = false;

  int lastPotValue = -1;
  int currentBrightness = 0;
} state;

void setup() {
  Serial.begin(115200);
  
  #if defined(ARDUINO_UNOR4_WIFI) || defined(ARDUINO_UNOR4_MINIMA)
    for (auto startNow = millis() + 2500; !Serial && millis() < startNow; delay(500));
  #else
    delay(2000);
  #endif

  printHeader();

  Serial.println("Initializing I2C...");
  I2C_BUS.begin();
  I2C_BUS.setClock(400000); // Standard 400kHz you can try 100khz if you have issue or simply unplug and plug qwiic connector or power cycle

  Serial.println("Initializing TLA2528...");
  if (!expander.begin(TLA2528_ADDRESS)) {
    Serial.println("ERROR: Failed to initialize TLA2528!");
    Serial.println("Check I2C connections and pull-up resistors.");
    while (1) delay(100);
  }

  Serial.print("SUCCESS: Device ID = 0x");
  Serial.println(expander.getDeviceID(), HEX);

  configurePins();
  configurePWM();

  Serial.println("\n--- Setup Complete ---");
  Serial.println("Button  -> Status LED");
  Serial.println("Pot     -> PWM LED brightness");
  Serial.println("GPIO4   -> Blink indicator");
  Serial.println();
}

void configurePins() {
  Serial.println("Configuring pins...");

  expander.pinMode(BUTTON_PIN, INPUT);
  expander.pinMode(LED_PIN, OUTPUT);
  expander.pinMode(PWM_LED_PIN, OUTPUT);
  expander.pinMode(POT_PIN, IO_ANALOG);
  expander.pinMode(BLINK_PIN, OUTPUT);

  expander.digitalWrite(LED_PIN, LOW);
  expander.digitalWrite(PWM_LED_PIN, LOW);
  expander.digitalWrite(BLINK_PIN, LOW);

  Serial.println("Pins configured");
}

void configurePWM() {
  Serial.println("Configuring PWM...");
  
  if (expander.setPWMConfig(100, 256)) {
    Serial.print("PWM: 100Hz, 8-bit (0-");
    Serial.print(expander.getPWMMaxValue());
    Serial.println(")");
  } else {
    Serial.println("PWM: Config may exceed I2C limits");
  }
}

void loop() {
  uint32_t now = millis();

  handleButton(now);
  updateBlink(now);
  handleAnalogPWM();

  if (!expander.update()) {
    static uint32_t lastError = 0;
    if (now - lastError > 1000) {
      Serial.println("Warning: I2C update failed");
      lastError = now;
    }
  }
}

void handleButton(uint32_t now) {
  bool reading = expander.digitalRead(BUTTON_PIN);

  if (reading != state.lastButtonReading) state.lastDebounceTime = now;

  if ((now - state.lastDebounceTime) > DEBOUNCE_DELAY) {
    if (reading != state.buttonState) {
      state.buttonState = reading;
      if (state.buttonState == LOW) {
        state.ledState = !state.ledState;
        expander.digitalWrite(LED_PIN, state.ledState);

        Serial.print("Button pressed - LED ");
        Serial.println(state.ledState ? "ON" : "OFF");
      }
    }
  }

  state.lastButtonReading = reading;
}

void updateBlink(uint32_t now) {
  if (now - state.lastBlinkTime >= BLINK_INTERVAL) {
    state.lastBlinkTime = now;
    state.blinkState = !state.blinkState;
    expander.digitalWrite(BLINK_PIN, state.blinkState);
  }
}

void handleAnalogPWM() {
  int potValue = expander.analogRead(POT_PIN);

  if (potValue < 0) {
    expander.analogWrite(PWM_LED_PIN, 0);
    return;
  }

  int targetBrightness = map(potValue, 0, 4095, 0, expander.getPWMMaxValue());
  state.currentBrightness += (targetBrightness - state.currentBrightness) / 4;

  expander.analogWrite(PWM_LED_PIN, state.currentBrightness);

  if (abs(potValue - state.lastPotValue) > 50) {
    Serial.print("Analog: ");
    Serial.print(potValue);
    Serial.print(" -> PWM: ");
    Serial.print(state.currentBrightness);
    Serial.print("/");
    Serial.println(expander.getPWMMaxValue());
    state.lastPotValue = potValue;
  }
}

void printHeader() {
  Serial.println();
  Serial.println("======================================");
  Serial.println("    TLA2528 Basic Example v2.0");
  Serial.println("    8-Channel ADC/GPIO Expander");
  Serial.println("======================================");
  Serial.println();
}
