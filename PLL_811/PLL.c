// PLL.c
// Runs on LM3S811
// A software function to change the bus speed using the PLL.
// Commented lines in the function PLL_Init() initialize the PWM
// to either 25 MHz or 50 MHz.  When using an oscilloscope to
// look at LED0, it should be clear to see that the LED flashes
// about 2 (50/25) times faster with a 50 MHz clock than with a
// 25 MHz clock.
// Daniel Valvano
// July 13, 2011

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to the Arm Cortex M3",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2011
   Program 2.10, Figure 2.31

 Copyright 2011 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */

#define GPIO_PORTC_DIR_R        (*((volatile unsigned long *)0x40006400))
#define GPIO_PORTC_AFSEL_R      (*((volatile unsigned long *)0x40006420))
#define GPIO_PORTC_DEN_R        (*((volatile unsigned long *)0x4000651C))
#define SYSCTL_RIS_R            (*((volatile unsigned long *)0x400FE050))
#define SYSCTL_RIS_PLLLRIS      0x00000040  // PLL Lock Raw Interrupt Status
#define SYSCTL_RCC_R            (*((volatile unsigned long *)0x400FE060))
#define SYSCTL_RCC_SYSDIV_M     0x07800000  // System Clock Divisor
#define SYSCTL_RCC_SYSDIV_4     0x01800000  // System clock /4
#define SYSCTL_RCC_SYSDIV_5     0x02000000  // System clock /5
#define SYSCTL_RCC_SYSDIV_6     0x02800000  // System clock /6
#define SYSCTL_RCC_SYSDIV_7     0x03000000  // System clock /7
#define SYSCTL_RCC_SYSDIV_8     0x03800000  // System clock /8
#define SYSCTL_RCC_SYSDIV_9     0x04000000  // System clock /9
#define SYSCTL_RCC_SYSDIV_10    0x04800000  // System clock /10
#define SYSCTL_RCC_SYSDIV_11    0x05000000  // System clock /11
#define SYSCTL_RCC_SYSDIV_12    0x05800000  // System clock /12
#define SYSCTL_RCC_SYSDIV_13    0x06000000  // System clock /13
#define SYSCTL_RCC_SYSDIV_14    0x06800000  // System clock /14
#define SYSCTL_RCC_SYSDIV_15    0x07000000  // System clock /15
#define SYSCTL_RCC_SYSDIV_16    0x07800000  // System clock /16
#define SYSCTL_RCC_USESYSDIV    0x00400000  // Enable System Clock Divider
#define SYSCTL_RCC_PWRDN        0x00002000  // PLL Power Down
#define SYSCTL_RCC_OEN          0x00001000  // PLL Output Enable
#define SYSCTL_RCC_BYPASS       0x00000800  // PLL Bypass
#define SYSCTL_RCC_XTAL_M       0x000003C0  // Crystal Value
#define SYSCTL_RCC_XTAL_6MHZ    0x000002C0  // 6 MHz Crystal
#define SYSCTL_RCC_XTAL_8MHZ    0x00000380  // 8 MHz Crystal
#define SYSCTL_RCC_OSCSRC_M     0x00000030  // Oscillator Source
#define SYSCTL_RCC_OSCSRC_MAIN  0x00000000  // MOSC
#define SYSCTL_RCGC2_R          (*((volatile unsigned long *)0x400FE108))
#define SYSCTL_RCGC2_GPIOC      0x00000004  // port C Clock Gating Control
#define GPIO_PORTC5             (*((volatile unsigned long *)0x40006080))
// access PC5

// configure the system to get its clock from the PLL
void PLL_Init(void){
  // 1) bypass PLL and system clock divider while initializing
  SYSCTL_RCC_R |= SYSCTL_RCC_BYPASS;
  SYSCTL_RCC_R &= ~SYSCTL_RCC_USESYSDIV;
  // 2) select the crystal value and oscillator source
  SYSCTL_RCC_R &= ~SYSCTL_RCC_XTAL_M;   // clear XTAL field
  SYSCTL_RCC_R += SYSCTL_RCC_XTAL_6MHZ; // configure for 6 MHz crystal (default setting)
  SYSCTL_RCC_R &= ~SYSCTL_RCC_OSCSRC_M; // clear oscillator source field
  SYSCTL_RCC_R += SYSCTL_RCC_OSCSRC_MAIN;// configure for main oscillator source (default setting)
  // 3) activate PLL by clearing PWRDN and OEN
  SYSCTL_RCC_R &= ~(SYSCTL_RCC_PWRDN|SYSCTL_RCC_OEN);
  // 4) set the desired system divider and the USESYSDIV bit
  SYSCTL_RCC_R &= ~SYSCTL_RCC_SYSDIV_M; // system clock divider field
//  SYSCTL_RCC_R += SYSCTL_RCC_SYSDIV_4;  // configure for 50 MHz clock
//  SYSCTL_RCC_R += SYSCTL_RCC_SYSDIV_5;  // configure for 40 MHz clock
//  SYSCTL_RCC_R += SYSCTL_RCC_SYSDIV_6;  // configure for 33.33 MHz clock
//  SYSCTL_RCC_R += SYSCTL_RCC_SYSDIV_7;  // configure for 28.57 MHz clock
  SYSCTL_RCC_R += SYSCTL_RCC_SYSDIV_8;  // configure for 25 MHz clock
//  SYSCTL_RCC_R += SYSCTL_RCC_SYSDIV_9;  // configure for 22.22 MHz clock
//  SYSCTL_RCC_R += SYSCTL_RCC_SYSDIV_10; // configure for 20 MHz clock
//  SYSCTL_RCC_R += SYSCTL_RCC_SYSDIV_11; // configure for 18.18 MHz clock
//  SYSCTL_RCC_R += SYSCTL_RCC_SYSDIV_12; // configure for 16.67 MHz clock
//  SYSCTL_RCC_R += SYSCTL_RCC_SYSDIV_13; // configure for 15.38 MHz clock
//  SYSCTL_RCC_R += SYSCTL_RCC_SYSDIV_14; // configure for 14.29 MHz clock
//  SYSCTL_RCC_R += SYSCTL_RCC_SYSDIV_15; // configure for 13.33 MHz clock
//  SYSCTL_RCC_R += SYSCTL_RCC_SYSDIV_16; // configure for 12.5 MHz clock (default setting)
  SYSCTL_RCC_R |= SYSCTL_RCC_USESYSDIV;
  // 5) wait for the PLL to lock by polling PLLLRIS
  while((SYSCTL_RIS_R&SYSCTL_RIS_PLLLRIS)==0){};
  // 6) enable use of PLL by clearing BYPASS
  SYSCTL_RCC_R &= ~SYSCTL_RCC_BYPASS;
}

//debug code
// delay function for testing from sysctl.c
// which delays 3*ulCount cycles
#ifdef __TI_COMPILER_VERSION__
	//Code Composer Studio Code
	void Delay(unsigned long ulCount){
	__asm (	"    subs    r0, #1\n"
			"    bne     Delay\n"
			"    bx      lr\n");
}

#else
	//Keil uVision Code
	__asm void
	Delay(unsigned long ulCount)
	{
    subs    r0, #1
    bne     Delay
    bx      lr
	}

#endif
int main2(void){  volatile unsigned long delay;
  PLL_Init();
  SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOC; // activate port C
  delay = SYSCTL_RCGC2_R;      // allow time for clock to stabilize
  GPIO_PORTC_DIR_R |= 0x20;    // make PC5 out (PC5 built-in LED)
  GPIO_PORTC_AFSEL_R &= ~0x20; // regular port function (default setting)
  GPIO_PORTC_DEN_R |= 0x20;    // enable digital I/O on PC5 (default setting)
  while(1){
    GPIO_PORTC5 = 0x20;        // turn on LED0
    Delay(16666667);           // delay ~1 sec at 50 MHz
    GPIO_PORTC5 = 0x00;        // turn off LED0
    Delay(16666667);           // delay ~1 sec at 50 MHz
  }
}
