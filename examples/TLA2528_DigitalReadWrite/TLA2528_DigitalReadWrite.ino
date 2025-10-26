#include <Wire.h>
#include <TLA2528.h>

TLA2528 expander;

void setup() {
  Wire.begin();
  expander.begin();
  
  expander.pinMode(0, OUTPUT);      // LED on GPIO0
  expander.pinMode(1, INPUT);       // Button on GPIO1
  expander.pinMode(2, IO_ANALOG);   // Sensor on GPIO2
}

void loop() {
  // Digital I/O
  bool button = expander.digitalRead(1);
  expander.digitalWrite(0, button);
  
  // Analog input (12-bit: 0-4095)
  int sensor = expander.analogRead(2);
  
  // PWM output (software-generated)
  int brightness = map(sensor, 0, 4095, 0, 255);
  expander.analogWrite(0, brightness);
  
  expander.update();  // Commit changes
}