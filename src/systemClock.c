/*
 * systemClock.c
 *
 *  Created on: 2021Äê12ÔÂ10ÈÕ
 *      Author: JiayunLi
 */

#include <ti/devices/msp432e4/driverlib/driverlib.h>

uint32_t systemClock;

void systemClock_init()
{
    /* Configure the system clock for 120 MHz */
    systemClock = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                                          SYSCTL_OSC_MAIN |
                                          SYSCTL_USE_PLL |
                                          SYSCTL_CFG_VCO_480),
                                         120000000);

    MAP_SysTickPeriodSet(systemClock/1000);
    MAP_SysTickEnable();
}

void delay(uint32_t ms)
{
    uint32_t curTime = SysTickValueGet();
    while(SysTickValueGet() - curTime < ms);
}
