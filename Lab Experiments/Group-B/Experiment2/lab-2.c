/*
 * main.c
 *
 *  Created on: Jan 31, 2016
 *      Author: Akshay
 */


#include <stdint.h>
#include <stdbool.h>
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"

// LOCK_F and CR_F - used for unlocking PORTF pin 0
#define LOCK_F (*((volatile unsigned long *)0x40025520))
#define CR_F   (*((volatile unsigned long *)0x40025524))

int sw1_state = 0;
int sw2_state = 0;
int sw2Status = 0;
bool flag = 0;

uint8_t vals[3]={2,8,4};
int i=0;

void switchPinConfig(void)
{
	// Following two line removes the lock from SW2 interfaced on PORTF Pin0 -- leave this unchanged
	LOCK_F=0x4C4F434BU;
	CR_F=GPIO_PIN_0|GPIO_PIN_4;

	// GPIO PORTF Pin 0 and Pin4
	GPIODirModeSet(GPIO_PORTF_BASE,GPIO_PIN_4|GPIO_PIN_0,GPIO_DIR_MODE_IN); // Set Pin-4 of PORT F as Input. Modifiy this to use another switch
	GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0);
	GPIOPadConfigSet(GPIO_PORTF_BASE,GPIO_PIN_4|GPIO_PIN_0,GPIO_STRENGTH_12MA,GPIO_PIN_TYPE_STD_WPU);
}

int main(void)
{
	uint32_t ui32Period;
	SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);

	switchPinConfig();

	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
	TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
	ui32Period = (SysCtlClockGet() / 100) / 2;
	TimerLoadSet(TIMER0_BASE, TIMER_A, ui32Period -1);
	IntEnable(INT_TIMER0A);
	TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	IntMasterEnable();
	TimerEnable(TIMER0_BASE, TIMER_A);


	while(1)
	{

	}
}
void Timer0IntHandler(void)
{
	// Clear the timer interrupt
	TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

	if(sw1_state == 0){
		if(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)==0x00){
			sw1_state = 1;
		}
	}
	else if(sw1_state == 1){
		if(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)==0x00){
			sw1_state = 2;
			flag = 1;
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, vals[i]);
		}
		else{
			sw1_state = 0;
		}
	}
	else {
		if(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)!=0x00){
			sw1_state = 0;
			flag = 0;
			i = (i+1)%3;
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3,0);
		}
	}

	if(sw2_state == 0){
		if(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)==0x00){
			sw2_state = 1;
		}
	}
	else if(sw2_state == 1){
		if(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)==0x00){
			sw2_state = 2;
			sw2Status++;
		}
		else{
			sw2_state = 0;
		}
	}
	else {
		if(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)!=0x00){
			sw2_state = 0;
		}
	}

}

