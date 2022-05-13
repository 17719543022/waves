/*
 * uart.c
 *
 *  Created on: 2021Äê12ÔÂ10ÈÕ
 *      Author: JiayunLi
 */

#include <ti/devices/msp432e4/driverlib/driverlib.h>
#include "uartstdio.h"

void ConfigureUART(uint32_t systemClock)
{
    /* Enable the clock to GPIO port A and UART 0 */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    /* Configure the GPIO Port A for UART 0 */
    MAP_GPIOPinConfigure(GPIO_PA0_U0RX);
    MAP_GPIOPinConfigure(GPIO_PA1_U0TX);
    MAP_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    /* Configure the UART for xxxxx bps 8-N-1 format */
    UARTStdioConfig(0, 115200, systemClock);
}
