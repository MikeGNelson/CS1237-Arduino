# CS1237 Arduino Library

Arduino library for interfacing with the **CS1237 24-bit ADC**. Developed for ESP32-based load-cell applications.

The library provides 24-bit ADC data acquisition and access to the CS1237 configuration register, including PGA gain, sample rate, input channel, reference, and power controls.

## Features

- 24-bit signed ADC readings
- Configurable PGA gain
  - 1×
  - 2×
  - 64×
  - 128×
- Configurable sample rate
  - 10 Hz
  - 40 Hz
  - 640 Hz
  - 1280 Hz
- Input channel selection
- Internal temperature sensor selection
- Reference output control
- Power-down and power-up control
- Configuration register read/write

## Usage

```cpp
#include <CS1237.h>

CS1237 adc(18, 19); // SCLK, DOUT

void setup() {
    Serial.begin(115200);
    adc.begin();
}

void loop() {
    long value = adc.readData();
    Serial.println(value);
}
```

Change the SCLK and DOUT pins to match your hardware configuration.

## API

```cpp
CS1237(uint8_t sclkPin, uint8_t doutPin);

void begin();

long readData();

void powerDown();
void powerUp();

int getRegister();
void setRegister(int registerToWrite, int valueToWrite);

void setGain(uint8_t gain);
void setDataRate(uint8_t rate);
```

## Configuration Register

`setRegister()` accepts a register field and value:

| Field | ID | Values |
| --- | ---: | --- |
| Channel | 0 | 0 = Channel A, 2 = Temperature, 3 = Internal Short |
| PGA | 1 | 0 = 1×, 1 = 2×, 2 = 64×, 3 = 128× |
| Data Rate | 2 | 0 = 10 Hz, 1 = 40 Hz, 2 = 640 Hz, 3 = 1280 Hz |
| Reference | 3 | 0 = On, 1 = Off |

Example:

```cpp
adc.setRegister(1, 3); // PGA = 128
adc.setRegister(2, 1); // 40 Hz
```

## Hardware

The CS1237 is a 24-bit sigma-delta ADC intended for high-precision differential measurements such as load cells and other bridge sensors.

Communication uses two signals:

- `SCLK` — serial clock
- `DRDY/DOUT` — data-ready and bidirectional data line

## ESP32 Timing

This implementation was developed for the **ESP32**.

The CS1237 requires precise sub-microsecond serial timing. The current implementation uses an ESP32-specific NOP delay for the required timing intervals. Other Arduino-compatible architectures may require modification of the timing implementation.

## Files

- `CS1237.h` — library interface
- `CS1237.cpp` — CS1237 communication and configuration implementation
- `examples/BasicRead/BasicRead.ino` — basic ADC reading example

## License

MIT License
