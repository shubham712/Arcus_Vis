/*
 * TCD1304.h
 *
 *  Last update: 31.10.2016
 *  Author: Mehmet Onur Cirit
 */

/*TCD1304 is a highly sensitive CCD with low dark current From TOSHIBA.
 *	Features -> 3648 pixels
 *			 -> 8µm x 200µm pixel size
 */


#ifndef TCD1304_H_
#define TCD1304_H_

//#pragma once  //---->>> This is a nice replacement to header guards. But it is compiler dependent.

#define MASTER_CLK_FREQ 	800000
#define FRAME_RATE 			10				//bu sayýlarýn sysctlclockgate'e tam bölünebilmesi gerekiyor, yoksa faz kaymasý olmaya baþlýyor. Aklýnda bulunsun, araþtýr.***
#define INT_RATE			10000//2400
#define ICG_PULSE_DELAY_US	5
#define SH_PULSE_WIDTH_US	5
#define SAMPLING_RATE 		200000
#define US_MULTIPLIER		1000000

/* function prototypes */

void config_TCD1304(void);
void start_TCD1304(void);
void stop_TCD1304(void);
void init_ADC(void);
void init_UART(void);


#endif /* TCD1304_H_ */
