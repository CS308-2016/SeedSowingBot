#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/sysctl.h"
#include "driverlib/adc.h"
#include "driverlib/debug.h"

uint32_t ui32ADC0Value[4];
volatile uint32_t ui32TempAvg;
volatile uint32_t ui32TempValueC;
volatile uint32_t ui32TempValueF;
volatile uint32_t TemperatureSet = 15;
volatile uint32_t CurrentTemparature = 0;

char Input;
bool ModeSet = false;

void printInteger(int);
void printString(char*);
void temperatureSetMode(void);

void UARTIntHandler(void)
{
	uint32_t ui32Status;
	ui32Status = UARTIntStatus(UART0_BASE, true); //get interrupt status
	UARTIntClear(UART0_BASE, ui32Status); //clear the asserted interrupts
	while(UARTCharsAvail(UART0_BASE)) //loop while there are chars
	{
		Input = UARTCharGetNonBlocking(UART0_BASE);
		UARTCharPutNonBlocking(UART0_BASE, Input); //echo character

		if(ModeSet) {
			if(Input=='\r') {
				ModeSet = false;
				UARTCharPutNonBlocking(UART0_BASE, '\n'); //echo character
				printString("Set Temperature updated to ");
				printInteger(TemperatureSet);
				printString(" *C\r\n");
			}
			else temperatureSetMode();
		}
		if(Input == 's'){
			UARTCharPutNonBlocking(UART0_BASE, '\r'); //echo character
			UARTCharPutNonBlocking(UART0_BASE, '\n'); //echo character
			printString("Enter the temperature : ");
			ModeSet = true;
			TemperatureSet = 0;
		}
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2); //blink LED
		SysCtlDelay(SysCtlClockGet() / (1000 * 3)); //delay ~1 msec
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0); //turn off LED
	}
}
void clockSetUp(void) {
	SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
}

void uartSetup(void) {

	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	GPIOPinConfigure(GPIO_PA0_U0RX);
	GPIOPinConfigure(GPIO_PA1_U0TX);
	GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

	UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200,
			(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
	IntMasterEnable(); //enable processor interrupts
	IntEnable(INT_UART0); //enable the UART interrupt
	UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT); //only enable RX and TX interrupts
}

void ledSetup(void) {
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF); //enable GPIO port for LED
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_3|GPIO_PIN_2); //enable pin for LED PF2
}

void adcSetup(void) {
	SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
	ADCHardwareOversampleConfigure(ADC0_BASE,64);
	ADCSequenceConfigure(ADC0_BASE, 1, ADC_TRIGGER_PROCESSOR, 0);
	ADCSequenceStepConfigure(ADC0_BASE, 1, 0, ADC_CTL_TS);
	ADCSequenceStepConfigure(ADC0_BASE, 1, 1, ADC_CTL_TS);
	ADCSequenceStepConfigure(ADC0_BASE, 1, 2, ADC_CTL_TS);
	ADCSequenceStepConfigure(ADC0_BASE,1,3,ADC_CTL_TS|ADC_CTL_IE|ADC_CTL_END);
	ADCSequenceEnable(ADC0_BASE, 1);
}

void printString(char* buf1) {
	int i = 0;
	while(buf1[i]!='\0') {
		UARTCharPut(UART0_BASE, buf1[i]);
		i++;
	}
}

void printInteger(int temp) {
	char buf[4];
	int i=0;
	while(temp) {
		buf[i] = (temp%10)+'0';
		temp = temp / 10;
		i++;
	}
	i--;
	while(i>=0){
		UARTCharPut(UART0_BASE,buf[i]);
		i--;
	}
}

void test1() {
	ADCIntClear(ADC0_BASE, 1);
	ADCProcessorTrigger(ADC0_BASE, 1);
	while(!ADCIntStatus(ADC0_BASE, 1, false)){}
	ADCSequenceDataGet(ADC0_BASE, 1, ui32ADC0Value);
	ui32TempAvg = (ui32ADC0Value[0] + ui32ADC0Value[1] + ui32ADC0Value[2] + ui32ADC0Value[3] + 2)/4;
	ui32TempValueC = (1475 - ((2475 * ui32TempAvg)) / 4096)/10;
	ui32TempValueF = ((ui32TempValueC * 9) + 160) / 5;
	if (CurrentTemparature != ui32TempValueC) {
		printString("Current Temperature ");
		printInteger(ui32TempValueC);
		printString("*c\r\n");
		CurrentTemparature = ui32TempValueC;
	}
}

void temperatureMonitorMode() {
	ADCIntClear(ADC0_BASE, 1);
	ADCProcessorTrigger(ADC0_BASE, 1);
	while(!ADCIntStatus(ADC0_BASE, 1, false)){}
	ADCSequenceDataGet(ADC0_BASE, 1, ui32ADC0Value);
	ui32TempAvg = (ui32ADC0Value[0] + ui32ADC0Value[1] + ui32ADC0Value[2] + ui32ADC0Value[3] + 2)/4;
	ui32TempValueC = (1475 - ((2475 * ui32TempAvg)) / 4096)/10;
	ui32TempValueF = ((ui32TempValueC * 9) + 160) / 5;
	if (CurrentTemparature != ui32TempValueC) {
		printString("Current Temp = ");
		printInteger(ui32TempValueC);
		CurrentTemparature = ui32TempValueC;
		printString(" *C , Set Temp = ");
		printInteger(TemperatureSet);
		printString(" *C\r\n");
	}
	if(CurrentTemparature < TemperatureSet) {
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_3, 8);
	}
	else {
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_3, 2);
	}
}

void temperatureSetMode() {
	if(Input>'9' || Input<'0') {
		printString("\r\nPlease provide valid Input\r\n");
		ModeSet = false;
	}
	else {
		TemperatureSet = TemperatureSet*10 + (Input-'0');
	}
}

int main(void) {
	clockSetUp();
	uartSetup();
	ledSetup();
	adcSetup();
	TemperatureSet = 16;
	while(1)
	{
		if(!ModeSet) temperatureMonitorMode();
	}
}