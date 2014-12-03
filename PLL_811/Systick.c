//------------------------------------------------

#include <stdio.h>
#include <stdint.h>
#include "inc/lm3s811.h"

//------------------------------------------------

void SysTick_Init(unsigned long period){
  int delay = 0;
	SYSCTL_RCGC2_R |= 0x00000008; // activate port D
	delay = 0;
	delay = 9;
  GPIO_PORTD_DIR_R |= 0x01;   // make PD0 out
  GPIO_PORTD_DEN_R |= 0x01;   // enable digital I/O on PD0
  NVIC_ST_CTRL_R = 0;         // disable SysTick during setup
  NVIC_ST_RELOAD_R = period - 1;// reload value
  NVIC_ST_CURRENT_R = 0;      // any write to current clears it
  NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x00FFFFFF)|0x40000000; // priority 2                
  NVIC_ST_CTRL_R = 0x00000007;// enable SysTick with core clock and interrupts
}

//------------------------------------------------

void SysTick_Wait(unsigned long delay){
  volatile unsigned long elapsedTime;
  unsigned long startTime = NVIC_ST_CURRENT_R;
  do{
    elapsedTime = (startTime-NVIC_ST_CURRENT_R)&0x00FFFFFF;
  }
  while(elapsedTime <= delay);
}
