#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/debug.h"
#include "driverlib/adc.h"
#include "inc/hw_types.h"
#define TARGET_IS_BLIZZARD_RB1
#include "driverlib/rom.h"

#define LOCK_F (*((volatile unsigned long *)0x40025520))
#define CR_F   (*((volatile unsigned long *)0x40025524))

void ledPinConfig(void)
{
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);  // Pin-1 of PORT F set as output. Modifiy this to use other 2 LEDs.
}

int main(void) {
	SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);


	ledPinConfig();

	GPIOPinConfigure(GPIO_PA0_U0RX);
	GPIOPinConfigure(GPIO_PA1_U0TX);
	GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
	UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200,
			(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));

	uint32_t ui32ADC0Value[4];
	volatile uint32_t ui32TempAvg;
	volatile uint32_t ui32TempValueC;
	volatile uint32_t ui32TempValueF;
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
	ROM_ADCHardwareOversampleConfigure(ADC0_BASE, 64);
	ROM_ADCSequenceConfigure(ADC0_BASE, 1, ADC_TRIGGER_PROCESSOR, 0);
	ROM_ADCSequenceStepConfigure(ADC0_BASE, 1, 0, ADC_CTL_TS);
	ROM_ADCSequenceStepConfigure(ADC0_BASE, 1, 1, ADC_CTL_TS);
	ROM_ADCSequenceStepConfigure(ADC0_BASE, 1, 2, ADC_CTL_TS);
	ROM_ADCSequenceStepConfigure(ADC0_BASE,1,3,ADC_CTL_TS|ADC_CTL_IE|ADC_CTL_END);
	ROM_ADCSequenceEnable(ADC0_BASE, 1);

	int monTemp = 25;
	int mode = 0;//0 = Monitor 1 = Set

	while (1)
	{
		ROM_ADCIntClear(ADC0_BASE, 1);
		ROM_ADCProcessorTrigger(ADC0_BASE, 1);

		if (UARTCharsAvail(UART0_BASE)){
			char check = UARTCharGet(UART0_BASE);
			if(check == 's' || check == 'S'){
				UARTCharPut(UART0_BASE, 'E');
				UARTCharPut(UART0_BASE, 'n');
				UARTCharPut(UART0_BASE, 't');
				UARTCharPut(UART0_BASE, 'e');
				UARTCharPut(UART0_BASE, 'r');
				UARTCharPut(UART0_BASE, ' ');
				UARTCharPut(UART0_BASE, 't');
				UARTCharPut(UART0_BASE, 'h');
				UARTCharPut(UART0_BASE, 'e');
				UARTCharPut(UART0_BASE, ' ');
				UARTCharPut(UART0_BASE, 'T');
				UARTCharPut(UART0_BASE, 'e');
				UARTCharPut(UART0_BASE, 'm');
				UARTCharPut(UART0_BASE, 'p');
				UARTCharPut(UART0_BASE, 'e');
				UARTCharPut(UART0_BASE, 'r');
				UARTCharPut(UART0_BASE, 'a');
				UARTCharPut(UART0_BASE, 't');
				UARTCharPut(UART0_BASE, 'u');
				UARTCharPut(UART0_BASE, 'r');
				UARTCharPut(UART0_BASE, 'e');
				UARTCharPut(UART0_BASE, ' ');
				UARTCharPut(UART0_BASE, ':');
				UARTCharPut(UART0_BASE, ' ');

				char input =  UARTCharGet(UART0_BASE);
				monTemp = 0;
				while(input != '\r')
				{
					UARTCharPut(UART0_BASE, input);
					monTemp = monTemp*10 + (int)(input-'0');
					input =  UARTCharGet(UART0_BASE);
				}
				UARTCharPut(UART0_BASE, '\r');
				UARTCharPut(UART0_BASE, '\n');
				UARTCharPut(UART0_BASE, 'S');
				UARTCharPut(UART0_BASE, 'e');
				UARTCharPut(UART0_BASE, 't');
				UARTCharPut(UART0_BASE, ' ');
				UARTCharPut(UART0_BASE, 'T');
				UARTCharPut(UART0_BASE, 'e');
				UARTCharPut(UART0_BASE, 'm');
				UARTCharPut(UART0_BASE, 'p');
				UARTCharPut(UART0_BASE, 'e');
				UARTCharPut(UART0_BASE, 'r');
				UARTCharPut(UART0_BASE, 'a');
				UARTCharPut(UART0_BASE, 't');
				UARTCharPut(UART0_BASE, 'u');
				UARTCharPut(UART0_BASE, 'r');
				UARTCharPut(UART0_BASE, 'e');
				UARTCharPut(UART0_BASE, ' ');
				UARTCharPut(UART0_BASE, 'u');
				UARTCharPut(UART0_BASE, 'p');
				UARTCharPut(UART0_BASE, 'd');
				UARTCharPut(UART0_BASE, 'a');
				UARTCharPut(UART0_BASE, 't');
				UARTCharPut(UART0_BASE, 'e');
				UARTCharPut(UART0_BASE, 'd');
				UARTCharPut(UART0_BASE, ' ');
				UARTCharPut(UART0_BASE, 't');
				UARTCharPut(UART0_BASE, 'o');
				UARTCharPut(UART0_BASE, ' ');


				char ans2[4] = {' ',' ',' ','\0'};
				int i = 0;
				int temporaryT = monTemp;
				char ch = ' ';
				while(temporaryT!=0){
					ch = (char)(temporaryT%10 + '0');
					ans2[i]=ch;
					i++;
					temporaryT/=10;
				}

				for(i=strlen(ans2)-1;i>=0;i--)
				{
					UARTCharPut(UART0_BASE, ans2[i]);
				}

				UARTCharPut(UART0_BASE, ' ');
				UARTCharPut(UART0_BASE, 'C');
				UARTCharPut(UART0_BASE, '\r');
				UARTCharPut(UART0_BASE, '\n');

			}

		}

		if(mode==0){

			while(!ROM_ADCIntStatus(ADC0_BASE, 1, false))
			{
			}
			ROM_ADCSequenceDataGet(ADC0_BASE, 1, ui32ADC0Value);
			ui32TempAvg = (ui32ADC0Value[0] + ui32ADC0Value[1] + ui32ADC0Value[2] + ui32ADC0Value[3] + 2)/4;
			ui32TempValueC = (1475 - ((2475 * ui32TempAvg)) / 4096)/10;
			ui32TempValueF = ((ui32TempValueC * 9) + 160) / 5;

			if( ui32TempValueC > monTemp )
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0x02);
			else
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0x08);

			UARTCharPut(UART0_BASE, 'C');
			UARTCharPut(UART0_BASE, 'u');
			UARTCharPut(UART0_BASE, 'r');
			UARTCharPut(UART0_BASE, 'r');
			UARTCharPut(UART0_BASE, 'e');
			UARTCharPut(UART0_BASE, 'n');
			UARTCharPut(UART0_BASE, 't');
			UARTCharPut(UART0_BASE, ' ');
			UARTCharPut(UART0_BASE, 'T');
			UARTCharPut(UART0_BASE, 'e');
			UARTCharPut(UART0_BASE, 'm');
			UARTCharPut(UART0_BASE, 'p');
			UARTCharPut(UART0_BASE, '=');
			UARTCharPut(UART0_BASE, ' ');

			char ch = ' ';
			char ans[4] = {' ',' ',' ','\0'};
			int i = 0;
			int temporaryT = ui32TempValueC;
			while(temporaryT!=0){
				ch = (char)(temporaryT%10 + '0');
				ans[i]=ch;
				i++;
				temporaryT/=10;
			}

			for(i=strlen(ans)-1;i>=0;i--)
			{
				UARTCharPut(UART0_BASE, ans[i]);
			}

			UARTCharPut(UART0_BASE, ' ');
			UARTCharPut(UART0_BASE, 'C');
			UARTCharPut(UART0_BASE, ',');
			UARTCharPut(UART0_BASE, ' ');


			UARTCharPut(UART0_BASE, 'S');
			UARTCharPut(UART0_BASE, 'e');
			UARTCharPut(UART0_BASE, 't');
			UARTCharPut(UART0_BASE, ' ');
			UARTCharPut(UART0_BASE, 'T');
			UARTCharPut(UART0_BASE, 'e');
			UARTCharPut(UART0_BASE, 'm');
			UARTCharPut(UART0_BASE, 'p');
			UARTCharPut(UART0_BASE, '=');
			UARTCharPut(UART0_BASE, ' ');

			char ans2[4] = {' ',' ',' ','\0'};
			i = 0;
			temporaryT = monTemp;
			while(temporaryT!=0){
				ch = (char)(temporaryT%10 + '0');
				ans2[i]=ch;
				i++;
				temporaryT/=10;
			}

			for(i=strlen(ans2)-1;i>=0;i--)
			{
				UARTCharPut(UART0_BASE, ans2[i]);
			}

			UARTCharPut(UART0_BASE, ' ');
			UARTCharPut(UART0_BASE, 'C');
			UARTCharPut(UART0_BASE, '\r');
			UARTCharPut(UART0_BASE, '\n');
		}

		if(mode == 1){
			//while(UARTCharsAvail(UART0_BASE)) UARTCharPut(UART0_BASE, UARTCharGet(UART0_BASE));
		}

		//ui32TempValueC = 132;

		SysCtlDelay(16000000u/3u);
	}
}



