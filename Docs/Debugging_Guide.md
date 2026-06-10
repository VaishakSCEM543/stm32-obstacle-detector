# Hardware Debugging Guide

If you are experiencing unexpected behavior while running this firmware on the STM32 Nucleo, refer to this checklist.

## 1. Ultrasonic Sensor Returns `ERR` or `-1.00cm`
- **Check Voltage:** The HC-SR04 requires 5V to register logic levels correctly. Ensure it is connected to the 5V pin on the Nucleo, NOT 3.3V.
- **Check Wiring:** Ensure TRIG is on `PB3` and ECHO is on `PA10`.
- **Logic Level Issue:** If the sensor receives 5V, its ECHO pin returns a 5V signal. Ensure `PA10` is 5V tolerant (it is on the STM32F401RE), otherwise add a voltage divider.

## 2. DHT11 Returns `Temp=--`
- **Pull-Up Resistor:** The 1-wire protocol requires a pull-up resistor (typically 4.7kΩ to 10kΩ) between the DATA pin and VCC.
- **Wire Length:** Long jumper wires can corrupt the high-frequency timings. Keep wires under 20cm if possible.
- **Sensor Death:** DHT11 sensors are fragile. Try replacing it if you consistently fail the 40-bit checksum.

## 3. LED / Buzzer Staying On
- **Active-Low Modules:** The firmware assumes your buzzer and LED modules activate when the signal is pulled to `GND`. This is why `LED_ON()` uses `GPIO_PIN_RESET`. If your modules are active-high, you must invert the macros in `main.c`.

## 4. No UART Output
- **Baud Rate Mismatch:** Ensure your terminal (PuTTY/TeraTerm) is set exactly to 115200 baud, 8 data bits, 1 stop bit, no parity.
- **ST-Link Driver:** Ensure the ST-Link Virtual COM port driver is installed.
