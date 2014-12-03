//------------------------------------------------

#include <stdio.h>
#include <stdint.h>
#include "inc/lm3s811.h"
#include "ST7735.h"

void drawMainMenu(void) {
	Output_Clear();
	ST7735_SetCursor(0,5);
	ST7735_OutString("Welcome to");
	ST7735_SetCursor(0,6);
	ST7735_OutString("Distanceometer");
	ST7735_SetCursor(0,8);
	ST7735_OutString("Choose a mode:");
	
	ST7735_SetCursor(0,9);
	ST7735_OutString("PC7 - Constant");
	ST7735_SetCursor(0,10);
	ST7735_OutString("      Display Mode");
	ST7735_SetCursor(0,11);
	ST7735_OutString("PC6 - Distance");
	ST7735_SetCursor(0,12);
	ST7735_OutString("      Alarm Mode");
	ST7735_SetCursor(0,13);
	ST7735_OutString("PC5 - Speed Measure");
}
