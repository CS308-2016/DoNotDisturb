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

int stateNo = 0;
int stateNo2 = 0;
int sw2Status = 0;
uint8_t ui8LED = 2;//8 = green, 2 = red, 4 = blue
uint8_t shouldChange = 0;

unsigned char detectKeyPressSwitch1()
{
	int flag = 0;
	if(stateNo == 0){
		if (!GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4))//switch pressed
			{
			flag = 1;
			stateNo = 1;
			}
		else{
			stateNo = 0;
		}
		}
		else if(stateNo == 1){
			if (!GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4))//switch pressed
				{
				stateNo = 2;
				}
			else{
				stateNo = 0;
			}
			}
		else if(stateNo == 2){
			if (!GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4))//switch pressed
				{
				stateNo = 2;
				}
			else{
				stateNo = 0;
				}
			}

	return flag;
}

unsigned char detectKeyPressSwitch2()
{
	int flag = 0;
	if(stateNo2 == 0){
			if (!GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0))//switch pressed
				{
				stateNo2 = 1;
				flag = 1;
				}
			else{
				stateNo2 = 0;
			}
			}
			else if(stateNo2 == 1){
				if (!GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0))//switch pressed
					{
					stateNo2 = 2;
					}
				else{
					stateNo2 = 0;
				}
				}
			else if(stateNo2 == 2){
				if (!GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0))//switch pressed
					{
					stateNo2 = 2;
					}
				else{
					stateNo2 = 0;
					}
				}

	return flag;

}


void switchPinConfig(void)
{
	// Following two line removes the lock from SW2 interfaced on PORTF Pin0 -- leave this unchanged
	LOCK_F=0x4C4F434BU;
	CR_F=GPIO_PIN_0|GPIO_PIN_4;

	// GPIO PORTF Pin 0 and Pin4
	GPIODirModeSet(GPIO_PORTF_BASE,GPIO_PIN_4,GPIO_DIR_MODE_IN); // Set Pin-4 of PORT F as Input for SW1. Modifiy this to use another switch
	GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_4);
	GPIOPadConfigSet(GPIO_PORTF_BASE,GPIO_PIN_4,GPIO_STRENGTH_12MA,GPIO_PIN_TYPE_STD_WPU);

	GPIODirModeSet(GPIO_PORTF_BASE,GPIO_PIN_0,GPIO_DIR_MODE_IN); // Set Pin-0 of PORT F as Input for SW2. Modifiy this to use another switch
	GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_0);
	GPIOPadConfigSet(GPIO_PORTF_BASE,GPIO_PIN_0,GPIO_STRENGTH_12MA,GPIO_PIN_TYPE_STD_WPU);
}

int main(void)
{

	uint32_t ui32Period;
	SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);

	switchPinConfig();

	TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
	ui32Period = (SysCtlClockGet() / 10);
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
	// Read the current state of the GPIO pin and
	// write back the opposite state

	int flag1 = detectKeyPressSwitch1();
	int flag2 = detectKeyPressSwitch2();

	if (flag1 == 1)
					{
					    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, ui8LED);
					    if (ui8LED == 8)
							{
								ui8LED = 4;
							}
					   else if (ui8LED == 4)
							{
								ui8LED = 2;
							}
					   else if (ui8LED == 2)
							{
								ui8LED = 8;
							}
					}
   else
				   {
					    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0x00);
				   }

	if (flag2 == 1){
					   sw2Status++;
					 }
}
