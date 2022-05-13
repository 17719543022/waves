/*
 * adc.h
 *
 *  Created on: 2021Äê12ÔÂ10ÈÕ
 *      Author: JiayunLi
 */

#ifndef INCLUDE_ADC_H_
#define INCLUDE_ADC_H_

/* Define for Samples to be captured and Sampling Frequency */
#define NUM_SAMPLES 1024
#define SAMP_FREQ   1024000

/* DMA Buffer declaration and buffer complet flag */
extern uint32_t dstBufferA[NUM_SAMPLES];
extern uint32_t dstBufferB[NUM_SAMPLES];

extern volatile bool setBufAReady;
extern volatile bool setBufBReady;

void adc_with_uDMA_init(void);

#endif /* INCLUDE_ADC_H_ */
