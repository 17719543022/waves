/*
 * waves.h
 *
 *  Created on: 2021Äê12ÔÂ10ÈÕ
 *      Author: JiayunLi
 */

#ifndef INCLUDE_WAVES_H_
#define INCLUDE_WAVES_H_

#include "adc.h"

float GetTHD(unsigned int *dat,unsigned int index);
bool transfrom_waves(void);

extern volatile int16_t fftOutput[NUM_SAMPLES*2];
extern uint32_t setFFTmaxFreqIndex;
extern char usedDmaCh;

#endif /* INCLUDE_WAVES_H_ */
