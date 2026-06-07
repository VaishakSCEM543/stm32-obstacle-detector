# UART Telemetry Format

The STM32 obstacle detection system pushes real-time telemetry data over USART2 every 1000ms.

## Serial Port Configuration
- **Baud Rate:** 115200
- **Data Bits:** 8
- **Stop Bits:** 1
- **Parity:** None
- **Flow Control:** None

## Data Packet Structure
The output is ASCII formatted and uses `\r\n` (CRLF) for line termination.

**Format:**
`Dist=<XX.XX>cm | LDR=<BRIGHT/DARK> | Temp=<XX>C | Hum=<XX>%`

### Parameters
1. **Dist:** Distance in centimeters measured by the HC-SR04. `ERR` indicates a timeout (sensor disconnected).
2. **LDR:** Ambient light status. Detects whether the environment is currently illuminated (`BRIGHT`) or dark (`DARK`).
3. **Temp:** Ambient temperature in Celsius read from the DHT sensor.
4. **Hum:** Relative humidity percentage read from the DHT sensor.

### Example Output
```text
✅ SYSTEM STARTED ✅
Distance | LDR | Temp | Hum
Dist=12.45cm | LDR=BRIGHT | Temp=24C | Hum=55%
Dist=11.10cm | LDR=BRIGHT | Temp=24C | Hum=55%
Dist=8.02cm | LDR=DARK | Temp=24C | Hum=55%
Dist=ERR | LDR=DARK | Temp=-- | Hum=-- (DHT FAIL)
```
