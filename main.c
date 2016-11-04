//v.4
//create a c lib for tcd1304 initializations.
//comment on the steps and write timing requirements.
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "inc/tm4c123gh6pm.h"
#include "inc/hw_memmap.h"

#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/pin_map.h"
#include "driverlib/adc.h"
#include "driverlib/uart.h"

#include "TCD1304.h"

#define CCDContentSize 4000

static uint32_t CCDContent[CCDContentSize]={0};
static bool dataReady = false;
static uint32_t n=100; 				//averaging limit
static uint8_t ui8ADC0Value[2];

void Timer1IntHandler(void){
	static uint32_t ui32ADC0Value;

//	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0xFF);
	TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
	ADCIntClear(ADC0_BASE, 1);

	static uint32_t i=0;
	static uint32_t j=0;

	ADCProcessorTrigger(ADC0_BASE, 3);
	while(!ADCIntStatus(ADC0_BASE, 3, false));
	ADCSequenceDataGet(ADC0_BASE, 3, &ui32ADC0Value);

	if(i<CCDContentSize)
		CCDContent[i]+=ui32ADC0Value;

	uint32_t arbitrary_limit=20000;
	i++;
	if(i==arbitrary_limit){
		i=0;
		j++;
		if(j==n){
			stop_TCD1304();
			j=0;
			dataReady = true;
		}
	}
//	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0x00);

//	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0xFF);
//	ADCProcessorTrigger(ADC0_BASE, 3);
//	while(!ADCIntStatus(ADC0_BASE, 3, false));
//
//	ADCSequenceDataGet(ADC0_BASE, 3, &ui32ADC0Value);
//	ui8ADC0Value[1] = ui32ADC0Value;
//	ui8ADC0Value[0] = (ui32ADC0Value >> 8);*/
//
//	data_sent++;
//	while(UARTBusy(UART0_BASE));
//	UARTCharPutNonBlocking(UART0_BASE, data_sent);
//	UARTCharPutNonBlocking(UART0_BASE, '*');
//
//
//	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0x00);
//
//	UARTCharPut(UART0_BASE, ui8ADC0Value[0]);//high byte should be sent first.
//	UARTCharPut(UART0_BASE, ui8ADC0Value[1]);
//
//
//	if(data_sent == 500){
//		UARTCharPut(UART0_BASE, '*');
//		UARTCharPut(UART0_BASE, 's');
//		UARTCharPut(UART0_BASE, 'p');
//		UARTCharPut(UART0_BASE, 'e');
//		UARTCharPut(UART0_BASE, 'c');
//		UARTCharPut(UART0_BASE, 't');
//		UARTCharPut(UART0_BASE, 'r');
//		UARTCharPut(UART0_BASE, 'a');
//		data_sent = 0;
//	}
}

void main(void) {

	/*Setting the Main Clock to 40Mhz, by dividing 400 MHz PLL*/
	SysCtlClockSet(SYSCTL_SYSDIV_2_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);

	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1);


	config_TCD1304();
	init_ADC();
	init_UART();
	//start_TCD1304();

	//trial section
	uint32_t a=0,b=0,c=0;
	a=SysCtlClockGet()/US_MULTIPLIER;
	b=a*SH_PULSE_WIDTH_US;
	c=(SysCtlClockGet()/US_MULTIPLIER)*SH_PULSE_WIDTH_US;
	//trial section ends

	start_TCD1304();
	uint32_t i;
	while(1){
		/*if(UARTCharsAvail(UART0_BASE)){
			UARTCharGet(UART0_BASE);
			start_TCD1304();
		}*/
		if(dataReady){
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0xFF);
			UARTCharPut(UART0_BASE, '*');
			UARTCharPut(UART0_BASE, 's');
			UARTCharPut(UART0_BASE, 'p');
			UARTCharPut(UART0_BASE, 'e');
			UARTCharPut(UART0_BASE, 'c');
			UARTCharPut(UART0_BASE, 't');
			UARTCharPut(UART0_BASE, 'r');
			UARTCharPut(UART0_BASE, 'a');
			for(i=0;i<CCDContentSize;i++){
				CCDContent[i] = CCDContent[i]/n;
				ui8ADC0Value[1] = CCDContent[i];
				ui8ADC0Value[0] = (CCDContent[i] >> 8);
				UARTCharPut(UART0_BASE, ui8ADC0Value[0]);		//high byte should be sent first.
				UARTCharPut(UART0_BASE, ui8ADC0Value[1]);
			}
			memset(CCDContent, 0, CCDContentSize*4);	//factor 4 is for uint32_t
			dataReady=false;
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0x00);
			start_TCD1304();
		}
	}
}
