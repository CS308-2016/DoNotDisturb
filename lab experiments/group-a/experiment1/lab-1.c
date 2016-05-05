#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"

// LOCK_F and CR_F - used for unlocking PORTF pin 0
#define LOCK_F (*((volatile unsigned long *)0x40025520))
#define CR_F   (*((volatile unsigned long *)0x40025524))

// Global Variable Declaration
uint8_t buttonStatus;
uint8_t sw2Status;

void setup(void)
{
	SysCtlClockSet(SYSCTL_SYSDIV_4|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
}

void ledPinConfig(void)
{
	// Pins-1,2,3 of PORT F set as output. These correspond to the 3 LEDs respectively
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
}

void switchPinConfig(void)
{

	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
	HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= 0x01;
	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = 0;

	// GPIO PORTF Pin 0 and Pin4
	GPIODirModeSet(GPIO_PORTF_BASE, GPIO_PIN_4 | GPIO_PIN_0, GPIO_DIR_MODE_IN);
	GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_4 | GPIO_PIN_0);
	GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4 | GPIO_PIN_0, GPIO_STRENGTH_12MA, GPIO_PIN_TYPE_STD_WPU);
}

// return 10 denotes sw2 is pressed and sw1 is not pressed
uint8_t getButtonStatus () {
	uint32_t tempButtonStatus = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4 | GPIO_PIN_0);
	buttonStatus = (uint8_t) tempButtonStatus;
	uint32_t sw1 = buttonStatus & 16;
	uint32_t sw2 = buttonStatus & 1;
	if (sw1 == 0 && sw2 == 0) {
		return 11;
	}
	else if (sw1 == 16 && sw2 == 0) {
		return 10;
	}
	else if (sw1 == 0 && sw2 == 1) {
			return 1;
	}
	else if (sw1 == 16 && sw2 == 1){
			return 0;
	}
}

int main(void)
{
	setup();
	ledPinConfig();
	switchPinConfig();

	uint8_t buttonState;
	uint8_t button1Pressed = 0;
	uint8_t button2Pressed = 0;
	uint8_t ui8LED = 2; // Starting with red

	while(1) {
		SysCtlDelay(200000);
		buttonState = getButtonStatus();

		if (buttonState == 1 || buttonState == 11 ) {
			// Button 1 is pressed
			if (button1Pressed == 0) {
				// Button is in the pressed state for the first time
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, ui8LED);
				// Cycle through Red, Green and Blue LEDs
				if (ui8LED == 2) { // Red
					ui8LED = 8;
				}
				else if (ui8LED == 4) { // Blue
					ui8LED = 2;
				}
				else { // Green
					ui8LED = 4;
				}
				button1Pressed = 1;
			}

		}
		else {
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0);
			button1Pressed = 0;
		}

		if (buttonState == 10 || buttonState == 11 ) {
			if(button2Pressed == 0) {
				sw2Status += 1;
				button2Pressed = 1;
			}
		}
		else {
			button2Pressed = 0;
		}
	}
}
