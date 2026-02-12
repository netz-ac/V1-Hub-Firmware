# Alternative Firmware for Vecos V1 Hub

This alternative firmware implements a custom communication protocol that allows the Vecos V1 Hub to function as a simple, universal controller for up to 16 Vecos V1 locks without requiring a Vecos terminal.
The firmware is written in C++ and can be flashed using PlatformIO.

The hub uses a ATmega328 microcontroller with 7,372,800 Hz clock speed.
For its input and output, it utilizes shift registers to control the locks and read their states.
SN74AHC595 shift registers are used for output control, while SNx4HC166 shift registers are used for input reading.

## Features
- Control up to 16 Vecos V1 locks.
- Read the state of each lock (open/closed).
- Read the presence of each lock (connected/disconnected).
- Manage power state for locks and USB ports/lights in the compartment.
- Communicate with the hub using RS485 protocol.

## Hardware Setup
1. Connect the Vecos V1 locks to the hub.
2. Connect the RS485 interface to the hub via RJ45. The pin configuration is as follows: 4 to A and 5 to B. We leave GND unconnected.
3. Connect the power supply (22 to 28 V DC) to the hub.

## Our Custom Communication Protocol
### Request Format
Send the following data via RS485 with a baud rate of 9600 to control the locks:

```
0xFF 0x00 [LOCKS_1_8] 0x00 0x00 0x00 0x00 [LOCKS_9_16] 0x00 [LIGHTING_USB]
```

- **[LOCKS_1_8]**: 1 byte for locks 1-8, where the MSB corresponds to lock 1.
- **[LOCKS_9_16]**: 1 byte for locks 9-16, where the MSB corresponds to lock 9.
- **[LIGHTING_USB]**: 1 byte, use `0x02` to turn on lights/USB or `0x00` to turn it off.

### Response Format
The hub responds with 66 bits in 9 bytes:

1. **Byte 1**: State of doors 8-1 (LSB corresponds to door 1).
2. **Byte 2**: Set state of locks 1-8 (irrelevant for us).
3. **Byte 3**: Connection status of locks 8-1.
4. **Byte 4**: Unused (Reads out DIP switches).
5. **Byte 5**: Unused (Reads out DIP switches).
6. **Byte 6**: State of doors 16-9.
7. **Byte 7**: Set state of locks 9-16 (irrelevant for us).
8. **Byte 8**: Connection status of locks 16-9.
9. **Byte 9**: 
   - Second-to-last bit: USB/Light power supply state.
   - Last bit: USB/Light power supply feedback.
   - Remaining bits are unused.

## How to Use
1. Flash the firmware to the Vecos V1 Hub using PlatformIO and SPI (see below). Note: Use the optiboot bootloader, otherwise it will not boot.
2. Connect the hardware as described in the **Hardware Setup** section.
3. Use an RS485 communication interface to send commands to the hub.

### SPI Connection for Flashing
To flash the firmware using SPI, disconnect the hub from power, open the hub and connect the programmer to P4 pin header on the PCB as follows:

| Programmer Pin | P4 Pin |
|----------------|--------|
| MISO           | 1      |
| VCC            | 2      |
| SCK            | 3      |
| MOSI           | 4      |
| RESET          | 5      |
| GND            | 6      |

Pin 1 is indicated on the PCB.

## Notes
- The firmware is designed for the Vecos V1 Hub and may not work with other hardware.

Hardware documentation can be found here:
https://www.houtenlocker.nl/images/Documenten/Vecos_V1.pdf

### Pin Configuration
For documentation purposes, the pin configuration of the ATmega328 is as follows:

| Pin Name            | Arduino Framework Pin | ATmega Pin | Description                          |
|---------------------|-----------------------|------------|--------------------------------------|
| RS485_ENABLE        | 2                     | PD2        | RS485 transmission enable pin.      |
| OUTPUT_ENABLED      | 7                     | PD7        | Output shift register enable pin.   |
| SHIFT_MODE          | 9                     | PB1        | Shift mode control pin.             |
| POWER_OUTPUT        | 15                    | PC1        | Power output for locks.             |
| POWER_FEEDBACK_PIN  | 14                    | PC0        | Power feedback input pin.           |
| DATA_OUT_PIN        | 3                     | PD3        | Data output pin for shift register. |
| DATA_IN_PIN         | 4                     | PD4        | Data input pin for shift register.  |
| CLOCK_PIN           | 5                     | PD5        | Clock pin for shift register.       |
| LATCH_PIN           | 8                     | PB0        | Latch pin for shift register.       |

## Disclaimer
Vecos is a registered trademark of Vecos IPCo B.V. This project is an independent development and is not affiliated with or endorsed by Vecos. The firmware is provided as-is, without any guarantees or warranties. Use this firmware at your own risk.
