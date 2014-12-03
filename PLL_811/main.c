//------------------------------------------------	

#include <stdio.h>
#include <stdint.h>
#include "string.h"
#include "inc/lm3s811.h"
#include "ST7735.h"
#include "Systick.h"
#include "Switch.h"
#include "LCD.h"
#include "PWM.h"
#include "PulseCount.h"
//------------------------------------------------	

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

//------------------------------------------------	

#define PC7  (*((volatile unsigned long *)0x40006200))
#define PC6  (*((volatile unsigned long *)0x40006100))
#define PC5  (*((volatile unsigned long *)0x40006080))
#define PE0  (*((volatile unsigned long *)0x40024004))

volatile uint32_t sensorInput;
//first, main menu. waits for input. PC5 = constant display distance mode, PC6 = alarm distance mode, PC7 = speed measure mode
volatile int constantDisplayMode = 0;
volatile int distanceAlarmMode = 0;
volatile int speedMeasureMode = 0;
volatile int alarmOn = 0;
volatile int desiredDistance=0;
volatile int digitCount = 0;
volatile int tensValue=0;
volatile int onesValue=0;
volatile int decValue=0;
volatile int pause = 0;
unsigned int count = 0;
volatile int pastDistances[5];
volatile int testPastDistance = 0;

volatile int Tcount = 0;
volatile int StatusRF = 1; //1 = waitng for rising, 0 = waiting for falling

//------------------------------------------------	

void Timer0A_Handler(void){
  TIMER0_ICR_R = TIMER_ICR_CAECINT;// acknowledge timer0A capture match
  //GPIO_PORTC_DATA_R = GPIO_PORTC_DATA_R^0x20; // toggle PC5
	
	if(StatusRF == 1)
	{
		StatusRF = 0;
		
		DisableInterrupts(); // Disable interrupts
		TIMER0_CTL_R &= ~(TIMER_CTL_TAEVENT_POS|0x8);		
		TIMER0_CTL_R |=  (TIMER_CTL_TAEVENT_POS|0x4);		
		EnableInterrupts(); // Disable interrupts
	}
	else if(StatusRF == 0)
	{
		StatusRF = 1;
		
		DisableInterrupts(); // Disable interrupts
		TIMER0_CTL_R &= ~(TIMER_CTL_TAEVENT_POS|0xC);
		EnableInterrupts(); // Disable interrupts
		
		Tcount = Tcount + 1;
	}
}

//------------------------------------------------	

void SysTick_Handler(void){
	//Input Capture, store into sensorInput
	//sensorInput = inputCapture();
  	if( count%24 == 0 )	//every 66ms
		{
				PE0 |= 0x01;
				SysTick_Wait(200);
				PE0 &= ~0x01;				
		}
		count++;	
}

//------------------------------------------------	

void Trigger_Init(void)
{
	int delay = 0;
	SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOE;
	delay = 1000;
	delay = 1000;
	delay = 1000;
  GPIO_PORTE_DIR_R |= 0x01;    // make PE0 out (PC5 built-in LED)
  GPIO_PORTE_AFSEL_R &= ~0x01; // regular port function 
  GPIO_PORTE_DEN_R |= 0x01;    // enable digital I/O on PE0
}

//------------------------------------------------	

int convertSensorInput(void){
	//convert sensor input into inches(meters?). return xx.x as xxx
	//range = high level time * velocity (340M/S) / 2
	//double x = (((double)sensorInput)/(8800.0));
	uint32_t x = PeriodMeasure()/88;
	return x;
}

//------------------------------------------------	

void ringAlarm(){
	PWM_ENABLE_R |= PWM_ENABLE_PWM0EN; // enable PWM0
}

//------------------------------------------------

void silenceAlarm() {
	PWM_ENABLE_R &= ~PWM_ENABLE_PWM0EN; // enable PWM0
}

//------------------------------------------------

volatile int period = 0;

int main2(void)
{
	Output_Init();					// initialize the LCD display		
	SysTick_Init(100000);
	Switch_Init();
	Trigger_Init();
	PeriodMeasure_Init();
	EnableInterrupts();
	
	while(1)
	{
		//if(count%25==0){
		sensorInput = PeriodMeasure();
		ST7735_SetCursor(0,0);
		ST7735_OutUDec(sensorInput/88);
		ST7735_SetCursor(0,1);
		ST7735_OutUDec(sensorInput/1);
		SysTick_Wait(6000);
		//}
	}
	
}








int main(void)
{ 
  Output_Init();					// initialize the LCD display		
	SysTick_Init(100000);
	Switch_Init();
	Trigger_Init();
	PeriodMeasure_Init();
	PWM0_Init(4500,2250);
	EnableInterrupts();

	drawMainMenu();
	while(1)
	{
			if( count%4 == 0 )	//every 66ms
			{
				PE0 |= 0x01;
				SysTick_Wait(60);
				PE0 &= ~0x01;
			}
		
			if(Poll_PC7()){
			if(constantDisplayMode) {
				constantDisplayMode=0;
				drawMainMenu();
				pause = 0;
			}
			else if(distanceAlarmMode){
				if(alarmOn){
					//reset all related variables
					alarmOn = 0;
					desiredDistance = 0;
					silenceAlarm();//even if it's not ringing it should be fine.if not i'll set flag for ringing
					
					distanceAlarmMode = 0;
					drawMainMenu();
				}
				else{
					//advance to next digit/set alarm if digit == 2
					if(digitCount==2){
						digitCount = 0;
						alarmOn = 1;
						desiredDistance *=10;
						desiredDistance+=decValue;
						tensValue = 0;
						onesValue = 0;
						decValue = 0;
						
						Output_Clear();
						ST7735_SetCursor(0,5);
						ST7735_OutString("Alarm will ring when");
						ST7735_SetCursor(0,6);
						ST7735_OutUDec(desiredDistance/10);
						ST7735_OutChar('.');
						ST7735_OutUDec(desiredDistance%10);
						ST7735_OutString(" In. from sensor");
						
					}
					else if(digitCount==1){
						digitCount++;
						desiredDistance+=onesValue;
						
						//Output_Clear();
						//ST7735_SetCursor(0,5);
						//ST7735_OutString("Set distance for");
						ST7735_SetCursor(0,6);
						ST7735_OutString("alarm: (Tenth Place)");
						//ST7735_SetCursor(0,7);
						//ST7735_OutUDec(desiredDistance);
						//ST7735_OutChar('.');
						//ST7735_OutUDec(0);
						
					}
					else if(digitCount==0){
						digitCount++;
						desiredDistance = tensValue*10;
						
						ST7735_SetCursor(0,6);
						ST7735_OutString("alarm: (One's Place)");
						//ST7735_SetCursor(0,7);
						//ST7735_OutUDec(desiredDistance);
						//ST7735_OutChar('.');
						//ST7735_OutUDec(0);
					}
				}
			}
			else if(speedMeasureMode) {
				speedMeasureMode = 0;
				drawMainMenu();
				pause = 0;
				testPastDistance=0;
			}
			else{
				constantDisplayMode = 1; //enter constant display mode
				Output_Clear(); //clear the output to allow new Mode display
				//setCursor center of screen
				ST7735_SetCursor(0,5);
				ST7735_OutString("Current Dist. in");
				ST7735_SetCursor(0,6);
				ST7735_OutString("Front of Sensor:");
			}
		}
		
		else if(Poll_PC6()){
			if(constantDisplayMode) {
				if(pause == 0){
					pause = 1;
				}
				else{
					pause = 0;
				}
			}
			else if(distanceAlarmMode){
				if(!alarmOn && count%11==0){
					if(digitCount==0 && tensValue>0){
						tensValue--;
					}
					if(digitCount==1 && onesValue>0){
						onesValue--;
					}
					if(digitCount==2 && decValue>0){
						decValue--;
					}
					ST7735_SetCursor(0,7);
					ST7735_OutUDec(tensValue);
					ST7735_OutUDec(onesValue);
					ST7735_OutChar('.');
					ST7735_OutUDec(decValue);
				}
			}
			else if(speedMeasureMode) {
				if(pause == 0){
					pause = 1;
				}
				else{
					pause = 0;
				}
			}
			else{
				distanceAlarmMode = 1;
						Output_Clear();
						ST7735_SetCursor(0,5);
						ST7735_OutString("Set distance for");
						ST7735_SetCursor(0,6);
						ST7735_OutString("alarm: (Ten's Place)");
						ST7735_SetCursor(0,7);
						ST7735_OutUDec(0);
						ST7735_OutUDec(0);
						ST7735_OutChar('.');
						ST7735_OutUDec(0);
			}
		}
		
		else if(Poll_PC5()){
			if(constantDisplayMode) {
				//does nothing
			}
			else if(distanceAlarmMode){
				if(!alarmOn && count%11==0){
					if(digitCount==0){
						tensValue = (tensValue+1)%10;
					}
					if(digitCount==1){
						onesValue = (onesValue+1)%10;
					}
					if(digitCount==2){
						decValue = (decValue+1)%10;
					}
					ST7735_SetCursor(0,7);
					ST7735_OutUDec(tensValue);
					ST7735_OutUDec(onesValue);
					ST7735_OutChar('.');
					ST7735_OutUDec(decValue);
				}
			}
			else if(speedMeasureMode) {
				//does nothing
			}
			else{
				int q;
				speedMeasureMode = 1;
				Output_Clear();
				for(q = 0;q<5;q++){
					pastDistances[q] = -1;
				}
				
				Output_Clear();
				ST7735_SetCursor(0,5);
				ST7735_OutString("Current Speed In");
				ST7735_SetCursor(0,6);
				ST7735_OutString("Front of Sensor:");
				
			}
		}
		else{
			//else do default action of that mode
			if(constantDisplayMode && count%51==0) {
				if(pause==0){
					int sensorOutput = 0;
					//Output_Clear();
					sensorOutput = convertSensorInput(); //returns xx.x in form xxx. so first print out first 2, then '.' , then last digit
					
					ST7735_SetCursor(0,7);
					ST7735_OutUDec(sensorOutput/10);
					ST7735_OutChar('.');
					ST7735_OutUDec(sensorOutput%10);
					ST7735_OutString(" Inches");
					
					//ST7735_SetCursor(0,9);
					//ST7735_OutString("Pause: ");
					//ST7735_OutUDec(pause);
				}
				else{
					//ST7735_SetCursor(0,9);
					//ST7735_OutString("Pause: ");
					//ST7735_OutUDec(pause);
					//nothing, don't update what's on the screen
				}
			}
			else if(distanceAlarmMode && count%51==0){
				if(!alarmOn){
					//update with current user selected values depending on digitCount
					//Output_Clear();
					//ST7735_SetCursor(0,5);
					/*
					if(digitCount==0){
						//ST7735_OutString("Set distance for");
						ST7735_SetCursor(0,6);
						ST7735_OutString("alarm: (Ten's Place)");
					}
					else if(digitCount==1){
						//ST7735_OutString("Set distance for");
						ST7735_SetCursor(0,6);
						ST7735_OutString("alarm: (One's Place)");
					}
					else if(digitCount==0){
						//ST7735_OutString("Set distance for");
						ST7735_SetCursor(0,6);
						ST7735_OutString("alarm: (Dec Place)");
					}
					ST7735_SetCursor(0,7);
					ST7735_OutUDec(tensValue);
					ST7735_OutUDec(onesValue);
					ST7735_OutChar('.');
					ST7735_OutUDec(decValue);*/
				}
				else{
					int x = convertSensorInput();
					if((x-10)<=desiredDistance && (x+10)>=desiredDistance){
						ringAlarm();
					}
				}
			}
			else if(speedMeasureMode && count%51==0) {
				//Output_Clear();
				
				if(testPastDistance!=0) {
					int x = convertSensorInput();
					if((x-testPastDistance)>0 && (x-testPastDistance)<40){
						ST7735_SetCursor(0,7);
						ST7735_OutUDec((x-testPastDistance)/(2)); //inches per second
						ST7735_OutString(" in/sec");
						ST7735_SetCursor(0,8);
						ST7735_OutUDec(x);
					}
				}
				testPastDistance=convertSensorInput();
			}
			else{
				//else still in main menu, do nothing. wait for user input
			}
		}
	}	
}

/*
if( Counts%4 == 0 )	//every 66ms
		{
			PE0 |= 0x01;
			SysTick_Wait(60);
			PE0 &= ~0x01;
		}
		if( ( Poll_PC5()() ) > 0 )
		{
			  ST7735_SetCursor(0,0);
				ST7735_OutUDec(5);
		}
		if( ( Poll_PC6()() ) > 0 )
		{
			  ST7735_SetCursor(0,0);
				ST7735_OutUDec(6);
		}
		if( ( Poll_PC7()() ) > 0 )
		{
			  ST7735_SetCursor(0,0);
				ST7735_OutUDec(7);
		}
		if( ((Poll_PC5()()) == 0) && ((Poll_PC6()()) == 0) && ((Poll_PC7()()) == 0) )
		{
			  ST7735_SetCursor(0,0);
				ST7735_OutUDec(0);
		}	
		*/

