# ECE 528/L - Robotics and Embedded Systems with Lab
**CSU Northridge**

**Department of Electrical and Computer Engineering**

## Line Follower Lab

### Overview

The Line Follower lab combines reflectance sensing, periodic interrupts, and motor control on the MSP432 LaunchPad and TI-RSLK MAX chassis. An 8-channel QTRX array detects 3/4" black tape on a white surface. Timer A1 samples the sensors about every 10 ms. The 8-bit mask then steers the robot forward, left, or right, or stops it at a T-intersection or dead end. LaunchPad LED1 and the RGB LED show which pattern was recognized.

This lab builds on the Reflectance Sensor and Motor Control labs. By the end of the assignment we:

- Used Timer A1 to schedule sensor measurements and steering updates
- Used Timer A0 to generate motor PWM and SysTick to keep a millisecond counter
- Implemented a line-following algorithm that tracks a basic line and stops at intersections or dead ends

### Components Used

| Component | Quantity | Manufacturer |
|---|---|---|
| MSP432P401R LaunchPad | 1 | Texas Instruments |
| TI-RSLK MAX chassis with motor-driver board | 1 | Pololu |
| Pololu gearmotors with encoders | 2 | Pololu |
| Pololu 8-Channel QTRX Sensor Array | 1 | Pololu |
| LaunchPad LED1 and RGB LED | Built in | Texas Instruments |
| USB-A to Micro-USB cable | 1 | N/A |
| AA batteries | 6 | N/A |
| 3/4" black tape on white paper | N/A | N/A |
| Computer with Code Composer Studio | 1 | Texas Instruments |

**Main signal connections**

| Signal | LaunchPad pin(s) |
|---|---|
| Reflectance sensor outputs 0–7 | P7.0–P7.7 |
| Even / odd IR emitter controls | P5.3 / P9.2 |
| Sensor supply / ground | 5 V / GND |
| Right / left motor PWM | P2.6 / P2.7 |
| Left / right motor direction | P5.4 / P5.5 |
| Motor enable (nSLEEP) | P3.6–P3.7 |
| Red LED1 | P1.0 |
| RGB LED | P2.0–P2.2 |

Bit 0 is the rightmost QTRX channel. Bit 7 is the leftmost channel.

### Analysis and Results

The tables and timing diagrams below come from the flashed firmware and from placing the robot on the tape patterns used in lab. PWM numbers are Timer A0 compare values from the driver, not measured wheel speed.


**Video of robot moving along the track**

The clip below shows the TI-RSLK MAX following 3/4" black tape: it drives forward on center, corrects when the array drifts, and stops when the line ends.

[Line-follower demonstration (IMG_3958.MOV)](media/IMG_3958.MOV)

<video src="media/IMG_3958.MOV" controls width="640">
  Your browser cannot play this video. Open <a href="media/IMG_3958.MOV">IMG_3958.MOV</a> instead.
</video>


**Sensor sampling and control timing**

`Timer_A1_Periodic_Task()` runs every 1 ms. At tick 10, `Reflectance_Sensor_Start()` turns on the IR emitters, drives P7.0–P7.7 high for 10 µs, then switches those pins to inputs. At tick 11, `Reflectance_Sensor_End()` reads Port 7, turns the emitters off, and calls `Detect_Line_Position()`. A high bit is black tape; a low bit is white paper.

The millisecond counter then resets to 1, so the next start/read pairs are at 20/21 ms, 30/31 ms, and 40/41 ms. Sensor and steering updates run at about **100 Hz**, even though the interrupt is 1 kHz. The discharge window is about 1 ms.

```text
Time (in ms):       10      11              20       21
Sensor action:   Start -> Read            Start -> Read
Control action:           LEDs + motors            LEDs + motors
                          <---------- 10 ms ---------->
```

**Position calculation and steering**

`Reflectance_Sensor_Position()` averages the weights of the active sensor bits:

```text
Bit:       7     6     5    4    3    2    1    0
Weight: -334  -238  -142  -48   48  142  238  334

Position = sum of active weights / number of active sensors
```

| Calculated position | Command |
|---|---|
| At least +332 | Pivot right |
| +47 through +331 | Forward with the right wheel slower |
| -46 through +46 | Forward with equal motor commands |
| -331 through -47 | Forward with the left wheel slower |
| At most -332 | Pivot left |

**Observed LED and motor behavior**

Left and right describe tape position relative to the sensor array. Motor arguments are `(left, right)` PWM compare values.

| Pattern | Sensor mask | Position | LED1 | RGB LED | Motor command | Observed result |
|---|---|---:|---|---|---|---|
| Center | `0001_1000` (`0x18`) | 0 | Off | Green | Forward `(2500, 2500)` | Drives straight on the line |
| Tape slightly left | `0011_1000` (`0x38`) | -47 | Off | Yellow | Forward `(1500, 2500)` | Gentle left correction |
| Tape slightly right | `0001_1100` (`0x1C`) | +47 | Off | Pink | Forward `(2500, 1500)` | Gentle right correction |
| Tape far left | `1000_0000` (`0x80`) | -334 | Off | White | Pivot left `(2500, 2500)` | Spins toward the line |
| Tape far right | `0000_0001` (`0x01`) | +334 | Off | Sky blue | Pivot right `(2500, 2500)` | Spins toward the line |
| Left turn | `1111_1000` (`0xF8`) | — | On | White | Pivot left `(2500, 2500)` | Turns left at the corner |
| Right turn | `0001_1111` (`0x1F`) | — | On | Sky blue | Pivot right `(2500, 2500)` | Turns right at the corner |
| T-intersection | `1111_1111` (`0xFF`) | — | Off | Blue | Stop | Stops on the junction |
| Dead end / no tape | `0000_0000` (`0x00`) | — | Off | Red | Stop | Stops when the line is lost |
| Any other mask | Other nonzero values | Calculated | Off | Off | Steer from position | Still follows, RGB LED off |

Unlike the sensor-only lab, an unrecognized nonzero pattern still steers. For example, `0x08` has position +48 and produces a gentle right correction even though the RGB LED is off.

On the lab floor the robot tracked a straight 3/4" tape segment, corrected when the array drifted left or right, and stopped when all eight sensors saw white (end of tape) or all eight saw black (T-intersection). LED1 only turned on for the left-turn and right-turn masks.

**Motor timing calculations**

Timer A0 uses 12 MHz SMCLK divided by 8, a period constant of 15000, and up/down counting:

```text
Timer clock = 12 MHz / 8 = 1.5 MHz
PWM period  = (2 × 15000) / 1,500,000 = 20 ms
PWM rate    = 1 / 0.020 = 50 Hz
Nominal command ratio = 2500 / 15000 = 16.7%
Slow command ratio    = 1500 / 15000 = 10.0%
```

`Motor_Left()` reverses the left motor and drives the right motor forward. `Motor_Right()` does the opposite. `Motor_Stop()` disables both motors and clears their PWM compare values.

### Known Issues or Limitations

- **A dead end and a lost line look the same.** Both can read `0x00`, which stops the motors immediately. There is no search or recovery. The stop is not latched, so a later valid reading can start motion again.
- **Turns and intersections need an exact 8-bit match.** A partial junction often looks like an ordinary offset line, so the robot steers instead of turning or stopping. Consecutive samples are not filtered.
- **100 Hz updates and a 1 ms discharge window are sensitive to alignment.** Wrinkled tape, a different tape width, or ambient light can chatter bits from sample to sample. The RGB LED then flickers between a valid color and off, and the chassis can wiggle on a straight segment.
- **This firmware does not print the sensor mask at run time.** UART is initialized, but the main loop is empty and steering runs in the Timer A1 task, so lab results were observed from the LEDs and robot motion rather than a serial log.

### Author Contribution

**Feranmi**
- Wrote the Overview, Components Used, Author Contribution, and References sections of this report
- Co-wrote Analysis and Results
- Co-implemented the line-follower source (sensor timing, LED decode, and motor commands)
- Helped run the robot demonstration on the tape patterns

**Brandon**
- Co-wrote Analysis and Results
- Wrote Known Issues or Limitations
- Co-implemented the line-follower source (sensor timing, LED decode, and motor commands)
- Helped run the robot demonstration on the tape patterns

### References

1. Aaron Nanas, *ECE 528/L Lab 3: Line Follower*, Fall 2026, `ece528L_fall26_lab3_line_follower.pdf`
2. Pololu, 8-Channel QTRX Sensor Array, https://www.pololu.com/product/3672
3. Pololu, Micro Metal Gearmotor with Encoder, https://www.pololu.com/product/3675
4. Texas Instruments, *MSP432P401R SimpleLink Microcontroller LaunchPad Development Kit User's Guide*, https://docs.rs-online.com/3934/A700000006811369.pdf
5. Texas Instruments, *MSP432P4xx Technical Reference Manual* (GPIO, Timer_A, SysTick)
6. Jonathan Valvano, *Embedded Systems: Introduction to the MSP432 Microcontroller* (clock and delay routines used in this project)
7. Line Follower firmware: `ece528L_line_follower/Line_Follower/main.c`, `src/Reflectance_Sensor.c`, `src/Motor.c`, `src/Timer_A0_PWM.c`, `src/Timer_A1_Interrupt.c`
8. Prior lab reports: Reflectance Sensor, Motor Control, and GPIO
