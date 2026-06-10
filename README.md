# Multi-Sensor Environmental & Reverse Alert System (STM32)
[![C/C++ CI](https://github.com/VaishakSCEM543/stm32-obstacle-detector/actions/workflows/c-cpp.yml/badge.svg)](https://github.com/VaishakSCEM543/stm32-obstacle-detector/actions/workflows/c-cpp.yml)

<img width="852" height="1280" alt="WhatsApp Image 2026-06-10 at 9 28 16 PM" src="https://github.com/user-attachments/assets/0d7e1b84-583a-4c6c-992e-588c70b72b46" />


## Introduction
This repository contains the bare-metal firmware for an STM32-based multi-sensor monitoring system. Designed for real-time spatial awareness and environmental monitoring, the system integrates an HC-SR04 ultrasonic sensor for distance measurement, a DHT sensor for temperature and humidity, and a digital LDR for ambient light detection. The system provides immediate audiovisual feedback (LED & Buzzer) based on configurable safety thresholds and streams continuous telemetry via UART.

## Features
- **Real-Time Obstacle Detection:** Measures distance in cm using an ultrasonic sensor.
- **Microsecond Precision:** Utilizes the ARM Cortex-M Data Watchpoint and Trace (DWT) cycle counter to measure echo pulses and bit-bang 1-wire protocols without consuming general-purpose timers.
- **Hierarchical Alerts:** 
  - `Critical Danger (<= 15cm)`: Fast flashing LED and Buzzer.
  - `Warning (<= 30cm)`: Slow flashing LED.
- **Environmental Tracking:** Monitors temperature and humidity, triggering an alarm if safe limits are exceeded (>= 35°C or >= 80% RH).
- **Ambient Light Monitoring:** Detects low-light conditions and automatically triggers an illumination LED.
- **UART Telemetry:** Outputs formatted system status (`Dist`, `LDR`, `Temp`, `Hum`) every second at 115200 baud.

## Hardware Used
| Component | Purpose | Connection |
| --------- | ------- | ---------- |
| **STM32 Nucleo-F401RE** | Main Controller | - |
| **HC-SR04** | Distance Measurement | TRIG: `PB3`, ECHO: `PA10` |
| **DHT11/22** | Temp & Humidity | DATA: `PA0` |
| **LDR Module** | Ambient Light | DATA: `PB10` |
| **LED** | Visual Alert / Illumination | `PB4` (Active Low) |
| **Active Buzzer** | Audio Alert | `PB5` (Active Low) |

## Software Architecture
- **Environment:** STM32CubeIDE, STM32CubeMX
- **Drivers:** STM32 HAL
- **RTOS:** Bare-metal super-loop

The system employs a non-blocking `HAL_GetTick()` approach for polling intervals, while relying on the internal `DWT->CYCCNT` register for blocking microsecond delays necessary for the DHT protocol and Ultrasonic echo timing.

## Repository Structure
```
Project/
│
├── Core/
│   ├── Inc/           # Header files (main.h, stm32f4xx_hal_conf.h, etc.)
│   └── Src/           # Source files (main.c, stm32f4xx_it.c, etc.)
├── Drivers/           # STM32 HAL and CMSIS files
├── PROJECT.ioc        # STM32CubeMX Configuration File
├── README.md          # Project Documentation
├── LICENSE            # Open Source License
└── .gitignore
```

## Setup & Build
1. **Clone the repository.**
2. **Open STM32CubeIDE.**
3. Navigate to `File > Import > General > Existing Projects into Workspace` and select the project folder.
4. **Build:** Click the Hammer icon (Build). Ensure zero errors.

## Flashing & Usage
1. Connect the STM32 Nucleo-F401RE to your PC via USB.
2. Wire the sensors according to the Hardware connections table.
   > **Note:** Ensure 5V logic requirements for HC-SR04 are met.
3. Click the green Play button (Run) in STM32CubeIDE to flash the firmware.
4. Open a serial terminal (e.g., PuTTY) connected to the ST-Link Virtual COM port.
   - **Baud Rate:** 115200
   - **Data Bits:** 8
   - **Stop Bits:** 1
   - **Parity:** None
5. You should see `✅ SYSTEM STARTED ✅` followed by 1Hz telemetry updates.

## Results & Expected Output
```text
✅ SYSTEM STARTED ✅
Distance | LDR | Temp | Hum
Dist=14.50cm | LDR=BRIGHT | Temp=24C | Hum=45%
Dist=32.10cm | LDR=DARK   | Temp=24C | Hum=45%
```

## Troubleshooting
- **`Dist=ERR` on UART:** The ultrasonic echo timed out. Check the wiring to `PA10` and `PB3`, and ensure the sensor is receiving 5V.
- **`Temp=-- (DHT FAIL)`:** The DHT sensor failed to respond. Check the pull-up resistor on the data line or verify the connection to `PA0`.
- **Alerts staying ON constantly:** The LED and Buzzer outputs are configured for active-low modules (`GPIO_PIN_RESET` turns them ON). If using active-high modules, invert the logic in the `LED_ON`/`BUZZER_ON` macros in `main.c`.

## Future Work
- Implement FreeRTOS to manage sensor reading threads.
- Replace bare-metal polling with DMA and Timer Input Capture for the ultrasonic sensor to completely unblock the CPU.
- Add CAN bus support for vehicle integration.
