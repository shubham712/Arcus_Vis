#include <stdbool.h>
#include <stdint.h>

#include "inc/tm4c123gh6pm.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"

#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/pin_map.h"
#include "driverlib/adc.h"
#include "driverlib/uart.h"

#include "TCD1304.h"


/* ICG 			-> PC5
 * Master Clock -> PB6
 * SH 			-> PC4	- 2.5khz
 */

void config_TCD1304(void){
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_WTIMER0);

	GPIOPinConfigure(GPIO_PB6_T0CCP0);				GPIOPinConfigure(GPIO_PC4_WT0CCP0);					GPIOPinConfigure(GPIO_PC5_WT0CCP1);
	GPIOPinTypeTimer(GPIO_PORTB_BASE, GPIO_PIN_6);	GPIOPinTypeTimer(GPIO_PORTC_BASE, GPIO_PIN_4);		GPIOPinTypeTimer(GPIO_PORTC_BASE, GPIO_PIN_5);

	TimerConfigure(TIMER0_BASE, TIMER_CFG_SPLIT_PAIR|TIMER_CFG_A_PWM);
	TimerConfigure(WTIMER0_BASE, TIMER_CFG_SPLIT_PAIR|TIMER_CFG_A_PWM|TIMER_CFG_B_PWM);
	TimerControlLevel(WTIMER0_BASE, TIMER_A, true);

	TimerLoadSet(TIMER0_BASE, TIMER_A, (SysCtlClockGet()/MASTER_CLK_FREQ)-1);
	TimerMatchSet(TIMER0_BASE, TIMER_A, ((SysCtlClockGet()/MASTER_CLK_FREQ)/2)-1);

	TimerLoadSet(WTIMER0_BASE, TIMER_A, (SysCtlClockGet()/INT_RATE)-1);
	TimerMatchSet(WTIMER0_BASE, TIMER_A, (SysCtlClockGet()/INT_RATE)-((SysCtlClockGet()/US_MULTIPLIER)*SH_PULSE_WIDTH_US-1));
	TimerLoadSet(WTIMER0_BASE, TIMER_B, (SysCtlClockGet()/FRAME_RATE)-1);
	TimerMatchSet(WTIMER0_BASE, TIMER_B, (SysCtlClockGet()/FRAME_RATE)-((SysCtlClockGet()/US_MULTIPLIER)*(SH_PULSE_WIDTH_US+ICG_PULSE_DELAY_US)-1));
}

void start_TCD1304(void){
	TimerEnable(TIMER0_BASE, TIMER_A);		//enable Master Clock
	TimerEnable(TIMER1_BASE, TIMER_A);		//enable adc timer
	TimerEnable(WTIMER0_BASE, TIMER_B);		//enable ICG
	TimerEnable(WTIMER0_BASE, TIMER_A);		//enable SH
}

void stop_TCD1304(void){
	TimerDisable(TIMER0_BASE, TIMER_A);		//disable Master Clock
	TimerDisable(TIMER1_BASE, TIMER_A);		//enable adc timer
	TimerDisable(WTIMER0_BASE, TIMER_B);	//disable ICG
	TimerDisable(WTIMER0_BASE, TIMER_A);	//disable SH
}

void init_UART(void){
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

	GPIOPinConfigure(GPIO_PA0_U0RX);
	GPIOPinConfigure(GPIO_PA1_U0TX);
	GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

	UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200,
		(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
}

void init_ADC(void){
	SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
	while(!(SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0)));
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
	GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3);

	ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);
	ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH0|ADC_CTL_IE|ADC_CTL_END);	// ts will be swapped with an adc channel
	ADCSequenceEnable(ADC0_BASE, 3);

	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
	TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC);
	TimerLoadSet(TIMER1_BASE, TIMER_A, (SysCtlClockGet()/SAMPLING_RATE)-1);

	IntEnable(INT_TIMER1A);
	TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
	IntMasterEnable();
}
