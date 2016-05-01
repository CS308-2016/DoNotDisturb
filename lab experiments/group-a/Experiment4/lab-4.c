#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/sysctl.h"
#include "driverlib/adc.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/uart.h"

uint32_t ui32ADC0Value[4];
uint32_t ui32TempAvg;
uint32_t ui32TempValueC;
int setTemp = 25;

// 0: Monitor mode & 1: Set mode
uint8_t mode = 0;

char firstString[15] = "Current Temp = ";
char secondString[15] = "C , Set Temp = ";

char inputString[26] = "Enter the temperature : \n\r";
char updatedString[28] = "Set Temperature updated to ";

void configADC() {
	SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);

	ADCSequenceConfigure(ADC0_BASE, 1, ADC_TRIGGER_PROCESSOR, 0);
	ADCSequenceStepConfigure(ADC0_BASE, 1, 0, ADC_CTL_TS);
	ADCSequenceStepConfigure(ADC0_BASE, 1, 1, ADC_CTL_TS);
	ADCSequenceStepConfigure(ADC0_BASE, 1, 2, ADC_CTL_TS);
	ADCSequenceStepConfigure(ADC0_BASE,1,3,ADC_CTL_TS|ADC_CTL_IE|ADC_CTL_END);
	ADCSequenceEnable(ADC0_BASE, 1);
}

void configUART() {
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

	GPIOPinConfigure(GPIO_PA0_U0RX);
	GPIOPinConfigure(GPIO_PA1_U0TX);
	GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

	UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
}

void configLED() {
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
}

void setColor() {
	uint8_t color = 2;
	if ( ui32TempValueC < setTemp) color = 8;
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, color);
}

void printMonitorMode() {

	int i;
	for (i = 0 ; i < 15; i++) {
		UARTCharPut(UART0_BASE, firstString[i]);
	}

	char digit1 = ui32TempValueC%10 + '0';
	char digit2 =  ui32TempValueC/10 + '0';
	char degree1 = 194;
	char degree2 = 176;
	UARTCharPut(UART0_BASE, digit2);
	UARTCharPut(UART0_BASE, digit1);
	UARTCharPut(UART0_BASE, degree1);
	UARTCharPut(UART0_BASE, degree2);

	for (i = 0 ; i < 15; i++) {
			UARTCharPut(UART0_BASE, secondString[i]);
	}

	char digit3;
	digit1 = setTemp%10 + '0';
	digit2 =  setTemp/10 + '0';
	digit3 = setTemp/100 + '0';
	UARTCharPut(UART0_BASE, digit3);
	UARTCharPut(UART0_BASE, digit2);
	UARTCharPut(UART0_BASE, digit1);
	UARTCharPut(UART0_BASE, degree1);
	UARTCharPut(UART0_BASE, degree2);
	UARTCharPut(UART0_BASE, 'C');
	UARTCharPut(UART0_BASE, '\n');
	UARTCharPut(UART0_BASE, '\r');
}

int main(void) {

	SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
	configADC();
	configUART();
	configLED();

	while(1)
	{
		if (mode == 0) {
			ADCIntClear(ADC0_BASE, 1);
			ADCProcessorTrigger(ADC0_BASE, 1);

			while(!ADCIntStatus(ADC0_BASE, 1, false)) {
			}
			ADCSequenceDataGet(ADC0_BASE, 1, ui32ADC0Value);
			ui32TempAvg = (ui32ADC0Value[0] + ui32ADC0Value[1] + ui32ADC0Value[2] + ui32ADC0Value[3] + 2)/4;
			ui32TempValueC = (1475 - ((2475 * ui32TempAvg)) / 4096)/10;
			printMonitorMode();
			setColor();

			SysCtlDelay(SysCtlClockGet() / 3); // Delay of ~ 1 sec
		}

		if (UARTCharsAvail(UART0_BASE)) {
			char input = UARTCharGet(UART0_BASE);
			if (input == 's') {
				int i;
				for (i = 0; i < 26; i++) {
					UARTCharPut(UART0_BASE, inputString[i]);
				}
				mode = 1;
				setTemp = 0;
			}
			else if (input == '\r') {
				int i;
				for (i = 0; i < 28; i++) {
					UARTCharPut(UART0_BASE, updatedString[i]);
				}
				char digit1 = setTemp%10 + '0';
				char digit2 =  setTemp/10 + '0';
				char digit3 =  setTemp/100 + '0';
				UARTCharPut(UART0_BASE, digit3);
				UARTCharPut(UART0_BASE, digit2);
				UARTCharPut(UART0_BASE, digit1);
				UARTCharPut(UART0_BASE, 194);
				UARTCharPut(UART0_BASE, 176);
				UARTCharPut(UART0_BASE, 'C');
				UARTCharPut(UART0_BASE, '\n');
				UARTCharPut(UART0_BASE, '\r');
				mode = 0;
			}
			else if (mode == 1) {
				setTemp = setTemp*10 + input - '0';
			}
		}
	}
}
