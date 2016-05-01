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
#include "driverlib/debug.h"
#include "driverlib/pwm.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"

#define PWM_FREQUENCY 55

/* Globals */

uint8_t idleStateSwitchOne = 1; // Initial state of state machine
uint8_t pressStateSwitchOne = 0;
uint8_t releaseStateSwitchOne = 0;

uint8_t idleStateSwitchTwo = 1; // Initial state of state machine
uint8_t pressStateSwitchTwo = 0;
uint8_t releaseStateSwitchTwo = 0;

/* PWM Globals */
volatile uint32_t ui32Load;
volatile uint32_t ui32PWMClock;
volatile uint8_t ui8AdjustRed;
volatile uint8_t ui8AdjustGreen;
volatile uint8_t ui8AdjustBlue;

/* Auto Mode Globals */
float angle = 0;
float delta = 1;
float step = 0.1;
float factor = 2.117;

/* Manual Mode Globals */
int mode = -1;
uint8_t upperCap = 20;
float sw1Count = 0;
float sw2Count = 0;

void pwmConfig() {

	// Initial color is red
	ui8AdjustRed = 255;
	ui8AdjustGreen = 1;
	ui8AdjustBlue = 1;

	SysCtlPWMClockSet(SYSCTL_PWMDIV_64);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);

	GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
	GPIOPinConfigure(GPIO_PF1_M1PWM5);
	GPIOPinConfigure(GPIO_PF2_M1PWM6);
	GPIOPinConfigure(GPIO_PF3_M1PWM7);

	ui32PWMClock = SysCtlClockGet() / 64;
	ui32Load = (ui32PWMClock / PWM_FREQUENCY) - 1;

	PWMGenConfigure(PWM1_BASE, PWM_GEN_2, PWM_GEN_MODE_DOWN);
	PWMGenConfigure(PWM1_BASE, PWM_GEN_3, PWM_GEN_MODE_DOWN);
	PWMGenPeriodSet(PWM1_BASE, PWM_GEN_2, ui32Load);
	PWMGenPeriodSet(PWM1_BASE, PWM_GEN_3, ui32Load);

	PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, ui8AdjustRed * ui32Load / 1000);
	PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, ui8AdjustBlue * ui32Load / 1000);
	PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, ui8AdjustGreen * ui32Load / 1000);

	PWMOutputState(PWM1_BASE, PWM_OUT_5_BIT | PWM_OUT_6_BIT | PWM_OUT_7_BIT, true);
	PWMGenEnable(PWM1_BASE, PWM_GEN_2);
	PWMGenEnable(PWM1_BASE, PWM_GEN_3);
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

int main(void) {
	uint32_t ui32Period;

	SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

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

	pwmConfig();

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

void computeAndSetColors() {

	angle += delta;
	angle = ((int)angle)%360;

	ui8AdjustRed = 1;
	ui8AdjustGreen = 1;
	ui8AdjustBlue = 1;

	/* Red */
	if (angle <= 120)  {
		ui8AdjustRed = (120 - angle)*factor + 1;
	}
	else if (angle >= 240) {
		ui8AdjustRed = (angle - 240)*factor + 1;
	}

	/* Green */
	if (angle <= 120)  {
		ui8AdjustGreen = (angle)*factor + 1;
	}
	else if (angle <= 240) {
		ui8AdjustGreen = (240-angle)*factor + 1;
	}

	/* Blue */
	if (angle >= 240)  {
		ui8AdjustBlue = (360 - angle)*factor + 1;
	}
	else if (angle >= 120) {
		ui8AdjustBlue = (angle - 120)*factor + 1;
	}

	PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, ui8AdjustRed * ui32Load / 1000);
	PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, ui8AdjustBlue * ui32Load / 1000);
	PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, ui8AdjustGreen * ui32Load / 1000);

}

void stateCheck() {
	if(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)==0x10 && sw1Count > 0) {
		if (sw1Count > upperCap && sw2Count > upperCap ) {
			mode = 3;
			ui8AdjustRed = 1;
			ui8AdjustBlue = 1;
			ui8AdjustGreen = 1;
			sw2Count = 0;
		}
		else if (sw1Count < upperCap && sw2Count > upperCap) {
			mode = 1;
			sw2Count = 0;
			ui8AdjustRed = 1;
						ui8AdjustBlue = 1;
						ui8AdjustGreen = 1;
		}
		sw1Count = 0;
	}

	else if(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)==0x01 && sw2Count > 0) {
			if (sw1Count > upperCap && sw2Count > upperCap ) {
				mode = 3;
				sw1Count = 0;
				ui8AdjustRed = 1;
							ui8AdjustBlue = 1;
							ui8AdjustGreen = 1;
			}
			else if (sw2Count < upperCap && sw1Count > upperCap) {
				mode = 2;
				sw1Count = 0;
				ui8AdjustRed = 1;
							ui8AdjustBlue = 1;
							ui8AdjustGreen = 1;
			}
			sw2Count = 0;
	}
	return;
}

void Timer0IntHandler(void)
{
	// Clear the timer interrupt
	TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

	if (mode == -1) {
		if (GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)==0x00 && GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)==0x00){
			mode=0;
			ui8AdjustRed = 1;
			ui8AdjustGreen = 1;
			ui8AdjustBlue = 1;
			return;
		}

		else if(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)==0x00)
		{
			delta = delta - step;
			if (delta < 1) delta = 1;
		}

		else if(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)==0x00)
		{
			delta = delta + step;
			if (delta > 30) delta = 30;
		}
		computeAndSetColors();
	}

	// State check
	stateCheck();

	// Button increment
	if (GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)==0x00) sw2Count += step;
	if (GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)==0x00) sw1Count += step;

	if(mode == 1) {
		if(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)==0x00)
		{
			ui8AdjustRed--;
			if (ui8AdjustRed < 1)
			{
				ui8AdjustRed = 1;
			}

		}
		else if(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)==0x00)
		{
			ui8AdjustRed++;
			if (ui8AdjustRed > 254)
			{
				ui8AdjustRed = 254;
			}

		}
	}
	else if(mode == 2) {
			if(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)==0x00)
			{
				ui8AdjustBlue--;
				if (ui8AdjustBlue < 1)
				{
					ui8AdjustBlue = 1;
				}

			}
			else if(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)==0x00)
			{
				ui8AdjustBlue++;
				if (ui8AdjustBlue > 254)
				{
					ui8AdjustBlue = 254;
				}

			}
		}
	else if(mode == 3) {
			if(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)==0x00)
			{
				ui8AdjustGreen--;
				if (ui8AdjustGreen < 1)
				{
					ui8AdjustGreen = 1;
				}

			}
			else if(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)==0x00)
			{
				ui8AdjustGreen++;
				if (ui8AdjustGreen > 254)
				{
					ui8AdjustGreen = 254;
				}

			}
		}
	PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, ui8AdjustRed * ui32Load / 1000);
	PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, ui8AdjustBlue * ui32Load / 1000);
	PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, ui8AdjustGreen * ui32Load / 1000);
}
