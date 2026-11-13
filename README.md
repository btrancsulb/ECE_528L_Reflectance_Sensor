# ECE 528/L - Robotics and Embedded Systems with Lab
**CSU Northridge**

**Department of Electrical and Computer Engineering**

## Lab 2: Reflectance Sensor

### Overview
This lab interfaces the TI MSP432P401R LaunchPad with the Pololu 8-Channel QTRX reflectance sensor array on the TI-RSLK MAX chassis. The program charges the eight sensor capacitors, waits a fixed decay time, and reads Port 7 as an 8-bit mask where `1` indicates black tape and `0` indicates a white surface. That mask is printed over UART in binary, then decoded in `Detect_Line_Position()` so LED1 and the RGB LED identify line position and maze features (center, offset left/right, left/right turns, T-intersection, and dead end).

The software is organized as:
- `Reflectance_Sensor_Init()` / `Reflectance_Sensor_Read()` in `src/Reflectance_Sensor.c`
- `Print_Binary()` in `src/Print_Binary.c`
- `Detect_Line_Position()` and the main loop in `main.c`

### Components Used
- TI MSP432P401R LaunchPad (MSP432P401R microcontroller)
- TI-RSLK MAX robot chassis
- Pololu 8-Channel QTRX Sensor Array ([product page](https://www.pololu.com/product/3672))
- LaunchPad user LED1 (P1.0) and RGB LED (P2.0–P2.2)
- USB UART connection on P1.2 / P1.3 (EUSCI_A0) for serial output
- 3/4" black electrical tape on white paper for T-intersection, left turn, right turn, and dead-end patterns

**QTRX pinout used in this lab**

| Sensor signal | LaunchPad pin |
|---|---|
| Sensor outputs 0–7 | P7.0–P7.7 |
| Even IR LED control | P5.3 |
| Odd IR LED control | P9.2 |
| VCC | 5 V |
| GND | GND |

### Analysis and Results
`Reflectance_Sensor_Init()` configures P5.3 and P9.2 as GPIO outputs and leaves both IR LED banks off. P7.0–P7.7 are configured as GPIO inputs.

`Reflectance_Sensor_Read(time)` then performs the QTRX charge/decay sequence:
1. Turn on even IR LEDs (P5.3) and odd IR LEDs (P9.2).
2. Drive P7.0–P7.7 high as outputs for 10 µs to charge the sensor capacitors.
3. Switch P7.0–P7.7 back to inputs.
4. Wait `time` microseconds (1000 µs in `main`).
5. Snapshot `P7->IN` into `reflectance_value`.
6. Turn the IR LEDs off and return the 8-bit result.

Black tape discharges slowly, so those bits remain `1` after 1000 µs. A white surface discharges quickly, so those bits read `0`. `Print_Binary()` sends the mask to the serial terminal in the form `Binary: xxxx_xxxx`. A value of `0` prints `Binary: 0000_0000`.

`Detect_Line_Position()` maps exact 8-bit patterns to the LaunchPad LEDs. The LEDs start off after initialization. Observed mappings:

| Line maze possibility | Sensor value | UART output | LED1 | RGB LED |
|---|---|---|---|---|
| Center | `0001_1000` (`0x18`) | `Binary: 0001_1000` | Off | Green |
| Slightly left of center | `0001_1100` (`0x1C`) | `Binary: 0001_1100` | Off | Yellow |
| Slightly right of center | `0011_1000` (`0x38`) | `Binary: 0011_1000` | Off | Pink |
| Far left of center | `0000_0001` (`0x01`) | `Binary: 0000_0001` | Off | White |
| Far right of center | `1000_0000` (`0x80`) | `Binary: 1000_0000` | Off | Sky blue |
| Left turn | `1111_1000` (`0xF8`) | `Binary: 1111_1000` | On | White |
| Right turn | `0001_1111` (`0x1F`) | `Binary: 0001_1111` | On | Sky blue |
| T-intersection | `1111_1111` (`0xFF`) | `Binary: 1111_1111` | Off | Blue |
| Dead end | `0000_0000` (`0x00`) | `Binary: 0000_0000` | Off | Red |
| Default (any other mask) | --- | printed mask | Off | Off |

Serial output from `Print_Binary()` was captured on COM5 while the robot was running. Each line is the 8-bit QTRX mask with an underscore after bit 4:

<<<<<<< HEAD

![UART output from Print_Binary with no tape](ece528L_reflectance_sensor/Reflectance_Sensor/src/Screenshot%202026-10-22%20150430.png)

Over white paper with no black tape, every channel reads `0` (dead end):

```
Binary: 0000_0000
```

<<<<<<< HEAD
**Position calculation and steering**
=======
![UART binary output from Print_Binary on COM5](ece528L_reflectance_sensor/Reflectance_Sensor/src/Screenshot%202026-10-08%20204502.png)
>>>>>>> parent of 0d801ad (Updated README with new UART output images and added explanation for reflectance sensor readings over white paper. Improved clarity of sensor data representation.)
=======
![output from Print_Binary on COM5](ece528L_reflectance_sensor/Reflectance_Sensor/src/Screenshot%202026-10-08%20204502.png)
>>>>>>> parent of d7fc713 (Revamped README to reflect changes for the Line Follower Lab, detailing the integration of reflectance sensing, motor control, and timer functionalities. Updated component list, signal connections, and analysis sections for clarity and completeness. Included video demonstration of the robot's performance on the track.)

```
Binary: 1101_0110
Binary: 1001_0000
Binary: 0100_1100
Binary: 1111_1001
Binary: 1001_0010
Binary: 0000_1011
```

These live readings show that the sensor bits change as the array moves over black tape and white paper. Exact table matches such as `0001_1000` (center) or `0000_0000` (dead end) light the corresponding RGB color; nearby masks like the ones above fall into the default case until the robot is aligned with a defined pattern.

The tape patterns required by the lab (T-intersection, left turn, right turn, and dead end) were used to confirm that the UART bitmask and LED color change together when the robot is placed on each feature.

### Known Issues or Limitations
- Detection uses an exact `switch` match on the 8-bit mask. Tape that is slightly off-center, wrinkled, or a different width than 3/4" often produces a nearby pattern (for example `0000_1000` instead of `0001_1000`). Those values fall into the default case, so both LEDs turn off even though a line is present.
- The 1000 µs wait is a single threshold for all eight channels. Ambient light, battery voltage, and surface color can shift which bits are still high at the sample instant. If bits chatter between samples, the RGB LED will flicker between a valid color and off.
- `Print_Binary()` special-cases zero and returns immediately. That is required by the lab prompt, but it is the same visual format as a normal print of `0`.
- This program only classifies line position. It does not steer the motors; line following is a later lab.

### Author Contribution
This lab was completed with the help of Feranmi

- Implemented `Reflectance_Sensor_Init()` and `Reflectance_Sensor_Read()` using bitwise `|=` / `&=` register updates.
- Implemented `Print_Binary()` with the `xxxx_xxxx` format and the zero case `Binary: 0000_0000`.
- Implemented `Detect_Line_Position()` with switch-case LED decoding and called it from the main loop.
- Tested the sensor over white paper and black tape line-maze features.
- Wrote this lab report.

### References
- ECE 528/L Lab 2: Reflectance Sensor lab manual (CSU Northridge)
- Pololu 8-Channel QTRX Sensor Array, https://www.pololu.com/product/3672
- MSP432P401R SimpleLink Microcontroller LaunchPad Development Kit User's Guide, https://docs.rs-online.com/3934/A700000006811369.pdf
- Texas Instruments MSP432P4xx Technical Reference Manual (GPIO, EUSCI_A0 UART)
- Jonathan Valvano, *Embedded Systems: Introduction to the MSP432 Microcontroller* (clock delay routines used by this project)
- TI-RSLK MAX robotics kit documentation (QTRX mounting and LaunchPad pin map)
