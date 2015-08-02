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
static buttonState_t io_lastButtonState[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
//-----------------------------------------------------------
void io_init()
{
  //all LED pins as inputs => off
  LED_DDR_13 &= ~(  (1<<LED_1_PIN) | (1<<LED_2_PIN) | (1<<LED_3_PIN)  );
  LED_DDR_46 &= ~(  (1<<LED_4_PIN) | (1<<LED_5_PIN) | (1<<LED_6_PIN)  );
  
  //all buttons rows as inputs (attention SS pin used!)
  SWITCH_DDR_12 &= ~(  (1<<SWITCH_ROW1_PIN) | (1<<SWITCH_ROW2_PIN) );
  SWITCH_DDR_3 &= ~(1<<SWITCH_ROW3_PIN);
  //pullup on
  SWITCH_PORT_12 |= (1<<SWITCH_ROW1_PIN) | (1<<SWITCH_ROW2_PIN);
  SWITCH_PORT_3 |= (1<<SWITCH_ROW3_PIN);
  
  //all buttown columns as outs, state high
  COL_DDR |= (1<<COL1_PIN) | (1<<COL2_PIN) | (1<<COL3_PIN) | (1<<COL4_PIN);
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
//Get new state for single button in row:col based on lastState
//@return new buttons state
static inline buttonState_t io_debounceButton( const uint8_t row, const uint8_t col, const buttonState_t lastState )
{
	buttonState_t state = lastState;
	uint8_t val=0;

	//all columns on
	COL_PORT |= ( (1<<COL1_PIN) | (1<<COL2_PIN) | (1<<COL3_PIN) | (1<<COL4_PIN) );
	//pin low for active column
	COL_PORT &= ~(1<<col);

	switch(row)
	{
		case 0:
				val = (SWITCH_INPUT_12 & (1<<SWITCH_ROW1_PIN) ) == 0;
				break;

		case 1:
				val = (SWITCH_INPUT_12 & (1<<SWITCH_ROW2_PIN) ) == 0;
				break;

		case 2:
				val = (SWITCH_INPUT_3 & (1<<SWITCH_ROW3_PIN) ) == 0;
				break;
	}

	// Add current pin state to debounce counter; changes in state should only
	// happen when mask is full (rising edge) or empty (falling edge)
	state = (( state << 1 ) | val ) & BUTTON_DEBOUNCE_MASK;

	if ( (lastState & BUTTON_STATE_PUSHED) && (0 != state) )
	{
		 // was pushed already, but release debounce not met
		state |= BUTTON_STATE_PUSHED;
	}
	else if ( state == BUTTON_DEBOUNCE_MASK )
	{
		// pushed (either new or held)
		state |= BUTTON_STATE_PUSHED;
	}
	// else default to not pressed

	return state;
}
//-----------------------------------------------------------
//read button matrix
void io_processButtons()
{
	uint8_t col;
	uint8_t row;
	uint8_t i=0;
	buttonState_t lastState, newState;

	for(row=0;row<3;row++)
	{
		for(col=0;col<4;col++)
		{
			lastState = io_lastButtonState[i];
			newState = io_debounceButton( row, col, lastState );

			//check if the button changed its state since the last call
			if ( (lastState ^ newState) & BUTTON_STATE_PUSHED )
			{
				// Handle rising edge of push
				if ( newState & BUTTON_STATE_PUSHED )
				{
					timer_touchAutosave();
					//toggle LED
					if (!(io_ledState&(1<<i)))
						io_ledState |= (1<<i);
					else
						io_ledState &= ~(1<<i);
				}
			}

			//update state memory
			io_lastButtonState[i] = newState;
			++i;
		}
	}
};
//-----------------------------------------------------------
static uint8_t buttonRowIndex = 0;
static uint8_t buttonColIndex = 0;
static uint8_t ledNr = 0;

void io_processButtonsPipelined()
{
	buttonState_t lastState, newState;

	lastState = io_lastButtonState[ ledNr ];
	newState = io_debounceButton( buttonRowIndex, buttonColIndex, lastState );

	//check if the button changed its state since the last call
	if ( (lastState ^ newState) & BUTTON_STATE_PUSHED )
	{
		// Handle rising edge of push
		if ( newState & BUTTON_STATE_PUSHED )
		{
			timer_touchAutosave();
			//toggle LED
			if (!(io_ledState&(1<<ledNr)))
				io_ledState |= (1<<ledNr);
			else
				io_ledState &= ~(1<<ledNr);
		}
	}

	//update state memory
	io_lastButtonState[ ledNr ] = newState;
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
uint8_t io_isButtonPushed(uint8_t buttonNr)
{
  uint8_t row = buttonNr/4; // [0:2]
  uint8_t col = buttonNr%4; // [0:3]

  //all columns on
  COL_PORT |= ( (1<<COL1_PIN) | (1<<COL2_PIN) | (1<<COL3_PIN) | (1<<COL4_PIN) );
  //pin low for active column
  COL_PORT &= ~(1<<col);
  
  //read active row input
  uint8_t val = 0;
  switch(row)
  {
  case 0:
      val = (SWITCH_INPUT_12 & (1<<SWITCH_ROW1_PIN) ) == 0;
      break;

  case 1:
      val = (SWITCH_INPUT_12 & (1<<SWITCH_ROW2_PIN) ) == 0;
      break;
    
  case 2:
      val = (SWITCH_INPUT_3 & (1<<SWITCH_ROW3_PIN) ) == 0;
      break;
  }
  
  return val;
}
//-----------------------------------------------------------

