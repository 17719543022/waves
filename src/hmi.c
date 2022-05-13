/*
 * hmi.c
 *
 *  Created on: 2021��12��10��
 *      Author: JiayunLi
 */

#include <ti/devices/msp432e4/driverlib/driverlib.h>
#include "adc.h"
#include "waves.h"

void itoa(int num, char str[])
{
  int sign = num,i = 0,j = 0;
  char temp[11];
  if(sign<0)
  {
     num = -num;
  }
  do
  {
    temp[i] = num%10+'0';
    num/=10;
    i++;
  }while(num>0);
  if(sign<0)
  {
    temp[i++] = '-';
  }
  temp[i] = '\0';
  i--;
  while(i>=0)
  {
    str[j] = temp[i];
    j++;
    i--;
  }
  str[j] = '\0';
}

void ConfigureUART_HMI(uint32_t systemClock)
{
    /* Enable the clock to GPIO port A and UART 2 */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART2);

    /* Configure the GPIO Port A for UART 0 */
    MAP_GPIOPinConfigure(GPIO_PA6_U2RX);
    MAP_GPIOPinConfigure(GPIO_PA7_U2TX);
    MAP_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_6 | GPIO_PIN_7);

    /* Configure the UART for 115200 bps 8-N-1 format */
    MAP_UARTConfigSetExpClk(UART2_BASE, systemClock, 9600,
                               (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                                UART_CONFIG_PAR_NONE));
}

void UART_HMI_Send(char *ptr)
{
    volatile int i;
    while(*ptr !='\0')
    {
       MAP_UARTCharPutNonBlocking(UART2_BASE, *ptr);
       ptr++;
    }
    for(i=0;i<50000;i++);
}

//������������ʾ����
//data:��ʾ���֣�С������Ϊ4λ(�ɵ�������λ)��
//num:ͨ��,��Χ: 0-9��Ŀǰֻ��0-6��
void UASRT_HMI_Figure(float data,int num)
{
    char str[11];
    char aisle[2];
    char ending_sign[4];
    int Data=data*10000;
    if(num<10&&num>=0)
    {
      aisle[0]=num+'0';
      aisle[1]='\0';

      ending_sign[0]=0xFF;
      ending_sign[1]=0xFF;
      ending_sign[2]=0xFF;
      ending_sign[3]='\0';
      itoa(Data,str);          //����ת�ַ�
      UART_HMI_Send("x");          //HMIЭ��
      UART_HMI_Send(aisle);        //HMIͨ��
      UART_HMI_Send(".val=");      //HMIЭ��
      UART_HMI_Send(str);          //��Ч����
      UART_HMI_Send(ending_sign);  //��β��־
    }
}

void UASRT_HMI_Waveform(int data)
{
    char str[11];
    char ending_sign[4];
    ending_sign[0]=0xFF;
    ending_sign[1]=0xFF;
    ending_sign[2]=0xFF;
    ending_sign[3]='\0';
    itoa(data,str);
    UART_HMI_Send("add 1,0,");
    UART_HMI_Send(str);
    UART_HMI_Send(ending_sign);
}

//void display_hmi()
//{
//    char i;
//    uint32_t sendBuffer[20]={0};
//
//    UASRT_HMI_Figure(GetTHD(fftOutput32,setFFTmaxFreqIndex)/100.0, 0);
//    Uo1+=Uo1;
//    Uo2+=Uo2;
//    Uo3+=Uo3;
//    Uo4+=Uo4;
//    Uo5+=Uo5;
//    UASRT_HMI_Figure(Uo1/10,1);
//    UASRT_HMI_Figure(Uo2/10,2);
//    UASRT_HMI_Figure(Uo3/10,3);
//    UASRT_HMI_Figure(Uo4/10,4);
//    UASRT_HMI_Figure(Uo5/10,5);
//
//    for(i = 0;i<20;i++)
//    {
//        sendBuffer[i]=dstBufferA[i+3200]/18;
//        UASRT_HMI_Waveform((int)sendBuffer[i]);
//    }
//    MAP_uDMAChannelDisable(UDMA_CH16_ADC0_2);
//}
