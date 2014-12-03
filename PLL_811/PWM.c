// PWM.c
// Runs on LM3S811
// Use PWM0 to generate pulse-width modulated outputs
// Output on PD0, fault input on PD6
// Daniel Valvano
// June 27, 2011

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to the Arm Cortex M3",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2011
  Program 6.7, section 6.3.2

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

#include "inc/lm3s811.h"

#define PWM_ENABLE_R            (*((volatile unsigned long *)0x40028008))
#define PWM_ENABLE_PWM0EN       0x00000001  // PWM0 Output Enable
#define PWM_0_CTL_R             (*((volatile unsigned long *)0x40028040))
#define PWM_X_CTL_ENABLE        0x00000001  // PWM Block Enable
#define PWM_0_LOAD_R            (*((volatile unsigned long *)0x40028050))
#define PWM_0_CMPA_R            (*((volatile unsigned long *)0x40028058))
#define PWM_0_GENA_R            (*((volatile unsigned long *)0x40028060))
#define PWM_X_GENA_ACTCMPAD_ONE 0x000000C0  // Set the output signal to 1
#define PWM_X_GENA_ACTLOAD_ZERO 0x00000008  // Set the output signal to 0
#define GPIO_PORTD_AFSEL_R      (*((volatile unsigned long *)0x40007420))
#define GPIO_PORTD_DEN_R        (*((volatile unsigned long *)0x4000751C))
#define SYSCTL_RCC_R            (*((volatile unsigned long *)0x400FE060))
#define SYSCTL_RCC_USEPWMDIV    0x00100000  // Enable PWM Clock Divisor
#define SYSCTL_RCC_PWMDIV_M     0x000E0000  // PWM Unit Clock Divisor
#define SYSCTL_RCC_PWMDIV_2     0x00000000  // /2
#define SYSCTL_RCGC0_R          (*((volatile unsigned long *)0x400FE100))
#define SYSCTL_RCGC0_PWM        0x00100000  // PWM Clock Gating Control
#define SYSCTL_RCGC2_R          (*((volatile unsigned long *)0x400FE108))
#define SYSCTL_RCGC2_GPIOD      0x00000008  // Port D Clock Gating Control


// period is 16-bit number of PWM clock cycles in one period (3<=period)
// duty is number of PWM clock cycles output is high  (2<=duty<=period-1)
// PWM clock rate = processor clock rate/SYSCTL_RCC_PWMDIV
//                = BusClock/2 
//                = 6 MHz/2 = 3 MHz (in this example)
// PWM clock rate = processor clock rate/SYSCTL_RCC_PWMDIV
void PWM0_Init(unsigned short period, unsigned short duty){
  volatile unsigned long delay;
  SYSCTL_RCGC0_R |= SYSCTL_RCGC0_PWM;   // 1)activate PWM
  SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOD; // 2)activate port D
  delay = SYSCTL_RCGC2_R;               // allow time to finish activating
  GPIO_PORTD_AFSEL_R |= 0x01;           //  enable alt funct on PD0
  GPIO_PORTD_DEN_R |= 0x01;             //  enable digital I/O on PD0
  SYSCTL_RCC_R |= SYSCTL_RCC_USEPWMDIV; // 3) use PWM divider
  SYSCTL_RCC_R &= ~SYSCTL_RCC_PWMDIV_M; //    clear PWM divider field
  SYSCTL_RCC_R += SYSCTL_RCC_PWMDIV_2;  //    configure for /2 divider
  PWM_0_CTL_R = 0;                      // 4) re-loading mode
  PWM_0_GENA_R = (PWM_X_GENA_ACTCMPAD_ONE|PWM_X_GENA_ACTLOAD_ZERO);
  PWM_0_LOAD_R = period - 1;       // 5) cycles needed to count down to 0
  PWM_0_CMPA_R = duty - 1;         // 6) count value when output rises
  PWM_0_CTL_R |= PWM_X_CTL_ENABLE; // 7) start PWM0
  //PWM_ENABLE_R |= PWM_ENABLE_PWM0EN; // enable PWM0
}
// change duty cycle
// duty is number of PWM clock cycles output is high  (2<=duty<=period-1)
void PWM0_Duty(unsigned short duty){
  PWM_0_CMPA_R = duty - 1;         // 6) count value when output rises
}


void PWM4_Init(unsigned short period, unsigned short duty){
  volatile unsigned long delay;
  SYSCTL_RCGC0_R |= SYSCTL_RCGC0_PWM;   // 1)activate PWM
  SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOE; // 2)activate port D
  delay = SYSCTL_RCGC2_R;               // allow time to finish activating
  GPIO_PORTD_AFSEL_R |= 0x01;           //  enable alt funct on PD0
  GPIO_PORTD_DEN_R |= 0x01;             //  enable digital I/O on PD0
  SYSCTL_RCC_R |= SYSCTL_RCC_USEPWMDIV; // 3) use PWM divider
  SYSCTL_RCC_R &= ~SYSCTL_RCC_PWMDIV_M; //    clear PWM divider field
  SYSCTL_RCC_R += SYSCTL_RCC_PWMDIV_2;  //    configure for /2 divider
  PWM_0_CTL_R = 0;                      // 4) re-loading mode
  PWM_0_GENA_R = (PWM_X_GENA_ACTCMPAD_ONE|PWM_X_GENA_ACTLOAD_ZERO);
  PWM_0_LOAD_R = period - 1;       // 5) cycles needed to count down to 0
  PWM_0_CMPA_R = duty - 1;         // 6) count value when output rises
  PWM_0_CTL_R |= PWM_X_CTL_ENABLE; // 7) start PWM0
  //PWM_ENABLE_R |= PWM_ENABLE_PWM0EN; // enable PWM0
}
