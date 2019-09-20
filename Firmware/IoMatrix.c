/*
 * IoMatrix.c
 *
 *  Copyright 2015 Julian Schmidt, Sonic Potions <julian@sonic-potions.com>
 *  Web: www.sonic-potions.com/penrose
 * 
 *  This file is part of the Penrose Quantizer Firmware.
 *
 *  The Penrose Quantizer Firmware is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  The Penrose Quantizer Firmware is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with the Penrose Quantizer Firmware.  If not, see <http://www.gnu.org/licenses/>.
 */ 

#include "IoMatrix.h"
#include <util/delay.h>
#include <avr/pgmspace.h> 
#include "spi.h"
#include "timebase.h"

//-----------------------------------------------------------
/*
led	pin A	Pin B
0	1	2
1	1	3
2	1	4
3	2	3
4	2	4
5	2	5
6	3	4
7	3	5
8	3	6
9 	4	5
10	4	6
11	5	6
*/
static const uint8_t ledPinArray[12][2] PROGMEM = {
  {LED_1_PIN,LED_2_PIN},
  {LED_1_PIN,LED_3_PIN},
  {LED_1_PIN,LED_4_PIN},
  {LED_2_PIN,LED_3_PIN},
  {LED_2_PIN,LED_4_PIN},
  {LED_2_PIN,LED_5_PIN},
  {LED_3_PIN,LED_4_PIN},
  {LED_3_PIN,LED_5_PIN},
  {LED_3_PIN,LED_6_PIN},
  {LED_4_PIN,LED_5_PIN},
  {LED_4_PIN,LED_6_PIN},
  {LED_5_PIN,LED_6_PIN},
};
//-----------------------------------------------------------
static uint16_t io_ledState=0xfff;		//state of the 12 LEDs == activated notes
static uint8_t io_activeStep=2;			//current active quantisation step == currently played note
static uint16_t io_lastButtonState=0x00;
//-----------------------------------------------------------
void io_init()
{
  //all LED pins as inputs => off
  LED_DDR_13 &= ~(  (1<<LED_1_PIN) | (1<<LED_2_PIN) | (1<<LED_3_PIN)  );
  LED_DDR_46 &= ~(  (1<<LED_4_PIN) | (1<<LED_5_PIN) | (1<<LED_6_PIN)  );
  
  //all button rows as outputs, state high
  SWITCH_DDR_12 |= (1<<SWITCH_ROW1_PIN) | (1<<SWITCH_ROW2_PIN);
  SWITCH_DDR_3 |= (1<<SWITCH_ROW3_PIN);
  SWITCH_PORT_12 |= (1<<SWITCH_ROW1_PIN) | (1<<SWITCH_ROW2_PIN);
  SWITCH_PORT_3 |= (1<<SWITCH_ROW3_PIN);
  
  //all button columns as inputs with pullup
  COL_DDR &= ~((1<<COL1_PIN) | (1<<COL2_PIN) | (1<<COL3_PIN) | (1<<COL4_PIN));
  COL_PORT |= ((1<<COL1_PIN) | (1<<COL2_PIN) | (1<<COL3_PIN) | (1<<COL4_PIN));
};
//-----------------------------------------------------------
uint16_t io_getActiveSteps()
{
  return io_ledState;
}
//-----------------------------------------------------------
void io_setActiveSteps(uint16_t val)
{
  io_ledState = val;
}
//-----------------------------------------------------------
void io_setCurrentQuantizedValue(uint8_t value)
{
  io_activeStep = value;
}
//-----------------------------------------------------------
void turnAllLedsOff()
{
  //all pins low / no pullup
  LED_PORT_13 &= ~( (1<<LED_1_PIN) | (1<<LED_2_PIN) | (1<<LED_3_PIN) );
  LED_PORT_46 &= ~( (1<<LED_4_PIN) | (1<<LED_5_PIN) | (1<<LED_6_PIN) );
  
  //all LED pins as inputs => off
  LED_DDR_13 &= ~(  (1<<LED_1_PIN) | (1<<LED_2_PIN) | (1<<LED_3_PIN)  );
  LED_DDR_46 &= ~(  (1<<LED_4_PIN) | (1<<LED_5_PIN) | (1<<LED_6_PIN)  );
}
//-----------------------------------------------------------
void turnLedOn(uint16_t ledNr, uint8_t colour)
{
  //get the needed pins from the LED array
  uint8_t pinA = pgm_read_byte(&ledPinArray[ledNr][0]);
  uint8_t pinB = pgm_read_byte(&ledPinArray[ledNr][1]);
  
  if(colour)
  {
    //colour 2 -> A=0, B=1
    if(pinA > LED_3_PIN)
    {
      LED_DDR_46 	|= (1<<pinA);	//pin as output
      LED_PORT_46	&= ~(1<<pinA);	//pin low
    }
    else
    {
      LED_DDR_13	|= (1<<pinA);
      LED_PORT_13	&= ~(1<<pinA);
    }
    
    if(pinB > LED_3_PIN)
    {
      LED_DDR_46	|= (1<<pinB);	//pin as output
      LED_PORT_46 	|= (1<<pinB);	//pin high
    }
    else
    {
      LED_DDR_13	|= (1<<pinB);
      LED_PORT_13	|= (1<<pinB);
    }
  }
  else
  {
    //colour 1 -> A out, B in
    if(pinA > LED_3_PIN)
    {
      LED_DDR_46 	|= (1<<pinA);	//pin as output
      LED_PORT_46	|= (1<<pinA);	//pin high
    }
    else
    {
      LED_DDR_13	|= (1<<pinA);
      LED_PORT_13	|= (1<<pinA);
    }
    
    if(pinB > LED_3_PIN)
    {
      LED_DDR_46	|= (1<<pinB);	//pin as output
      LED_PORT_46 	&= ~(1<<pinB);	//pin low
    }
    else
    {
      LED_DDR_13	|= (1<<pinB);
      LED_PORT_13	&= ~(1<<pinB);
    }    
  }
}
//-----------------------------------------------------------
//one circle trough the whole LED matrix
void io_processLed()
{
  for(int i=0; i<12; i++)
  {    
    if(i==io_activeStep)
    {
      //this step is currently played => set color 1
      turnLedOn(i,0);
    } 
    else if ( (io_ledState & (1<<i)) > 0)
    {
      //step is active => colour 2
      turnLedOn(i,1);
    }
     _delay_us(300); //otherwise the LEDs are too dark
    turnAllLedsOff();
  }
};
//-----------------------------------------------------------
static uint8_t ledState = 0;
void io_processLedPipelined()
{
  turnAllLedsOff();

  if(ledState==io_activeStep)
  {
    //this step is currently played => set color 1
    turnLedOn(ledState,0);
  } 
  else if ( (io_ledState & (1<<ledState)) > 0)
  {
    //step is active => colour 2
    turnLedOn(ledState,1);
  }
  
  ledState++;
  if(ledState>=12) ledState=0;
};
//-----------------------------------------------------------
static uint8_t buttonRowIndex = 0;
static uint8_t buttonColIndex = 0;
static uint8_t ledNr = 0;

void io_processButtonsPipelined()
{
	uint8_t i= ledNr;
	uint16_t val;
	  
	//all rows on
	SWITCH_PORT_12 |= (1<<SWITCH_ROW1_PIN) | (1<<SWITCH_ROW2_PIN);
	SWITCH_PORT_3 |= (1<<SWITCH_ROW3_PIN);
	//pin low for active row
	switch(buttonRowIndex)
	{
	  default:
	  case 0:
	      SWITCH_PORT_12 &= ~(1<<SWITCH_ROW1_PIN);
	      break;

	  case 1:
	      SWITCH_PORT_12 &= ~(1<<SWITCH_ROW2_PIN);
	      break;

	  case 2:
	      SWITCH_PORT_3 &= ~(1<<SWITCH_ROW3_PIN);
	      break;
	}

	// Wait for the pin to settle
	_delay_us(1);
	//read active column input
	val = (COL_INPUT & (1<<buttonColIndex)) == 0;
    
	//check if the button changed its state since the last call
	if(   (io_lastButtonState&(1<<i))   != (val<<i)   )
	{
		//update state memory
		io_lastButtonState &= ~(1<<i);
		io_lastButtonState |=val<<i;
		//toggle LED
		if(val)
		{
		  timer_touchAutosave();
		  if(!(io_ledState&(1<<i)))
		  {
		    io_ledState |= 1<<i;
		  } else 
		  {
		    io_ledState &= ~(1<<i);
		  }
		}
	}
	
	ledNr++;
	
	buttonColIndex++;
	if(buttonColIndex>=4)
	{
	  buttonColIndex=0;
	  buttonRowIndex++;
	  if(buttonRowIndex>=3)
	  {
	    buttonRowIndex=0;
	    ledNr = 0;
	  }
	}
}
//-----------------------------------------------------------

