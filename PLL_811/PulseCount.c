// HighPulseMeasure.c
// Runs on LM3S811
// Use Timer0A in edge time mode to record time at rising and falling
// edges of PD4 (CCP0), and subtract them to get high pulse duration.
// Daniel Valvano
// June 27, 2011

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to the Arm Cortex M3",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2011

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

// pulse start signal connected to PC5 (rising edge starts pulse)
// external signal connected to PD4 (CCP0) (trigger on rising edge)

#include <stdio.h>
#include <stdint.h>
#include "string.h"
#include "inc/lm3s811.h"

#define TIMER0_CFG_R            (*((volatile unsigned long *)0x40030000))
#define TIMER0_TAMR_R           (*((volatile unsigned long *)0x40030004))
#define TIMER0_CTL_R            (*((volatile unsigned long *)0x4003000C))
#define TIMER0_IMR_R            (*((volatile unsigned long *)0x40030018))
#define TIMER0_RIS_R            (*((volatile unsigned long *)0x4003001C))
#define TIMER0_ICR_R            (*((volatile unsigned long *)0x40030024))
#define TIMER0_TAILR_R          (*((volatile unsigned long *)0x40030028))
#define TIMER0_TAR_R            (*((volatile unsigned long *)0x40030048))
#define TIMER_CFG_16_BIT        0x00000004  // 16-bit timer configuration,
                                            // function is controlled by bits
                                            // 1:0 of GPTMTAMR and GPTMTBMR
#define TIMER_TAMR_TACMR        0x00000004  // GPTM TimerA Capture Mode
#define TIMER_TAMR_TAMR_CAP     0x00000003  // Capture mode
#define TIMER_CTL_TAEN          0x00000001  // GPTM TimerA Enable
#define TIMER_CTL_TAEVENT_M     0x0000000C  // GPTM TimerA Event Mask
#define TIMER_CTL_TAEVENT_POS   0x00000000  // Positive edge
#define TIMER_CTL_TAEVENT_NEG   0x00000004  // Negative edge
#define TIMER_RIS_CAERIS        0x00000004  // GPTM CaptureA Event Raw
                                            // Interrupt
#define TIMER_ICR_CAECINT       0x00000004  // GPTM CaptureA Event Interrupt
                                            // Clear
#define TIMER_TAILR_TAILRL_M    0x0000FFFF  // GPTM TimerA Interval Load
                                            // Register Low
#define GPIO_PORTC_DIR_R        (*((volatile unsigned long *)0x40006400))
#define GPIO_PORTC_DEN_R        (*((volatile unsigned long *)0x4000651C))
#define GPIO_PORTC5             (*((volatile unsigned long *)0x40006080))
#define GPIO_PORTD_AFSEL_R      (*((volatile unsigned long *)0x40007420))
#define GPIO_PORTD_DEN_R        (*((volatile unsigned long *)0x4000751C))
#define SYSCTL_RCGC1_R          (*((volatile unsigned long *)0x400FE104))
#define SYSCTL_RCGC2_R          (*((volatile unsigned long *)0x400FE108))
#define SYSCTL_RCGC1_TIMER0     0x00010000  // timer 0 Clock Gating Control
#define SYSCTL_RCGC2_GPIOD      0x00000008  // port D Clock Gating Control
#define SYSCTL_RCGC2_GPIOC      0x00000004  // port C Clock Gating Control
#define CALIBRATION             0

void PeriodMeasure_Init(void){
  volatile unsigned long delay;
  SYSCTL_RCGC1_R |= SYSCTL_RCGC1_TIMER0;// activate timer0
                                   // activate port C and port D
  SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOD;
  delay = SYSCTL_RCGC2_R;          // allow time to finish activating
  //GPIO_PORTC5 = 0x20;              // turn on PC5
 // GPIO_PORTC_DIR_R |= 0x20;        // make PC5 out (PC5 built-in LED)
  //GPIO_PORTC_DEN_R |= 0x20;        // enable digital I/O on PC5
  GPIO_PORTD_DEN_R |= 0x10;        // enable digital I/O on PD4
  GPIO_PORTD_AFSEL_R |= 0x10;      // enable alt funct on PD4
  TIMER0_CTL_R &= ~TIMER_CTL_TAEN; // disable timer0A during setup
  TIMER0_CFG_R = TIMER_CFG_16_BIT; // configure for 16-bit timer mode
                                   // configure for capture mode
  TIMER0_TAMR_R = (TIMER_TAMR_TACMR|TIMER_TAMR_TAMR_CAP);
  TIMER0_TAILR_R = TIMER_TAILR_TAILRL_M;// start value
  TIMER0_IMR_R &= ~0x7;            // disable all interrupts for timer0A
  TIMER0_ICR_R = TIMER_ICR_CAECINT;// clear timer0A capture match flag
  TIMER0_CTL_R |= TIMER_CTL_TAEN;  // enable timer0A 16-b, edge timing, no interrupts
}
// return resistance in kohms
// 0 to 1000 kohm
unsigned short PeriodMeasure(void){
  unsigned short rising;
  TIMER0_ICR_R = TIMER_ICR_CAECINT;// clear timer0A capture match flag
  //GPIO_PORTC5 = 0x00;              // turn off PC5
  //GPIO_PORTC5 = 0x20;              // turn on PC5
                                   // clear trigger event field
  TIMER0_CTL_R &= ~TIMER_CTL_TAEVENT_M;
                                   // configure for rising edge event
  TIMER0_CTL_R += TIMER_CTL_TAEVENT_POS;
  while((TIMER0_RIS_R&TIMER_RIS_CAERIS)==0){};//wait for rise
  rising = TIMER0_TAR_R;           // timerA0 at rising edge
  TIMER0_ICR_R = TIMER_ICR_CAECINT;// clear timer0A capture match flag
                                   // clear trigger event field
  TIMER0_CTL_R &= ~TIMER_CTL_TAEVENT_M;
                                   // configure for falling edge event
  TIMER0_CTL_R += TIMER_CTL_TAEVENT_NEG;
  while((TIMER0_RIS_R&TIMER_RIS_CAERIS)==0){};//wait for fall
  TIMER0_ICR_R = TIMER_ICR_CAECINT;// clear timer0A capture match flag
  return(rising-TIMER0_TAR_R-CALIBRATION)&0xFFFF;
}


