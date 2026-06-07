# Hardware Setup & Schematic

Below is the detailed wiring schematic mapping for the STM32F401RE Nucleo board and the external sensors.

## Pinout Map

| Module | Pin | STM32 Pin | Type | Notes |
|--------|-----|-----------|------|-------|
| **HC-SR04** | VCC | 5V | Power | Requires 5V logic |
| **HC-SR04** | GND | GND | Power | Common Ground |
| **HC-SR04** | TRIG | `PB3` | Output | 10µs trigger pulse |
| **HC-SR04** | ECHO | `PA10` | Input | Returns width-modulated pulse |
| **DHT11** | VCC | 3.3V | Power | Works at 3.3V |
| **DHT11** | DATA | `PA0` | In/Out | 1-wire protocol, requires pull-up |
| **LDR** | D0 | `PB10` | Input | Digital out from LDR module |
| **LED** | IN | `PB4` | Output | Active-Low logic |
| **Buzzer** | IN | `PB5` | Output | Active-Low logic |

## Power Considerations
Ensure that your Nucleo board is powered via the ST-Link USB connector or an external power supply. The HC-SR04 sensor must be supplied with 5V to function accurately, while the DHT11 and LDR modules can run on the 3.3V rail. All sensor grounds must be tied to the STM32 GND pins.
