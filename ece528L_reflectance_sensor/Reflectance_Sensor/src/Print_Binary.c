/**
 * @file Print_Binary.c
 * @brief Source code for Print_Binary function implementation.
 *
 * This file contains the function implementation for the Print_Binary function,
 * which takes in a uint8_t value and prints the value in binary format.
 * It relies on the EUSCI_A0_UART driver.
 *
 * @author Aaron Nanas
 *
 */

#include "../inc/Print_Binary.h"

void Print_Binary(uint8_t value_to_convert)
{
    int i;

    if (value_to_convert == 0)
    {
        printf("Binary: 0000_0000\n");
        return;
    }

    printf("Binary: ");

    for (i = 7; i >= 0; i--)
    {
        if (value_to_convert & (1 << i))
        {
            printf("1");
        }
        else
        {
            printf("0");
        }

        if (i == 4)
        {
            printf("_");
        }
    }

    printf("\n");
}
