/**
 * @file main.c
 * @brief Main source code for the Reflectance Sensor program.
 *
 * This file contains the main entry point and function definitions for the Reflectance Sensor program.
 * It interfaces with the following:
 *  - User LEDs of the TI MSP432 LaunchPad
 *  - 8-Channel QTRX Sensor Array module
 *
 * To verify the pinout of the user buttons and LEDs, refer to the MSP432P401R SimpleLink Microcontroller LaunchPad Development Kit User's Guide
 * Link: https://docs.rs-online.com/3934/A700000006811369.pdf
 *
 * @author
 */

#include <stdint.h>
#include "msp.h"
#include "inc/Clock.h"
#include "inc/EUSCI_A0_UART.h"
#include "inc/GPIO.h"
#include "inc/Reflectance_Sensor.h"
#include "inc/Print_Binary.h"

void Detect_Line_Position(uint8_t reflectance_sensor_data);

int main(void)
{
    // Initialize the 48 MHz Clock
    Clock_Init48MHz();

    // Initialize the built-in red LED and the RGB LEDs
    LED1_Init();
    LED2_Init();

    // By default, the user LEDs should be off
    LED1_Output(RED_LED_OFF);
    LED2_Output(RGB_LED_OFF);

    // Initialize the reflectance sensor
    Reflectance_Sensor_Init();

    // Initialize EUSCI_A0_UART
    EUSCI_A0_UART_Init_Printf();

    // Initialize a local variable to store the data read from the reflectance sensor
    uint8_t reflectance_sensor_data = 0;

    while(1)
    {
        reflectance_sensor_data = Reflectance_Sensor_Read(1000);
        Print_Binary(reflectance_sensor_data);
        Detect_Line_Position(reflectance_sensor_data);
        Clock_Delay1ms(10);
    }
}

/**
 * @brief Indicate the detected line-maze pattern using LED1 and the RGB LED.
 *
 * @param reflectance_sensor_data 8-bit QTRX sensor mask (1 = black, 0 = white).
 *
 * @return None
 */
void Detect_Line_Position(uint8_t reflectance_sensor_data)
{
    switch (reflectance_sensor_data)
    {
        // Center: 0001_1000
        case 0x18:
            LED1_Output(RED_LED_OFF);
            LED2_Output(RGB_LED_GREEN);
            break;

        // Slightly Left from Center: 0001_1100
        case 0x1C:
            LED1_Output(RED_LED_OFF);
            LED2_Output(RGB_LED_YELLOW);
            break;

        // Slightly Right from Center: 0011_1000
        case 0x38:
            LED1_Output(RED_LED_OFF);
            LED2_Output(RGB_LED_PINK);
            break;

        // Far Left from Center: 0000_0001
        case 0x01:
            LED1_Output(RED_LED_OFF);
            LED2_Output(RGB_LED_WHITE);
            break;

        // Far Right from Center: 1000_0000
        case 0x80:
            LED1_Output(RED_LED_OFF);
            LED2_Output(RGB_LED_SKY_BLUE);
            break;

        // Left Turn: 1111_1000
        case 0xF8:
            LED1_Output(RED_LED_ON);
            LED2_Output(RGB_LED_WHITE);
            break;

        // Right Turn: 0001_1111
        case 0x1F:
            LED1_Output(RED_LED_ON);
            LED2_Output(RGB_LED_SKY_BLUE);
            break;

        // T-Intersection: 1111_1111
        case 0xFF:
            LED1_Output(RED_LED_OFF);
            LED2_Output(RGB_LED_BLUE);
            break;

        // Dead End: 0000_0000
        case 0x00:
            LED1_Output(RED_LED_OFF);
            LED2_Output(RGB_LED_RED);
            break;

        default:
            LED1_Output(RED_LED_OFF);
            LED2_Output(RGB_LED_OFF);
            break;
    }
}
