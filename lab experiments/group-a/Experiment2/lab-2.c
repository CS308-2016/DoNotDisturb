#include <stdint.h>
#include <stdbool.h>
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"

/* Globals */
uint8_t ui8LED = 2; // Starting with red
uint8_t sw2Status = 0;

uint8_t idleStateSwitchOne = 1; // Initial state of state machine
uint8_t pressStateSwitchOne = 0;
uint8_t releaseStateSwitchOne = 0;

uint8_t idleStateSwitchTwo = 1; // Initial state of state machine
uint8_t pressStateSwitchTwo = 0;
uint8_t releaseStateSwitchTwo = 0;

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

int main(void) {
	uint32_t ui32Period;

	SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
	switchPinConfig();

	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
	TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);

	// Configured for 10ms
	ui32Period = SysCtlClockGet()/100;
	TimerLoadSet(TIMER0_BASE, TIMER_A, ui32Period -1);

	IntEnable(INT_TIMER0A);
	TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	IntMasterEnable();

	TimerEnable(TIMER0_BASE, TIMER_A);

	while(1) {
	}
}

// This function returns one when key is pressed for the first time
uint8_t detectKeyPressSwitchOne() {
	uint32_t tempButtonStatus = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4 | GPIO_PIN_0);
	uint8_t buttonStatus = (uint8_t) tempButtonStatus;
	uint32_t sw1 = buttonStatus & 16;

	if (idleStateSwitchOne == 1) {
		if (sw1 == 0) {
			pressStateSwitchOne = 1;
			idleStateSwitchOne = 0;
		}
	}
	else if (pressStateSwitchOne == 1) {
		if (sw1 == 0) {
			releaseStateSwitchOne = 1;
			pressStateSwitchOne = 0;
			return 1;
		}
		else {
			idleStateSwitchOne = 1;
			pressStateSwitchOne = 0;
		}
	}
	else if (releaseStateSwitchOne == 1) {
		if (sw1 == 16) {
			idleStateSwitchOne = 1;
			releaseStateSwitchOne = 0;
		}
	}
	return 0;
}

// This function returns one when key is pressed for the first time
uint8_t detectKeyPressSwitchTwo() {
	uint32_t tempButtonStatus = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4 | GPIO_PIN_0);
	uint8_t buttonStatus = (uint8_t) tempButtonStatus;
	uint32_t sw2 = buttonStatus & 1;

	if (idleStateSwitchTwo == 1) {
		if (sw2 == 0) {
			pressStateSwitchTwo = 1;
			idleStateSwitchTwo = 0;
		}
	}
	else if (pressStateSwitchTwo == 1) {
		if (sw2 == 0) {
			releaseStateSwitchTwo = 1;
			pressStateSwitchTwo = 0;
			return 1;
		}
		else {
			idleStateSwitchTwo = 1;
			pressStateSwitchTwo = 0;
		}
	}
	else if (releaseStateSwitchTwo == 1) {
		if (sw2 == 1) {
			idleStateSwitchTwo = 1;
			releaseStateSwitchTwo = 0;
		}
	}
	return 0;
}

void Timer0IntHandler(void)
{
	// Clear the timer interrupt
	TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

	if (detectKeyPressSwitchOne() == 1) {
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
	}
	else {
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0);
	}

	if(detectKeyPressSwitchTwo() == 1) {
					sw2Status += 1;
	}
}
