# TLA2528 Arduino Library

![Version](https://img.shields.io/badge/version-1.0.0-blue.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)
![Arduino](https://img.shields.io/badge/arduino-%3E%3D1.8.0-brightgreen.svg)

Arduino library for the Texas Instruments TLA2528 - an 8-channel, 12-bit ADC with GPIO and software PWM capabilities. Transform your I²C bus into a versatile I/O expansion system.

## ✨ Features

- **🎯 Simple Arduino-style API** - Familiar `pinMode()`, `digitalWrite()`, `digitalRead()`, `analogWrite()`, `analogRead()` functions
- **📊 12-bit ADC Resolution** - 4096 levels for precise analog measurements
- **💡 Software PWM** - Smooth LED dimming and brightness control
- **🔄 Efficient Updates** - Batched I²C transactions for optimal performance


## 🚀 Quick Start

```cpp
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
```

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

todo - schematic

### I²C Address Configuration

The TLA2528 address according to datasheet is 0x17 but when I use the I2C scan it return 0x10
- Address `0x10` (default)


### ⚠️ Important Notes

1. **No Internal Pull-ups**: The TLA2528 lacks internal pull-up/pull-down resistors. Add external 10kΩ resistors for button inputs.

2. **I²C Pull-ups Required**: Always use 4.7kΩ pull-up resistors on SDA and SCL lines.

3. **Software PWM Limitations**: 
   - Best at 100Hz, 8-bit resolution
   - Higher frequencies may flicker
   - Not suitable for motor control
   - Use hardware PWM for critical applications

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

todo - will add logic analyzer pictures

| Operation | Typical Time | I²C Transactions |
|-----------|-------------|------------------|
| Digital Read | ~150μs | 1 |
| Digital Write | ~5μs* | 0* |
| Analog Read | ~200μs | 2 |
| PWM Update | ~150μs | 0-1 |
| Full Update | ~150μs | 1 |

*Cached until `update()` is called

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

- Gemini and Claude for vibe coding this as you can tell from this amazing readme

## 📮 Support

- 📧 Report issues on [GitHub Issues](https://github.com/robomaniac/TLA2528-Arduino/issues)
- 💬 Join discussions on [GitHub Discussions](https://github.com/robomaniac/TLA2528-Arduino/discussions)
- 📖 Read the [datasheet](https://github.com/robomaniac/TLA2528_Arduino_Library/tree/main/documents)

---

**Made with ❤️ for the Arduino community**

