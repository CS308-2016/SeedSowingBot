#include <stdint.h>
#include <stdbool.h>
#include "inc/tm4c123gh6pm.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/debug.h"
#include "driverlib/pwm.h"
#include "driverlib/pin_map.h"
#include "inc/hw_gpio.h"
#include "driverlib/rom.h"


#define PWM_FREQUENCY 55

#define L GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)
#define R GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)

volatile uint32_t ui32Load;
volatile uint32_t step = 1;
volatile uint32_t step2 = 100000;
volatile uint32_t step3 = 1000;

int sw1_state = 0;
int sw2_state = 0;
int sw2Status = 0;
bool tstat = 0;
int state = 0;

int lcc = 0;
int color = 1;
int count = 0;

int rstate = 0;
int lstate = 0;

bool mode1;
bool mode2;
bool mode3;

bool auto_mode;

int m_r = 254, m_g = 254, m_b = 254;

void setUp(){

	SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
	SysCtlPWMClockSet(SYSCTL_PWMDIV_64);

	SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
	HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= 0x01;
	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = 0;
	GPIODirModeSet(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0, GPIO_DIR_MODE_IN);
	GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

	volatile uint32_t ui32PWMClock;
	ui32PWMClock = SysCtlClockGet() / 64;
	ui32Load = (ui32PWMClock / PWM_FREQUENCY) - 1;


}
void redSetup(volatile uint8_t ui8Adjust){

	GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_1);
	GPIOPinConfigure(GPIO_PF1_M1PWM5);
	PWMGenConfigure(PWM1_BASE, PWM_GEN_2, PWM_GEN_MODE_DOWN);
	PWMGenPeriodSet(PWM1_BASE, PWM_GEN_2, ui32Load);

	PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, ui8Adjust * ui32Load / 1000);
	PWMOutputState(PWM1_BASE, PWM_OUT_5_BIT, true);
	PWMGenEnable(PWM1_BASE, PWM_GEN_2);
}

void blueSetup(volatile uint8_t ui8Adjust){


	GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_2);
	GPIOPinConfigure(GPIO_PF2_M1PWM6);
	PWMGenConfigure(PWM1_BASE, PWM_GEN_3, PWM_GEN_MODE_DOWN);
	PWMGenPeriodSet(PWM1_BASE, PWM_GEN_3, ui32Load);

	PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, ui8Adjust * ui32Load / 1000);
	PWMOutputState(PWM1_BASE, PWM_OUT_6_BIT, true);
	PWMGenEnable(PWM1_BASE, PWM_GEN_3);
}

void greenSetup(volatile uint8_t ui8Adjust){



	GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_3);
	GPIOPinConfigure(GPIO_PF3_M1PWM7);
	PWMGenConfigure(PWM1_BASE, PWM_GEN_3, PWM_GEN_MODE_DOWN);
	PWMGenPeriodSet(PWM1_BASE, PWM_GEN_3, ui32Load);

	PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, ui8Adjust * ui32Load / 1000);
	PWMOutputState(PWM1_BASE, PWM_OUT_7_BIT, true);
	PWMGenEnable(PWM1_BASE, PWM_GEN_3);
}

void timerConfig(){

	uint32_t ui32Period;
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
	TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
	ui32Period = (SysCtlClockGet() / 100) / 2;
	TimerLoadSet(TIMER0_BASE, TIMER_A, ui32Period -1);
	IntEnable(INT_TIMER0A);
	TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	IntMasterEnable();
	TimerEnable(TIMER0_BASE, TIMER_A);

}

bool isLClicked(void) {
	if (lstate == 0  && L == 0){
		lstate = 1;
		return false;
	} else if (lstate == 1) {
		if (L == 0) {
			lstate = 2;
			return true;
		} else {
			lstate = 0;
			return false;
		}
	} else if (L != 0) {
		lstate = 0;
	}
	if (lstate == 2)return true;
	return false;
}

bool isRClicked(void) {
	if (rstate == 0  && R == 0){
		rstate = 1;
		return false;
	} else if (rstate == 1) {
		if (R == 0) {
			rstate = 2;
			return true;
		} else {
			rstate = 0;
			return false;
		}
	} else if (R != 0) {
		rstate = 0;
	}
	if (rstate == 2)return true;
	return false;
}

void Timer0IntHandler(void)
{
	// Clear the timer interrupt
	TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);


	if(auto_mode){


		if(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)==0x00 && GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)==0x00){
			auto_mode = false;
			m_r = 254;
			m_g = 1;
			m_b = 1;
		}

		if(sw1_state == 0){
			if(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)==0x00){
				sw1_state = 1;
			}
		}
		else if(sw1_state == 1){
			if(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)==0x00){
				sw1_state = 2;
				step2-=step3;
			}
			else{
				sw1_state = 0;
			}
		}
		else {
			if(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)!=0x00){
				sw1_state = 0;
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
				step2+=step3;
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
	else{
		if (!tstat){
			if(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)!=0x00 && GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)!=0x00){
				tstat = 1;
			}
			else
				return;
		}

		if (state == 10) {
			if (!isLClicked() && !isRClicked())
				state = 0;
			return;
		}

		if (state == 0) {
			if (isLClicked() && !isRClicked()) {
				// Increase color
				if(color == 1){
					if(m_r < 254) {
						m_r++;
					}
				}
				else if(color == 2) {
					if(m_g < 254) {
						m_g++;
					}
				}
				else{
					if(m_b < 254) {
						m_b++;
					}
				}
			}

			else if (isRClicked()) {
				state = 1;
				count = 0;
				if (isLClicked())  {state = 10; color = 2; m_r = 1; m_g = 254; m_b = 1;};
			}
		} else if (state == 1 || state == 2) {
			if (state == 1 && !isRClicked()) {
				// decrease color
				if(color == 1){
					if(m_r > 1) {
						m_r--;
					}
				}
				else if(color == 2) {
					if(m_g > 1) {
						m_g--;
					}
				}
				else{
					if(m_b > 1) {
						m_b--;
					}
				}
				state = 0;
				return;
			}

			if (state == 2 && !isRClicked()) {
				if (count == 1){
					color = 1;
					m_r = 254; m_g = 1; m_b = 1;
				}
				else{
					color = 3;
					m_r = 1; m_g = 1; m_b = 254;
				}
				state = 0;
				return;
			}

			if (!lcc && isLClicked()) {
				state = 2;
				count += 1;
				lcc = 1;
			} else {
				if (!isLClicked())
					lcc = 0;
			}
		}
	}
}


void startSystem(){

	volatile uint8_t r,g,b;

	r = 254;
	g = 1;
	b = 1;

	redSetup(r);
	greenSetup(g);
	blueSetup(b);

	while(1){

		if(auto_mode){
			r = 254;
			g = 1;
			b = 1;

			// to yellow
			for(g;g<255;g+=step){
				PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, g * ui32Load / 1000);
				SysCtlDelay(step2);
			}
			// to green
			for(r;r>0;r-=step){
				PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, r * ui32Load / 1000);
				SysCtlDelay(step2);
			}
			// to cyan
			for(b;b<255;b+=step){
				PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, b * ui32Load / 1000);
				SysCtlDelay(step2);
			}
			// to blue
			for(g;g>0;g-=step){
				PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, g * ui32Load / 1000);
				SysCtlDelay(step2);
			}
			// to magenta
			for(r;r<255;r+=step){
				PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, r * ui32Load / 1000);
				SysCtlDelay(step2);
			}
			// to red
			for(b;b>0;b-=step){
				PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, b * ui32Load / 1000);
				SysCtlDelay(step2);
			}
		}
		else{
			PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, m_r * ui32Load / 1000);
			PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, m_g * ui32Load / 1000);
			PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, m_b * ui32Load / 1000);
		}
	}

}





int main(void)
{

	setUp();
	timerConfig();
	auto_mode = true;
	startSystem();
}

