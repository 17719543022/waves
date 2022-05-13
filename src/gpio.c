/*
 * gpio.c
 *
 *  Created on: 2022Äê2ÔÂ25ÈÕ
 *      Author: JiayunLi
 */

#include "ti/devices/msp432e4/driverlib/driverlib.h"
#include <stdlib.h>
#include <stdbool.h>
#include "systemClock.h"
#include "waves.h"
#include "adc.h"

void gpioInit()
{
    /* Enable the clock to the GPIO Port N and wait for it to be ready */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);
    while (!(SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ)));

    /* Configure the GPIO PJ0 as input */
    MAP_GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_0);
    GPIOPadConfigSet(GPIO_PORTJ_BASE, GPIO_PIN_0, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
}

bool isBtnPressed()
{
    bool btnPressed = false;
    if (!MAP_GPIOPinRead(GPIO_PORTJ_BASE, GPIO_PIN_0))
    {
        delay(20);
        if (!MAP_GPIOPinRead(GPIO_PORTJ_BASE, GPIO_PIN_0))
        {
            while (!MAP_GPIOPinRead(GPIO_PORTJ_BASE, GPIO_PIN_0));
            btnPressed = true;
        }
    }
    return btnPressed;
}

void btnPressedEvent()
{
    if (isBtnPressed())
    {
        adc_with_uDMA_init();

//        MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
//        MAP_TimerEnable(TIMER0_BASE, TIMER_A);
//        if (usedDmaCh == 0)
//            MAP_uDMAChannelTransferSet(UDMA_CH16_ADC0_2 | UDMA_PRI_SELECT,
//                                       UDMA_MODE_PINGPONG,
//                                       (void *)&ADC0->SSFIFO2, (void *)&dstBufferA,
//                                       sizeof(dstBufferA) / 4);
//        else if(usedDmaCh == 1)
//            MAP_uDMAChannelTransferSet(UDMA_CH16_ADC0_2 | UDMA_ALT_SELECT,
//                                       UDMA_MODE_PINGPONG,
//                                       (void *)&ADC0->SSFIFO2, (void *)&dstBufferB,
//                                       sizeof(dstBufferB) / 4);
//        usedDmaCh = 2;
    }
}
