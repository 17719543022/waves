/*
 * waves.c
 *
 *  Created on: 2021Äê12ÔÂ10ÈÕ
 *      Author: JiayunLi
 */

#include "ti/devices/msp432e4/driverlib/driverlib.h"
#include <stdlib.h>
#include <stdbool.h>
#include "arm_math.h"
#include "arm_const_structs.h"
#include "uartstdio.h"
#include "adc.h"

#define IFFTFLAG 0
#define BITREVERSE 1

float magBuffer[NUM_SAMPLES];
q31_t correlation[NUM_SAMPLES*2-1];

bool transfrom_waves()
{
    uint32_t i;
    bool isBufAReady = false;
    bool isBufBReady = false;

    if ((isBufAReady = setBufAReady) && (isBufBReady = setBufBReady))
    {
        setBufAReady = false;
        setBufBReady = false;

        /* compute samples number of single period with self correlation */
//        arm_correlate_q31((const q31_t *)dstBufferA, NUM_SAMPLES, (const q31_t *)dstBufferA, NUM_SAMPLES, correlation);

        /* Compute the 1024 point FFT on the sampled data and then find the
         * FFT point for maximum energy and the energy value */
        arm_cfft_q15(&arm_cfft_sR_q15_len1024, (q15_t *)dstBufferA, IFFTFLAG, BITREVERSE);

        for(i = 0; i < NUM_SAMPLES; i++) {
            arm_status status = arm_sqrt_f32((int16_t)(dstBufferA[i]/65536) * (int16_t)(dstBufferA[i]/65536) + \
                                             (int16_t)(dstBufferA[i]%65536) * (int16_t)(dstBufferA[i]%65536), \
                                             &magBuffer[i]);
        }

        UARTprintf("dc:%3d.%03d Volts\n\n", 0, 0);
    }

    return isBufAReady & isBufBReady;
}
