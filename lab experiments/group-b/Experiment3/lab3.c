#include <stdint.h>
#include <stdbool.h>
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

int main(void)
{
	volatile uint32_t ui32Load;
	volatile uint32_t ui32PWMClock;
	volatile uint8_t ui8Adjust;
	volatile int pwmNowB=10;
	volatile int pwmNowG=10;
	volatile int pwmNowR=260;
	volatile float inc=2;

	volatile int fadeUpR=0;
	volatile int fadeUpG=1;
	volatile int fadeUpB=1;

	volatile int startR=1;
	volatile int startG=1;
	volatile int startB=0;

	volatile int autoM=1;
	volatile int man1=0;
	volatile int man2=0;
	volatile int man3=0;

	volatile int speedR=0;
	volatile int speedG=0;
	volatile int speedB=0;

	volatile int c1=0;
	volatile int c2=0;

	volatile int once=0;
	volatile int twice=0;

	SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
	SysCtlPWMClockSet(SYSCTL_PWMDIV_64);

	SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);


	GPIOPinConfigure(GPIO_PF1_M1PWM5);
	GPIOPinConfigure(GPIO_PF2_M1PWM6);
	GPIOPinConfigure(GPIO_PF3_M1PWM7);
	GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);

	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
	HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= 0x01;
	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = 0;
	GPIODirModeSet(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0, GPIO_DIR_MODE_IN);
	GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

	ui32PWMClock = SysCtlClockGet() / 64;
	ui32Load = (ui32PWMClock / PWM_FREQUENCY) - 1;
	PWMGenConfigure(PWM1_BASE, PWM_GEN_2, PWM_GEN_MODE_DOWN);
	PWMGenConfigure(PWM1_BASE, PWM_GEN_3, PWM_GEN_MODE_DOWN);

	PWMGenPeriodSet(PWM1_BASE, PWM_GEN_2, ui32Load);
	PWMGenPeriodSet(PWM1_BASE, PWM_GEN_3, ui32Load);


	PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, ui8Adjust * ui32Load / 1000);//
	PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, ui8Adjust * ui32Load / 1000);//
	PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, ui8Adjust * ui32Load / 1000);//

	PWMOutputState(PWM1_BASE, PWM_OUT_5_BIT|PWM_OUT_6_BIT|PWM_OUT_7_BIT, true);

	PWMGenEnable(PWM1_BASE, PWM_GEN_2);
	PWMGenEnable(PWM1_BASE, PWM_GEN_3);


	volatile int trueR=0;
	volatile int cap=30;
	while(1)
	{
		if(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)==0x00){
			c1++;//switch1
			trueR=1;
		}
		else{
			if(trueR==1){
				if(once==1)twice=1;
				else if(once==0) once=1;
				trueR=0;
			}

			if(c2>=cap && c1<cap && c1>0 && once==1 && twice==0){
							man1=1;man2=0;man3=0;autoM=0;
						}
			else if(c2>=cap && c1>0 && c1<30 && twice==1){
							man1=0;man2=1;man3=0;autoM=0;
						}
			else if(c1>=cap && c2>=cap){
							man1=0;man2=0;man3=1;autoM=0;
						}

			c1=0;
		}

		if(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)==0x00){
			c2++;//switch1
		}
		else{
			if(c1>=cap && c2>=cap)
			{
				man1=0;man2=0;man3=1;autoM=0;
			}

			c2=0;
			once=0;
			twice=0;
		}


		if(autoM){
			if ( fadeUpR && startR ) {
				pwmNowR += inc;
				if (pwmNowR >= 260) { fadeUpR = false; pwmNowR=254;startB=0;startG=1;}
			}
			else if(startR) {
				pwmNowR -= inc;
				if (pwmNowR <= 10) { fadeUpR = true; pwmNowR=10; startR=0;startB=1;}
			}

			if (fadeUpG && startG) {
				pwmNowG += inc;
				if (pwmNowG >= 260) { fadeUpG = false; pwmNowG=254;startR=0;startB=1;}
			}
			else if(startG) {
				pwmNowG -= inc;
				if (pwmNowG <= 10) { fadeUpG = true; pwmNowG=10;startG=0;startR=1; }
			}

			if (fadeUpB && startB) {
				pwmNowB += inc;
				if (pwmNowB >= 260) { fadeUpB = false; pwmNowB=254;startG=0;startR=1;}
			}
			else if(startB) {
				pwmNowB -= inc;
				if (pwmNowB <= 10) { fadeUpB = true; pwmNowB=10;startB=0;startG=1; }
			}


			PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5,pwmNowR*ui32Load/1000);
			PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6,pwmNowB*ui32Load/1000);
			PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7,pwmNowG*ui32Load/1000);


			if(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)==0x00)
			{
				inc = inc - 0.2;
				if (inc <= 1)
				{
					inc = 1;
				}
			}

			if(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)==0x00)
			{
				inc = inc + 0.2;
				if (ui8Adjust >= 40)
				{
					ui8Adjust = 40;
				}
			}

		}

		if(man1){
			speedR = pwmNowR;
			if(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)==0x00)
			{
				speedR--;
				if (speedR <= 10)
				{
					speedR = 10;
				}
				PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5,speedR*ui32Load/1000);
				PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6,pwmNowB*ui32Load/1000);
				PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7,pwmNowG*ui32Load/1000);
			}

			if(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)==0x00)
			{
				speedR++;
				if (speedR >= 260)
				{
					speedR = 260;
				}
				PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5,speedR*ui32Load/1000);
				PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6,pwmNowB*ui32Load/1000);
				PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7,pwmNowG*ui32Load/1000);
			}
			pwmNowR=speedR;
		}
		if(man2){
			speedB = pwmNowB;
			if(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)==0x00)
			{
				speedB--;
				if (speedB <= 10)
				{
					speedB = 10;
				}
				PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5,pwmNowR*ui32Load/1000);
				PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6,speedB*ui32Load/1000);
				PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7,pwmNowG*ui32Load/1000);
			}

			if(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)==0x00)
			{
				speedB++;
				if (speedB >= 260)
				{
					speedB = 260;
				}
				PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5,pwmNowR*ui32Load/1000);
				PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6,speedB*ui32Load/1000);
				PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7,pwmNowG*ui32Load/1000);
			}
			pwmNowB=speedB;

		}
		if(man3){
			speedG = pwmNowG;
			if(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)==0x00)
			{
				speedG--;
				if (speedG <= 10)
				{
					speedG = 10;
				}
				PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5,pwmNowR*ui32Load/1000);
				PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6,pwmNowB*ui32Load/1000);
				PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7,speedG*ui32Load/1000);
			}

			if(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)==0x00)
			{
				speedG++;
				if (speedG >= 260)
				{
					speedG = 260;
				}
				PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5,pwmNowR*ui32Load/1000);
				PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6,pwmNowB*ui32Load/1000);
				PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7,speedG*ui32Load/1000);
			}
			pwmNowG=speedG;
		}
		SysCtlDelay(200000);
	}

}

