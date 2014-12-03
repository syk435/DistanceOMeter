//------------------------------------------------

#include <stdio.h>
#include <stdint.h>
#include "inc/lm3s811.h"
#include "Systick.h"

//------------------------------------------------

#define PC7  (*((volatile unsigned long *)0x40006200))
#define PC6  (*((volatile unsigned long *)0x40006100))
#define PC5  (*((volatile unsigned long *)0x40006080))

//------------------------------------------------

void Switch_Init(void){
  volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x00000004;  // 1) activate clock for Port C
  delay = SYSCTL_RCGC2_R;        // allow time for clock to settle
  GPIO_PORTC_DIR_R &= ~0x70;     // 2) set direction register
  GPIO_PORTC_AFSEL_R &= ~0x70;   // 3) regular port function
  GPIO_PORTC_DEN_R |= 0x70;      // 4) enable digital port
}

//------------------------------------------------

int Poll_PC5(void) 
{
	if(PC5 > 0)
	{
		SysTick_Wait(60000);
		return PC5;
	}
	else
	{
		return 0;
	}
}
int Poll_PC6(void) 
{
	if(PC6 > 0)
	{
		SysTick_Wait(60000);
		return PC6;
	}
	else
	{
		return 0;
	}
}
int Poll_PC7(void) 
{
	if(PC7 > 0)
	{
		SysTick_Wait(60000);
		return PC7;
	}
	else
	{
		return 0;
	}
}
