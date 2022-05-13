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

volatile int16_t fftOutput[NUM_SAMPLES * 2];
float magBuffer[NUM_SAMPLES];
uint32_t setFFTmaxFreqIndex;
char usedDmaCh = 0;

void GetPowerMag(unsigned int *dat, unsigned int *FFT_Mag, unsigned int len)
{
    signed short lX, lY;
    float X, Y, Mag;
    unsigned short i;
    unsigned int *FFT_OutData = dat;
#if 1
    for (i = 0; i < len / 2; i++)
    {
        lX = (FFT_OutData[i] << 16) >> 16;
        lY = (FFT_OutData[i] >> 16);

        X = len * ((float)lX) / 32768;
        Y = len * ((float)lY) / 32768;

        arm_sqrt_f32(X * X + Y * Y, &Mag);
        Mag = Mag / len;
        if (i == 0)
            FFT_Mag[i] = (unsigned long)(Mag * 32768);
        else
            FFT_Mag[i] = (unsigned long)(Mag * 65536);
    }
#else
    for (i = 0; i < len / 2; i++)
    {
        X = (FFT_OutData[i] << 16) >> 16;
        Y = (FFT_OutData[i] >> 16);
        FFT_Mag[i] = sqrt(X * X + Y * Y);
    }
#endif
}

float GetTHD(unsigned int *dat, unsigned int index)
{
    float Uo1 = 0, Uo2 = 0, Uo3 = 0, Uo4 = 0, Uo5 = 0;
    double THD, thd_fz = 0, thd_fm = 0;
    Uo1 = dat[index * 1];
    Uo2 = dat[index * 2];
    Uo3 = dat[index * 3];
    Uo4 = dat[index * 4];
    Uo5 = dat[index * 5];
    thd_fm = Uo1;
    thd_fz = Uo2 * Uo2 + Uo3 * Uo3 + Uo4 * Uo4 + Uo5 * Uo5;
    thd_fz = sqrt(thd_fz);
    THD = thd_fz / thd_fm * 100;

    return THD;
}

//extern uint32_t g_ui32Base;
bool transfrom_waves()
{
//    int i,j;
//    char msg[7] = {0xa5,0,0,0,0,0,0x5a};

    volatile float32_t rmsBuff, dcBuff;
    float32_t rmsCalculation;
    uint32_t ii;
    int_fast32_t i32IPart[3], i32FPart[3];
    bool isBufAReady;

    uint32_t *dstBuffer[2] = {dstBufferA, dstBufferB};
    volatile bool *setBufReady[2] = {&setBufAReady, &setBufBReady};
    static char ch = 0;

    if (isBufAReady = *setBufReady[ch])
    {
        usedDmaCh = ch;
        *setBufReady[ch] = false;
        dcBuff = 0.0f;
        rmsBuff = 0.0f;

        /* First convert the sampled data to floating point format as the RMS
         * and DC average is being computed */
        for (ii = 0; ii < NUM_SAMPLES; ii++)
        {
            rmsBuff += ((float32_t)(dstBuffer[ch][ii] * dstBuffer[ch][ii]) * (float32_t)(33 * 33)) / (float32_t)(40960 * 40960);
            dcBuff += (float32_t)(dstBuffer[ch][ii] * 33) / (float32_t)40960;
        }

        /* Now calculate the final DC Average and RMS */
        dcBuff = dcBuff / ((float32_t)NUM_SAMPLES);
        rmsBuff = rmsBuff / ((float32_t)NUM_SAMPLES);
        arm_sqrt_f32(rmsBuff, &rmsCalculation);

        /* Compute the 1024 point FFT on the sampled data and then find the
         * FFT point for maximum energy and the energy value */
        arm_cfft_q15(&arm_cfft_sR_q15_len1024, (q15_t *)dstBuffer[ch], IFFTFLAG, BITREVERSE);

        /*
         * UARTprintf begin
         */

        int i;
        for(i = 0; i < NUM_SAMPLES; i++) {
            arm_status status = arm_sqrt_f32((int16_t)(dstBuffer[ch][i]/65536) * (int16_t)(dstBuffer[ch][i]/65536) + \
                                             (int16_t)(dstBuffer[ch][i]%65536) * (int16_t)(dstBuffer[ch][i]%65536), \
                                             &magBuffer[i]);
        }

        /*
         * UARTprintf end
         */

        /* Convert the floating point values to integer and fractional parts
         * for display on the serial console */
        i32IPart[0] = (int32_t)rmsCalculation;
        i32FPart[0] = (int32_t)(rmsCalculation * 1000.0f);
        i32FPart[0] = (int32_t)(i32FPart[0] - i32IPart[0] * 1000.0);
        i32IPart[1] = (int32_t)dcBuff;
        i32FPart[1] = (int32_t)(dcBuff * 1000.0f);
        i32FPart[1] = (int32_t)(i32FPart[1] - i32IPart[1] * 1000.0);
        i32IPart[2] = (int32_t)((setFFTmaxFreqIndex * SAMP_FREQ) / NUM_SAMPLES);
        i32FPart[2] = (int32_t)(((setFFTmaxFreqIndex * SAMP_FREQ) / NUM_SAMPLES) * 1000);
        i32FPart[2] = i32FPart[2] - i32IPart[2] * 1000;

        /* Print the DC Average, RMS, Maximum FFT Amplitude and the FFT
         * frequency at which Max FFT Amplitude is detected */
        UARTprintf("rms:%3d.%03d Volts\n", i32IPart[0], i32FPart[0]);
        UARTprintf("dc:%3d.%03d Volts\n\n", i32IPart[1], i32FPart[1]);
//        UARTprintf("FFTmaxValue:%03d\n", setFFTmaxValue);
//        UARTprintf("mainFreq:%3d.%03d Hz\n\n", i32IPart[2], i32FPart[2]);
        for(ii = 0; ii < NUM_SAMPLES; ii++)
        {
            if(fftOutput[ii])
            {
                i32IPart[2] = (int32_t)((ii * SAMP_FREQ) / NUM_SAMPLES);
                i32FPart[2] = (int32_t)(((ii * SAMP_FREQ) / NUM_SAMPLES) * 1000);
                UARTprintf("Freq:%3d.%03d Hz\n", i32IPart[2], i32FPart[2]);
                UARTprintf("Value:%03d\n\n", fftOutput[ii]);
            }
        }

//        if(ch)
//        for(i=0; i<NUM_SAMPLES; ++i)
//        {
//            msg[5] = 0;
//            msg[5] += msg[1] = dstBuffer[ch][i];
//            msg[5] += msg[2] = dstBuffer[ch][i]>>8;
//            msg[5] += msg[3] = dstBuffer[ch][i]>>16;
//            msg[5] += msg[4] = dstBuffer[ch][i]>>24;
//            for(j=0; j<7; ++j)
//                MAP_UARTCharPut(g_ui32Base, msg[j]);
//        }

        ch = !ch;
    }
    return isBufAReady;
}
