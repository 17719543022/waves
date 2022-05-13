#include <ti/devices/msp432e4/driverlib/driverlib.h>
#include "systemClock.h"
#include "uart.h"
#include "uartstdio.h"
#include "hmi.h"
#include "adc.h"
#include "waves.h"
#include "gpio.h"

void main(void)
{
    int a = 0;

    systemClock_init();
    ConfigureUART(systemClock);
    gpioInit();
//    ConfigureUART_HMI(systemClock);

    while(true)
    {
        if(transfrom_waves())
        {

            if(++a >= 20)
            {
                a = 0;

//                display_hmi();
            }
        }

        btnPressedEvent();
    }
}
