/**
 * TLA2528 Arduino Library - Implementation
 * 
 * @author  Jerome Demers
 * @version 1.0.0
 * @license MIT
 */

#include "TLA2528.h"
#include <math.h>

// Gamma correction for LED brightness
static constexpr float LED_GAMMA = 2.2;

TLA2528::TLA2528(TwoWire& wire) : _wire(&wire) {}

bool TLA2528::begin(uint8_t address) {
  _address = address;
  
  // Try fast I2C first (1MHz), fall back to 400kHz if needed
  _wire->setClock(1000000);
  _wire->beginTransmission(_address);
  
  if (_wire->endTransmission() != 0) {
    _wire->setClock(400000);
    _wire->beginTransmission(_address);
    if (_wire->endTransmission() != 0) {
      return false;
    }
  }
  
  _connected = true;
  
  // Initialize GPIO outputs to off
  if (!writeRegister(TLA2528_REG::GPO_VALUE, 0x00)) {
    _connected = false;
    return false;
  }
  
  // Configure for manual ADC channel selection
  if (!writeRegister(TLA2528_REG::SEQUENCE_CFG, 0x00)) {
    _connected = false;
    return false;
  }
  
  // Read device ID for verification
  if (!readRegister(TLA2528_REG::DEVICE_ID, _deviceID)) {
    _connected = false;
    return false;
  }
  
  // Initialize PWM with defaults
  setPWMConfig(100, 256);
  
  return true;
}

bool TLA2528::pinMode(uint8_t pin, uint8_t mode) {
  if (!_connected || pin >= NUM_CHANNELS) return false;
  
  uint8_t mask = 1 << pin;
  uint8_t pinCfg, gpioCfg;
  
  // Read current pin configuration
  if (!readRegister(TLA2528_REG::PIN_CFG, pinCfg)) return false;
  
  if (mode == IO_ANALOG) {
    // Analog input mode
    pinCfg &= ~mask;  // 0 = Analog
    return writeRegister(TLA2528_REG::PIN_CFG, pinCfg);
  }
  
  // Digital mode
  pinCfg |= mask;  // 1 = GPIO
  if (!writeRegister(TLA2528_REG::PIN_CFG, pinCfg)) return false;
  
  // Configure GPIO direction
  if (!readRegister(TLA2528_REG::GPIO_CFG, gpioCfg)) return false;
  
  if (mode == OUTPUT) {
    gpioCfg |= mask;  // 1 = Output
    if (!writeRegister(TLA2528_REG::GPIO_CFG, gpioCfg)) return false;
    
    // Configure push-pull drive
    uint8_t driveCfg;
    if (!readRegister(TLA2528_REG::GPO_DRIVE_CFG, driveCfg)) return false;
    driveCfg |= mask;  // 1 = Push-Pull
    return writeRegister(TLA2528_REG::GPO_DRIVE_CFG, driveCfg);
  } else {
    gpioCfg &= ~mask;  // 0 = Input
    return writeRegister(TLA2528_REG::GPIO_CFG, gpioCfg);
  }
}

bool TLA2528::digitalRead(uint8_t pin) {
  if (!_connected || pin >= NUM_CHANNELS) return false;
  
  uint8_t gpiValue;
  if (!readRegister(TLA2528_REG::GPI_VALUE, gpiValue)) return false;
  
  return gpiValue & (1 << pin);
}

void TLA2528::digitalWrite(uint8_t pin, bool value) {
  if (pin >= NUM_CHANNELS) return;
  
  _pwm[pin].isPWM = false;
  
  if (value) {
    _gpoCache |= (1 << pin);
  } else {
    _gpoCache &= ~(1 << pin);
  }
}

int16_t TLA2528::analogRead(uint8_t pin) {
  if (!_connected || pin >= NUM_CHANNELS) return -1;
  
  // Select channel
  if (!writeRegister(TLA2528_REG::CHANNEL_SEL, pin)) return -1;
  
  // Request conversion result (2 bytes)
  if (_wire->requestFrom(_address, uint8_t(2)) != 2) return -1;
  
  // Read 12-bit result
  uint16_t result = (_wire->read() << 4) | (_wire->read() >> 4);
  return result;
}

void TLA2528::analogWrite(uint8_t pin, uint16_t value) {
  if (pin >= NUM_CHANNELS) return;
  
  _pwm[pin].isPWM = true;
  _pwm[pin].targetValue = min(value, _pwmMaxValue);
  calculateOnTime(pin);
}

bool TLA2528::setPWMConfig(const PWMConfig& config) {
  return setPWMConfig(config.frequency, config.resolution);
}

bool TLA2528::setPWMConfig(uint16_t frequency, uint16_t resolution) {
  frequency = max(uint16_t(1), frequency);
  resolution = max(uint16_t(2), resolution);
  
  // Check if configuration is feasible with I2C speed
  uint32_t stepsPerSecond = uint32_t(frequency) * resolution;
  uint32_t usPerStep = 1000000 / stepsPerSecond;
  
  if (usPerStep < 40) {  // ~40us minimum per I2C transaction at 1MHz
    // Configuration exceeds I2C bandwidth
    return false;
  }
  
  _cycleTimeUs = 1000000 / frequency;
  _pwmMaxValue = resolution - 1;
  
  // Recalculate all PWM on-times
  for (uint8_t i = 0; i < NUM_CHANNELS; i++) {
    if (_pwm[i].isPWM) {
      calculateOnTime(i);
    }
  }
  
  return true;
}

bool TLA2528::update() {
  if (!_connected) return false;
  
  updatePWMStates();
  return commitGPO();
}

void TLA2528::calculateOnTime(uint8_t pin) {
  if (pin >= NUM_CHANNELS) return;
  
  // Apply gamma correction for better LED brightness perception
  float normalized = float(_pwm[pin].targetValue) / _pwmMaxValue;
  float corrected = pow(normalized, LED_GAMMA);
  _pwm[pin].onTimeUs = uint32_t(corrected * _cycleTimeUs);
}

void TLA2528::updatePWMStates() {
  uint32_t now = micros();
  uint32_t elapsed = now - _lastCycleTimeUs;
  
  // Check for new cycle
  if (elapsed >= _cycleTimeUs) {
    _lastCycleTimeUs = now;
    elapsed = 0;
    
    // Start of new cycle - turn on PWM pins that need it
    for (uint8_t i = 0; i < NUM_CHANNELS; i++) {
      if (!_pwm[i].isPWM) continue;
      
      if (_pwm[i].onTimeUs > 0) {
        _gpoCache |= (1 << i);
        _pwm[i].currentState = true;
      } else {
        _gpoCache &= ~(1 << i);
        _pwm[i].currentState = false;
      }
    }
  }
  
  // Check if any PWM pins need to turn off
  for (uint8_t i = 0; i < NUM_CHANNELS; i++) {
    if (!_pwm[i].isPWM || !_pwm[i].currentState) continue;
    
    if (elapsed >= _pwm[i].onTimeUs && _pwm[i].onTimeUs < _cycleTimeUs) {
      _gpoCache &= ~(1 << i);
      _pwm[i].currentState = false;
    }
  }
}

bool TLA2528::commitGPO() {
  if (_gpoCache == _gpoLastWritten) return true;
  
  if (writeRegister(TLA2528_REG::GPO_VALUE, _gpoCache)) {
    _gpoLastWritten = _gpoCache;
    return true;
  }
  return false;
}

bool TLA2528::readRegister(uint8_t reg, uint8_t& value) {
  _wire->beginTransmission(_address);
  _wire->write(TLA2528_REG::OP_READ);
  _wire->write(reg);
  
  if (_wire->endTransmission(false) != 0) return false;
  
  if (_wire->requestFrom(_address, uint8_t(1)) != 1) return false;
  
  // Wait for data with timeout
  uint32_t start = micros();
  while (!_wire->available()) {
    if (micros() - start > 500) return false;
  }
  
  value = _wire->read();
  return true;
}

bool TLA2528::writeRegister(uint8_t reg, uint8_t value) {
  _wire->beginTransmission(_address);
  _wire->write(TLA2528_REG::OP_WRITE);
  _wire->write(reg);
  _wire->write(value);
  return _wire->endTransmission() == 0;
}
