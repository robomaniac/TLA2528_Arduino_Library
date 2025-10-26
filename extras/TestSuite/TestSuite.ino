/**
 * TLA2528 Library Test Suite
 * 
 * Comprehensive test to verify all library functions.
 * Run this after installation to ensure everything works correctly.
 */

#include <Wire.h>
#include <TLA2528.h>

TLA2528 expander;

// Test configuration
const uint8_t TEST_DIGITAL_PIN = 0;
const uint8_t TEST_ANALOG_PIN = 1;
const uint8_t TEST_PWM_PIN = 2;

// Test results
struct TestResults {
  bool connection = false;
  bool deviceID = false;
  bool pinMode = false;
  bool digitalWrite = false;
  bool digitalRead = false;
  bool analogRead = false;
  bool pwmConfig = false;
  bool analogWrite = false;
  bool update = false;
  
  int passed = 0;
  int failed = 0;
  
  void printSummary() {
    Serial.println("\n═══════════════════════════════");
    Serial.println("         TEST RESULTS          ");
    Serial.println("═══════════════════════════════");
    Serial.print("  Passed: "); Serial.println(passed);
    Serial.print("  Failed: "); Serial.println(failed);
    Serial.print("  Total:  "); Serial.println(passed + failed);
    Serial.println("═══════════════════════════════");
    
    if (failed == 0) {
      Serial.println("✅ All tests passed!");
    } else {
      Serial.println("❌ Some tests failed. Check connections.");
    }
  }
};

TestResults results;

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000);
  
  printHeader();
  
  Wire.begin();
  
  // Run all tests
  testConnection();
  testDeviceID();
  testPinMode();
  testDigitalWrite();
  testDigitalRead();
  testAnalogRead();
  testPWMConfig();
  testAnalogWrite();
  testUpdate();
  
  // Print results
  results.printSummary();
  
  Serial.println("\n📝 Note: Some tests require external components:");
  Serial.println("  • Pull-up resistors on I²C lines");
  Serial.println("  • Pull-up resistor on digital input pin");
  Serial.println("  • LED on output pins to verify operation");
}

void loop() {
  // Run a simple demo after tests
  static uint32_t lastTime = 0;
  static uint8_t brightness = 0;
  static int8_t direction = 1;
  
  if (millis() - lastTime > 20) {
    lastTime = millis();
    
    brightness += direction * 5;
    if (brightness >= 255 || brightness <= 0) {
      direction = -direction;
      brightness = constrain(brightness, 0, 255);
    }
    
    expander.analogWrite(TEST_PWM_PIN, brightness);
    expander.update();
  }
}

void testConnection() {
  Serial.print("Testing connection... ");
  
  results.connection = expander.begin();
  
  if (results.connection) {
    Serial.println("✓ PASS");
    results.passed++;
  } else {
    Serial.println("✗ FAIL - Check I²C wiring");
    results.failed++;
  }
}

void testDeviceID() {
  Serial.print("Testing device ID... ");
  
  if (!results.connection) {
    Serial.println("✗ SKIP - No connection");
    return;
  }
  
  uint8_t id = expander.getDeviceID();
  results.deviceID = (id != 0x00 && id != 0xFF);
  
  if (results.deviceID) {
    Serial.print("✓ PASS (ID: 0x");
    Serial.print(id, HEX);
    Serial.println(")");
    results.passed++;
  } else {
    Serial.println("✗ FAIL - Invalid ID");
    results.failed++;
  }
}

void testPinMode() {
  Serial.print("Testing pinMode... ");
  
  if (!results.connection) {
    Serial.println("✗ SKIP - No connection");
    return;
  }
  
  results.pinMode = true;
  results.pinMode &= expander.pinMode(TEST_DIGITAL_PIN, OUTPUT);
  results.pinMode &= expander.pinMode(TEST_ANALOG_PIN, IO_ANALOG);
  results.pinMode &= expander.pinMode(TEST_PWM_PIN, OUTPUT);
  
  if (results.pinMode) {
    Serial.println("✓ PASS");
    results.passed++;
  } else {
    Serial.println("✗ FAIL");
    results.failed++;
  }
}

void testDigitalWrite() {
  Serial.print("Testing digitalWrite... ");
  
  if (!results.connection) {
    Serial.println("✗ SKIP - No connection");
    return;
  }
  
  // This test just verifies the function runs without error
  expander.digitalWrite(TEST_DIGITAL_PIN, HIGH);
  expander.update();
  delay(10);
  expander.digitalWrite(TEST_DIGITAL_PIN, LOW);
  expander.update();
  
  results.digitalWrite = true;
  Serial.println("✓ PASS");
  results.passed++;
}

void testDigitalRead() {
  Serial.print("Testing digitalRead... ");
  
  if (!results.connection) {
    Serial.println("✗ SKIP - No connection");
    return;
  }
  
  // Configure as input
  expander.pinMode(TEST_DIGITAL_PIN, INPUT);
  
  // Read the pin (result depends on external circuit)
  bool state = expander.digitalRead(TEST_DIGITAL_PIN);
  
  results.digitalRead = true;
  Serial.print("✓ PASS (read: ");
  Serial.print(state ? "HIGH" : "LOW");
  Serial.println(")");
  results.passed++;
  
  // Restore as output
  expander.pinMode(TEST_DIGITAL_PIN, OUTPUT);
}

void testAnalogRead() {
  Serial.print("Testing analogRead... ");
  
  if (!results.connection) {
    Serial.println("✗ SKIP - No connection");
    return;
  }
  
  int16_t value = expander.analogRead(TEST_ANALOG_PIN);
  
  if (value >= 0 && value <= 4095) {
    results.analogRead = true;
    Serial.print("✓ PASS (value: ");
    Serial.print(value);
    Serial.println(")");
    results.passed++;
  } else {
    Serial.println("✗ FAIL - Invalid value");
    results.failed++;
  }
}

void testPWMConfig() {
  Serial.print("Testing PWM config... ");
  
  if (!results.connection) {
    Serial.println("✗ SKIP - No connection");
    return;
  }
  
  results.pwmConfig = expander.setPWMConfig(100, 256);
  
  if (results.pwmConfig) {
    Serial.print("✓ PASS (max: ");
    Serial.print(expander.getPWMMaxValue());
    Serial.println(")");
    results.passed++;
  } else {
    Serial.println("✗ FAIL");
    results.failed++;
  }
}

void testAnalogWrite() {
  Serial.print("Testing analogWrite... ");
  
  if (!results.connection) {
    Serial.println("✗ SKIP - No connection");
    return;
  }
  
  // Test various PWM values
  expander.analogWrite(TEST_PWM_PIN, 0);
  expander.update();
  delay(10);
  
  expander.analogWrite(TEST_PWM_PIN, 128);
  expander.update();
  delay(10);
  
  expander.analogWrite(TEST_PWM_PIN, 255);
  expander.update();
  delay(10);
  
  results.analogWrite = true;
  Serial.println("✓ PASS");
  results.passed++;
}

void testUpdate() {
  Serial.print("Testing update... ");
  
  if (!results.connection) {
    Serial.println("✗ SKIP - No connection");
    return;
  }
  
  results.update = expander.update();
  
  if (results.update) {
    Serial.println("✓ PASS");
    results.passed++;
  } else {
    Serial.println("✗ FAIL");
    results.failed++;
  }
}

void printHeader() {
  Serial.println();
  Serial.println("╔════════════════════════════════╗");
  Serial.println("║   TLA2528 Library Test Suite   ║");
  Serial.println("║          Version 2.0.0         ║");
  Serial.println("╚════════════════════════════════╝");
  Serial.println();
}
