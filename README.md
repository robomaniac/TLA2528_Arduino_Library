# TLA2528 Arduino Library

![Version](https://img.shields.io/badge/version-1.0.0-blue.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)
![Arduino](https://img.shields.io/badge/arduino-%3E%3D1.8.0-brightgreen.svg)

Arduino library for the Texas Instruments TLA2528 - an 8-channel, 12-bit ADC with GPIO and software PWM capabilities. Transform your I²C bus into a versatile I/O expansion system.

## 📑 Table of Content
<details>
<summary>📖 View Table of Contents</summary>

- [TLA2528 Arduino Library](#tla2528-arduino-library)
  - [📑 Table of Content](#-table-of-content)
  - [✨ Features](#-features)
  - [🚀 Quick Start](#-quick-start)
  - [📦 Installation](#-installation)
    - [Arduino Library Manager (Recommended)](#arduino-library-manager-recommended)
    - [Manual Installation](#manual-installation)
  - [🔌 Hardware Setup](#-hardware-setup)
    - [Basic Connections](#basic-connections)
    - [Hardware](#hardware)
  - [Platform-Specific Notes](#platform-specific-notes)
    - [ESP32](#esp32)
    - [Arduino Nano R4 (WiFi/Minima)](#arduino-nano-r4-wifiminima)
    - [Other Arduino Boards](#other-arduino-boards)
    - [I²C Address Configuration](#ic-address-configuration)
    - [⚠️ Important Notes](#️-important-notes)
  - [⚠️ IMPORTANT: PWM Best Practices](#️-important-pwm-best-practices)
  - [📖 API Reference](#-api-reference)
    - [Core Functions](#core-functions)
      - [`begin(address)`](#beginaddress)
      - [`pinMode(pin, mode)`](#pinmodepin-mode)
      - [`update()`](#update)
    - [Digital I/O](#digital-io)
      - [`digitalWrite(pin, value)`](#digitalwritepin-value)
      - [`digitalRead(pin)`](#digitalreadpin)
    - [Analog I/O](#analog-io)
      - [`analogRead(pin)`](#analogreadpin)
      - [`analogWrite(pin, value)`](#analogwritepin-value)
    - [PWM Configuration](#pwm-configuration)
      - [`setPWMConfig(frequency, resolution)`](#setpwmconfigfrequency-resolution)
  - [📊 Examples](#-examples)
  - [🎯 Use Cases](#-use-cases)
    - [Not Recommended For:](#not-recommended-for)
  - [🔧 Advanced Configuration](#-advanced-configuration)
    - [Custom PWM Settings](#custom-pwm-settings)
    - [Multiple Devices](#multiple-devices)
    - [Alternative I²C Bus (ESP32)](#alternative-ic-bus-esp32)
  - [🐥 Troubleshooting](#-troubleshooting)
    - [Device Not Found](#device-not-found)
    - [PWM Flickering](#pwm-flickering)
    - [Analog Read Issues](#analog-read-issues)
  - [📈 Performance](#-performance)
  - [🤝 Contributing](#-contributing)
  - [📄 License](#-license)
  - [🙏 Acknowledgments](#-acknowledgments)
  - [📮 Support](#-support)
</details>

## ✨ Features


- **Simple Arduino-style API** - Familiar `pinMode()`, `digitalWrite()`, `digitalRead()`, `analogWrite()`, `analogRead()` functions
- **12-bit ADC Resolution** - 4096 levels for precise analog measurements
- **Software PWM** - Smooth LED dimming and brightness control
- **Efficient Updates** - Batched I²C transactions for optimal performance

<img src="images/Block_Diagram.jpg" alt="Block Diagram" width="700">


## 🚀 Quick Start

```cpp
#include <Wire.h>
#include <TLA2528.h>

const uint8_t TLA2528_ADDRESS = 0x10;
const uint8_t BUTTON_PIN = 0;
const uint8_t LED_PIN = 1;
const uint8_t PWM_PIN = 2;
const uint8_t ANALOG_PIN = 3;

TLA2528 expander;

void setup() {
  Wire.begin();
  
  if (!expander.begin(TLA2528_ADDRESS)) {
    // Handle error
    while (1);
  }
  
  // Configure pins
  expander.pinMode(BUTTON_PIN, INPUT);
  expander.pinMode(LED_PIN, OUTPUT);
  expander.pinMode(PWM_PIN, OUTPUT);
  expander.pinMode(ANALOG_PIN, IO_ANALOG);
  
  // Configure PWM: 100Hz, 8-bit (0-255)
  expander.setPWMConfig(100, 256);
}

void loop() {
  // Digital read
  bool buttonState = expander.digitalRead(BUTTON_PIN);
  
  // Digital write
  expander.digitalWrite(LED_PIN, buttonState);
  
  // Analog read (12-bit: 0-4095)
  int analogValue = expander.analogRead(ANALOG_PIN);
  
  // Analog write (PWM)
  int brightness = map(analogValue, 0, 4095, 0, 255);
  expander.analogWrite(PWM_PIN, brightness);
  
  // Apply all changes
  expander.update();
}
```

📖 For a complete, working example** with platform auto-detection (ESP32, Nano R4, Arduino), smooth non-blocking PWM, and proper error handling, see [TLA2528_DigitalReadWrite.ino](examples/TLA2528_DigitalReadWrite/TLA2528_DigitalReadWrite.ino).

⚠️ **Important:** For smooth PWM operation, call `update()` continuously without blocking delays. See the example for the proper non-blocking pattern.



## 📦 Installation

### Arduino Library Manager (Recommended)
1. Open Arduino IDE
2. Go to **Sketch** → **Include Library** → **Manage Libraries...**
3. Search for "TLA2528"
4. Click **Install**

### Manual Installation
1. Download this repository as ZIP
2. In Arduino IDE: **Sketch** → **Include Library** → **Add .ZIP Library...**
3. Select the downloaded file


## 🔌 Hardware Setup

### Basic Connections

TODO - Add schematic

### Hardware

| Component | Image | Supplier Link |
|-----------|-------|---------------|
| TLA2528 | <img src="images/Chip.jpg" alt="TLA2528" width="100"> | https://www.digikey.com/en/products/detail/texas-instruments/TLA2528IRTER/12328606 |
| QFN-16 to DIP ADAPTER | <img src="images/QFN.jpg" alt="TLA2528" width="100"> | https://www.digikey.com/en/products/detail/schmalztech-llc/ST-QFN-16-3X3-05/24394924 |
| Decoupling Cap | <img src="images/Capacitor.jpg" alt="TLA2528" width="100"> | https://www.digikey.com/en/products/detail/samsung-electro-mechanics/CL10B105KP8NNNC/3887604 |

## Platform-Specific Notes

### ESP32
```cpp
TLA2528 expander;  // Uses default Wire
Wire.begin();      // Or Wire.begin(SDA_PIN, SCL_PIN) for custom pins
```

### Arduino Nano R4 (WiFi/Minima)
The Nano R4 has TWO I2C buses:
- **Wire** - A4/A5 pins (5V logic)
- **Wire1** - Qwiic connector (3.3V with level shifting) ⭐ Recommended

For 3.3V devices like TLA2528:
```cpp
TLA2528 expander(Wire1);  // Use Qwiic connector
Wire1.begin();
```

### Other Arduino Boards
```cpp
TLA2528 expander;  // Uses default Wire on A4 (SDA) / A5 (SCL)
Wire.begin();
```

### I²C Address Configuration

The TLA2528 supports two addresses based on the ADDR pin connection:

| ADDR Connection | I²C Address | Notes |
|----------------|-------------|--------|
| **Floating (default)** | **0x10** | Most common, used in examples |
| ADDR → DECAP | 0x17 | Requires 0Ω resistor short |

**To find your device address**, run an I²C scanner sketch before using the library.

Confirmed: 0x17 works.

 <img src="images/i2c_address_selector.jpg" alt="I2C selector" width="600">


### ⚠️ Important Notes

1. **No Internal Pull-ups**: The TLA2528 lacks internal pull-up/pull-down resistors. Add external 10kΩ resistors for button inputs.

2. **I²C Pull-ups Required**: Always use 4.7kΩ pull-up resistors on SDA and SCL lines.

3. **Software PWM Limitations**: 
   - Best at 100Hz, 8-bit resolution
   - Higher frequencies may flicker
   - Not suitable for motor control
   - Use hardware PWM for critical applications

## ⚠️ IMPORTANT: PWM Best Practices

For smooth PWM operation, **never use `delay()` in your main loop**. The `update()` function must be called continuously (ideally every 1-2ms) for flicker-free dimming.

❌ **Wrong** (causes flickering):
```cpp
void loop() {
  expander.update();
  delay(20);  // Blocks PWM updates!
}
```

✅ **Correct** (smooth operation):
```cpp
void loop() {
  // Read sensors periodically using millis()
  if (millis() - lastRead >= 50) {
    lastRead = millis();
    // ... read sensors ...
  }
  
  expander.update();  // Called continuously!
}
```

See examples for complete non-blocking patterns.


## 📖 API Reference

### Core Functions

#### `begin(address)`
Initialize communication with the TLA2528.
- **Parameters**: `address` - I²C address (default: 0x10)
- **Returns**: `true` if successful

#### `pinMode(pin, mode)`
Configure a pin's operating mode.
- **Parameters**: 
  - `pin` - GPIO pin (0-7)
  - `mode` - `INPUT`, `OUTPUT`, or `IO_ANALOG`

#### `update()`
Commit all pending changes to the device. Call this regularly in `loop()`.
- **Returns**: `true` if successful

### Digital I/O

#### `digitalWrite(pin, value)`
Set a digital output pin state.
- **Parameters**:
  - `pin` - GPIO pin (0-7)
  - `value` - `HIGH` or `LOW`

#### `digitalRead(pin)`
Read a digital input pin state.
- **Parameters**: `pin` - GPIO pin (0-7)
- **Returns**: `true` (HIGH) or `false` (LOW)

### Analog I/O

#### `analogRead(pin)`
Read a 12-bit analog value.
- **Parameters**: `pin` - GPIO pin configured as `IO_ANALOG`
- **Returns**: 0-4095, or -1 on error
- **Note**: This function blocks during conversion (~100μs)

#### `analogWrite(pin, value)`
Set PWM duty cycle (software-generated).
- **Parameters**:
  - `pin` - GPIO pin configured as `OUTPUT`
  - `value` - 0 to `getPWMMaxValue()`

### PWM Configuration

#### `setPWMConfig(frequency, resolution)`
Configure software PWM parameters.
- **Parameters**:
  - `frequency` - PWM frequency in Hz (default: 100)
  - `resolution` - Number of steps (default: 256)
- **Returns**: `true` if configuration is stable
- **Note**: frequency × resolution should be ≤ 25000 for stability

## 📊 Examples

The library includes comprehensive examples:

- **TLA2528_AnalogReadSerial.ino** - Basic analog input reading
- **TLA2528_DigitalReadWrite.ino** - Button input with LED control
- **TLA2528_Basic_Example.ino** - Don't let the name fool you, this is cool demo of all features

## 🎯 Use Cases

### Not Recommended For:
- High-speed PWM (>1kHz)
- Motor control
- Time-critical applications
- High-frequency signal generation

## 🔧 Advanced Configuration

### Custom PWM Settings

```cpp
// For smoother LED fading
expander.setPWMConfig(50, 512);  // 50Hz, 9-bit resolution

// For faster response
expander.setPWMConfig(200, 128);  // 200Hz, 7-bit resolution
```

### Multiple Devices

```cpp
TLA2528 expander1, expander2;

void setup() {
  Wire.begin();
  expander1.begin(0x10); 
  expander2.begin(0x11);  
}
```

### Alternative I²C Bus (ESP32)

```cpp
TLA2528 expander(Wire1);  // Use Wire1 instead of Wire

void setup() {
  Wire1.begin(21, 22);  // Custom SDA, SCL pins
  expander.begin();
}
```

## 🐥 Troubleshooting

### Device Not Found
- Check I²C connections and pull-up resistors
- Verify power supply (2.7-5.5V)
- Scan I²C bus for correct address
- Ensure `Wire.begin()` is called before `expander.begin()`

### PWM Flickering
- Reduce frequency or resolution
- Call `update()` more frequently
- Check I²C bus speed
- Consider hardware PWM alternatives

### Analog Read Issues
- Verify pin is configured as `IO_ANALOG`

## 📈 Performance

TODO - Add logic analyzer pictures

## 🤝 Contributing

Contributions are welcome! Please feel free to submit pull requests, report bugs, or suggest features.

1. Fork the repository
2. Create your feature branch
3. Commit your changes
4. Push to the branch
5. Open a Pull Request

## 📄 License

This library is released under the MIT License. See [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

Thanks to Gemini and Claude for contributing to this README. 

## 📮 Support

- 📧 Report issues on [GitHub Issues](https://github.com/robomaniac/TLA2528-Arduino/issues)
- 💬 Join discussions on [GitHub Discussions](https://github.com/robomaniac/TLA2528-Arduino/discussions)
- 📖 Read the [datasheet](https://github.com/robomaniac/TLA2528_Arduino_Library/tree/main/documents)

---

**Made with ❤️ for the Arduino community**

