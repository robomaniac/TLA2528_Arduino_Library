#include <Wire.h>
#include <TLA2528.h>

TLA2528 expander;

const uint8_t BUTTON_PIN = 0;
const uint8_t LED_PIN = 1;
const uint8_t PWM_LED_PIN = 2;
const uint8_t POT_PIN = 3;

void setup() {
  Serial.begin(115200);
  delay(500);

  Wire.begin();

  if (!expander.begin(0x10)) {
    Serial.println("TLA2528 not detected!");
    while (1);
  }

  // Configure pins
  expander.pinMode(BUTTON_PIN, INPUT);
  expander.pinMode(LED_PIN, OUTPUT);
  expander.pinMode(PWM_LED_PIN, OUTPUT);
  expander.pinMode(POT_PIN, IO_ANALOG);

  // Configure PWM
  expander.setPWMConfig(100, 256); // 100 Hz, 8-bit resolution (0–255)

  // Start with LEDs off
  expander.digitalWrite(LED_PIN, LOW);
  expander.digitalWrite(PWM_LED_PIN, LOW);

  Serial.println("Setup complete.");
}

void loop() {
  // Button (active-low)
  bool button = expander.digitalRead(BUTTON_PIN);
  expander.digitalWrite(LED_PIN, button ? LOW : HIGH);

  // Potentiometer (0–4095)
  int sensor = expander.analogRead(POT_PIN);
  if (sensor < 0) sensor = 0;

  // Map 12-bit ADC to 8-bit PWM
  int brightness = map(sensor, 0, 4095, 0, expander.getPWMMaxValue());

  expander.analogWrite(PWM_LED_PIN, brightness);

  // Apply all pending writes
  expander.update();

  delay(20);
}
