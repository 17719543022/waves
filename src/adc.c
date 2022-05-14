/*
 * adc.c
 *
 *  Created on: 2021Äê12ÔÂ10ÈÕ
 *      Author: JiayunLi
 */

#include <ti/devices/msp432e4/driverlib/driverlib.h>
#include "systemClock.h"
#include "adc.h"

/* DMA Buffer declaration and buffer complete flag */
uint32_t dstBufferA[NUM_SAMPLES];
uint32_t dstBufferB[NUM_SAMPLES];

volatile bool setBufAReady = false;
volatile bool setBufBReady = false;

/* The control table used by the uDMA controller.  This table must be aligned
 * to a 1024 byte boundary. */
#if defined(__ICCARM__)
#pragma data_alignment=1024
uint8_t pui8ControlTable[1024];
#elif defined(__TI_ARM__)
#pragma DATA_ALIGN(pui8ControlTable, 1024)
uint8_t pui8ControlTable[1024];
#else
uint8_t pui8ControlTable[1024] __attribute__ ((aligned(1024)));
#endif

void uMDA_for_adc_init()
{
    /* Enable the DMA and Configure Channel for TIMER0A for Ping Pong mode of transfer */
   MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA);
   while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_UDMA)))
   {
   }

   MAP_uDMAEnable();

   /* Point at the control table to use for channel control structures. */
   MAP_uDMAControlBaseSet(pui8ControlTable);

   /* Map the ADC0 Sequencer 2 DMA channel */
   MAP_uDMAChannelAssign(UDMA_CH16_ADC0_2);

   /* Put the attributes in a known state for the uDMA ADC0 Sequencer 2
    * channel. These should already be disabled by default. */
   MAP_uDMAChannelAttributeDisable(UDMA_CH16_ADC0_2,
                                   UDMA_ATTR_ALTSELECT | UDMA_ATTR_USEBURST |
                                   UDMA_ATTR_HIGH_PRIORITY |
                                   UDMA_ATTR_REQMASK);

   /* Configure the control parameters for the primary control structure for
    * the ADC0 Sequencer 2 channel. The primary control structure is used for
    * copying the data from ADC0 Sequencer 2 FIFO to dstBufferA. The transfer
    * data size is 32 bits and the source address is not incremented while
    * the destination address is incremented at 32-bit boundary.
    */
   MAP_uDMAChannelControlSet(UDMA_CH16_ADC0_2 | UDMA_PRI_SELECT,
                             UDMA_SIZE_32 | UDMA_SRC_INC_NONE |
                             UDMA_DST_INC_32 | UDMA_ARB_1);

   /* Set up the transfer parameters for the ADC0 Sequencer 2 primary control
    * structure. The mode is Basic mode so it will run to completion. */
   MAP_uDMAChannelTransferSet(UDMA_CH16_ADC0_2 | UDMA_PRI_SELECT,
                              UDMA_MODE_PINGPONG,
                              (void *)&ADC0->SSFIFO2, (void *)&dstBufferA,
                              sizeof(dstBufferA)/4);

   /* Configure the control parameters for the alternate control structure for
    * the ADC0 Sequencer 2 channel. The alternate control structure is used for
    * copying the data from ADC0 Sequencer 2 FIFO to dstBufferB. The transfer
    * data size is 32 bits and the source address is not incremented while
    * the destination address is incremented at 32-bit boundary.
    */
   MAP_uDMAChannelControlSet(UDMA_CH16_ADC0_2 | UDMA_ALT_SELECT,
                             UDMA_SIZE_32 | UDMA_SRC_INC_NONE |
                             UDMA_DST_INC_32 | UDMA_ARB_1);

   /* Set up the transfer parameters for the ADC0 Sequencer 2 alternate
    * control structure. The mode is Basic mode so it will run to
    * completion */
   MAP_uDMAChannelTransferSet(UDMA_CH16_ADC0_2 | UDMA_ALT_SELECT,
                              UDMA_MODE_PINGPONG,
                              (void *)&ADC0->SSFIFO2, (void *)&dstBufferB,
                              sizeof(dstBufferB)/4);

   /* The uDMA ADC0 Sequencer 2 channel is primed to start a transfer. As
    * soon as the channel is enabled and the Timer will issue an ADC trigger,
    * the ADC will perform the conversion and send a DMA Request. The data
    * transfers will begin. */
   MAP_uDMAChannelEnable(UDMA_CH16_ADC0_2);
}

void timer0_for_adc_init()
{
    /* Enable Timer-0 clock and configure the timer in periodic mode with
     * a frequency of 1 KHz. Enable the ADC trigger generation from the
     * timer-0. */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0)))
    {
    }

    MAP_TimerConfigure(TIMER0_BASE, TIMER_CFG_A_PERIODIC);
    MAP_TimerLoadSet(TIMER0_BASE, TIMER_A, (systemClock/SAMP_FREQ));
    MAP_TimerADCEventSet(TIMER0_BASE, TIMER_ADC_TIMEOUT_A);
    MAP_TimerControlTrigger(TIMER0_BASE, TIMER_A, true);
    MAP_TimerEnable(TIMER0_BASE, TIMER_A);
}

void adc_with_uDMA_init()
{
    /* Enable the clock to GPIO Port E and wait for it to be ready */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE)))
    {
    }

    /* Configure PE3 as ADC input channel */
    MAP_GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3);

    /* Enable the clock to ADC-0 and wait for it to be ready */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0)))
    {
    }

    /* Configure Sequencer 2 to sample the analog channel : AIN0. The end of
     * conversion and interrupt generation is set for AIN0 */
    MAP_ADCSequenceStepConfigure(ADC0_BASE, 2, 0, ADC_CTL_CH0 | ADC_CTL_IE |
                                 ADC_CTL_END);

    /* Enable sample sequence 2 with a timer signal trigger.  Sequencer 2
     * will do a single sample when the timer generates a trigger on timeout*/
    MAP_ADCSequenceConfigure(ADC0_BASE, 2, ADC_TRIGGER_TIMER, 2);

    /* Clear the interrupt status flag before enabling. This is done to make
     * sure the interrupt flag is cleared before we sample. */
    MAP_ADCIntClearEx(ADC0_BASE, ADC_INT_DMA_SS2);
    MAP_ADCIntEnableEx(ADC0_BASE, ADC_INT_DMA_SS2);

    /* Enable the DMA request from ADC0 Sequencer 2 */
    MAP_ADCSequenceDMAEnable(ADC0_BASE, 2);

    /* Since sample sequence 2 is now configured, it must be enabled. */
    MAP_ADCSequenceEnable(ADC0_BASE, 2);

    /* Enable the Interrupt generation from the ADC-0 Sequencer */
    MAP_IntEnable(INT_ADC0SS2);

    uMDA_for_adc_init();
    timer0_for_adc_init();
}

void ADC0SS2_IRQHandler(void)
{
    uint32_t getIntStatus;
    uint32_t getDMAStatus;

    /* Get the interrupt status from the ADC */
    getIntStatus = MAP_ADCIntStatusEx(ADC0_BASE, true);

    /* Clear the ADC interrupt flag. */
    MAP_ADCIntClearEx(ADC0_BASE, getIntStatus);

    /* Read the primary and alternate control structures to find out which
     * of the structure has completed and generated the done interrupt. Then
     * re-initialize the appropriate structure */
    getDMAStatus = MAP_uDMAChannelModeGet(UDMA_CH16_ADC0_2 |
                                          UDMA_PRI_SELECT);

    /* Check if the primary or alternate channel has completed. On completion
     * re-initalize the channel control structure. If the Primary channel has
     * completed then set Buffer-A ready flag so that the main application
     * may perform the DSP computation. Similarly if the Alternate channel
     * has completed then set Buffer-B ready flag so that the main application
     * may perform the DSP computation. */
    if(getDMAStatus == UDMA_MODE_STOP)
    {
        MAP_uDMAChannelTransferSet(UDMA_CH16_ADC0_2 | UDMA_PRI_SELECT,
                                   UDMA_MODE_PINGPONG,
                                   (void *)&ADC0->SSFIFO2, (void *)&dstBufferA,
                                   sizeof(dstBufferA)/4);
        setBufAReady = true;
//        MAP_TimerDisable(TIMER0_BASE, TIMER_A);
    }

    getDMAStatus = MAP_uDMAChannelModeGet(UDMA_CH16_ADC0_2 |
                                          UDMA_ALT_SELECT);

    if(getDMAStatus == UDMA_MODE_STOP)
    {
        MAP_uDMAChannelTransferSet(UDMA_CH16_ADC0_2 | UDMA_ALT_SELECT,
                                   UDMA_MODE_PINGPONG,
                                   (void *)&ADC0->SSFIFO2, (void *)&dstBufferB,
                                   sizeof(dstBufferB)/4);
        setBufBReady = true;
        MAP_TimerDisable(TIMER0_BASE, TIMER_A);
    }
}
