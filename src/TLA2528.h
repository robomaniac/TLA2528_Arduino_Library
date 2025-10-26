/**
 * TLA2528 Arduino Library
 * 
 * A clean, efficient library for the Texas Instruments TLA2528
 * 8-channel, 12-bit ADC with GPIO and software PWM capabilities.
 * 
 * @author  Jerome Demers
 * @version 1.0.0
 * @license MIT
 */

#ifndef TLA2528_H
#define TLA2528_H

#include <Arduino.h>
#include <Wire.h>

// Pin modes
#ifndef IO_ANALOG
  #define IO_ANALOG 0x04
#endif

// TLA2528 Register Map
namespace TLA2528_REG {
  constexpr uint8_t DEVICE_ID     = 0x01;
  constexpr uint8_t PIN_CFG       = 0x05;
  constexpr uint8_t GPIO_CFG      = 0x07;
  constexpr uint8_t GPO_DRIVE_CFG = 0x09;
  constexpr uint8_t GPO_VALUE     = 0x0B;
  constexpr uint8_t GPI_VALUE     = 0x0D;
  constexpr uint8_t SEQUENCE_CFG  = 0x10;
  constexpr uint8_t CHANNEL_SEL   = 0x11;
  
  // Opcodes
  constexpr uint8_t OP_READ       = 0x10;
  constexpr uint8_t OP_WRITE      = 0x08;
}

class TLA2528 {
public:
  static constexpr uint8_t NUM_CHANNELS = 8;
  static constexpr uint16_t ADC_MAX_VALUE = 4095;  // 12-bit
  static constexpr uint8_t DEFAULT_ADDRESS = 0x10;
  
  // Constructor
  explicit TLA2528(TwoWire& wire = Wire);
  
  // Initialization
  bool begin(uint8_t address = DEFAULT_ADDRESS);
  bool isConnected() const { return _connected; }
  uint8_t getDeviceID() const { return _deviceID; }
  
  // Pin Configuration
  bool pinMode(uint8_t pin, uint8_t mode);
  
  // Digital I/O
  bool digitalRead(uint8_t pin);
  void digitalWrite(uint8_t pin, bool value);
  
  // Analog I/O
  int16_t analogRead(uint8_t pin);
  void analogWrite(uint8_t pin, uint16_t value);
  
  // PWM Configuration
  struct PWMConfig {
    uint16_t frequency = 100;     // Hz
    uint16_t resolution = 256;    // Steps (8-bit default)
  };
  
  bool setPWMConfig(const PWMConfig& config);
  bool setPWMConfig(uint16_t frequency, uint16_t resolution);
  uint16_t getPWMMaxValue() const { return _pwmMaxValue; }
  
  // Update - Must be called regularly in loop()
  bool update();

private:
  // Hardware interface
  TwoWire* _wire;
  uint8_t _address;
  bool _connected = false;
  uint8_t _deviceID = 0;
  
  // GPIO state cache
  uint8_t _gpoCache = 0x00;
  uint8_t _gpoLastWritten = 0xFF;  // Force first write
  
  // PWM state for each pin
  struct PWMState {
    uint16_t targetValue = 0;
    uint32_t onTimeUs = 0;
    bool currentState = false;
    bool isPWM = false;
  };
  PWMState _pwm[NUM_CHANNELS];
  
  // PWM timing
  uint32_t _lastCycleTimeUs = 0;
  uint32_t _cycleTimeUs = 10000;   // 100Hz default
  uint16_t _pwmMaxValue = 255;     // 8-bit default
  
  // I2C communication
  bool readRegister(uint8_t reg, uint8_t& value);
  bool writeRegister(uint8_t reg, uint8_t value);
  bool commitGPO();
  
  // PWM calculations
  void calculateOnTime(uint8_t pin);
  void updatePWMStates();
};

// Maintain backward compatibility with old class name
using TLA2528_IO = TLA2528;

#endif // TLA2528_H
