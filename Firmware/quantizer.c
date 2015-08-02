/*
 * quantizer.c
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

#include <avr/io.h>

#include "MCP4802.h"
#include "adc.h"
#include "IoMatrix.h"
#include "eeprom.h"
#include "timebase.h"
#include <util/atomic.h>
#include <util/delay.h> 
#include <avr/interrupt.h>  
#include <stdlib.h>

//-----------------------------------------------------------
static uint8_t quantizeValue(uint16_t input);
static uint16_t lastInput=0;
static uint8_t lastQuantValue=0;
//-----------------------------------------------------------

#define TRIGGER_INPUT_PIN		PD7
#define TRIGGER_INPUT_IN_PORT		PIND
#define TRIGGER_INPUT_PORT		PORTD
#define TRIGGER_INPUT_DDR		DDRD

#define SWITCH_PIN			PC4
#define SWITCH_PORT			PORTC
#define SWITCH_IN_PORT			PINC
#define SWITCH_DDR			DDRC

#define INPUT_VOLTAGE			5.f	//Volt
#define OCTAVES				10.f	//octaves
#define VOLT_PER_OCTAVE			(INPUT_VOLTAGE/OCTAVES) // 0.5
#define VOLT_PER_NOTE			(VOLT_PER_OCTAVE/12.f)  // 0.04166666666666666667
#define VOLT_PER_ADC_STEP		(INPUT_VOLTAGE/1024.f)  // 0.0048828125
#define ADC_STEPS_PER_NOTE		(VOLT_PER_NOTE/VOLT_PER_ADC_STEP) //~8.53

#define GATE_IN_CONNECTED ((SWITCH_IN_PORT & (1<<SWITCH_PIN))==0)
//-----------------------------------------------------------
void init()
{
    // power save stuff
    ACSR |= (1<<ACD); //analog comparator off
  
    //switch is input with pullup
    SWITCH_DDR &= ~(1<<SWITCH_PIN);
    SWITCH_PORT |= (1<<SWITCH_PIN);

    //trigger is input with no pullup
    TRIGGER_INPUT_DDR &= ~(1<<TRIGGER_INPUT_PIN);
    TRIGGER_INPUT_PORT &= ~(1<<TRIGGER_INPUT_PIN);

    timer_init();
    mcp4802_init();
    adc_init();
    io_init();

    /*
    Set up Interrupt for trigger input 

    PCINT0 to PCINT7 refer to the PCINT0 interrupt, PCINT8 to PCINT14 refer to the PCINT1 interrupt 
    and PCINT15 to PCINT23 refer to the PCINT2 interrupt
    -->
    TRIGGER_INPUT_PIN = PD7 = PCINT23 = pcint2 pin change interrupt for trigger
    */
    //interrupt trigger	(pin change)

    PCICR |= (1<<PCIE2);   //Enable PCINT2
    PCMSK2 |= (1<<PCINT23); //Trigger on change of PCINT23 (PD7)
       
    sei();
}
//-----------------------------------------------------------
void process()
{
	const uint16_t input = adc_read()+1;

	if (abs(input-lastInput) < ADC_STEPS_PER_NOTE/2)
		return;

	const uint8_t quantValue = quantizeValue(input);
	//if the value changed, update DAC (also sets gate)
	if(lastQuantValue != quantValue)
	{
		lastInput = input;
		lastQuantValue = quantValue;
		
		mcp4802_outputData(quantValue,0);
		//start gate off timer
		timer0_start();
	}
}
//-----------------------------------------------------------
ISR(PCINT2_vect)
{
  if(bit_is_clear(PIND,7)) //only rising edge
  {
    process();
  }	
  return;
};
//-----------------------------------------------------------
uint8_t quantizeValue(uint16_t input)
{
  const uint16_t activeSteps = io_getActiveSteps();
  if(!activeSteps)
  {
    //no stepselected
    io_setCurrentQuantizedValue(99); //no active step LED
    return 0;
  }

	//quantize input value to all steps
	/* instead of input/ADC_STEPS_PER_NOTE we use the magic number 17 here.
	 * ADC_STEPS_PER_NOTE = 8.5 which will reult in a rounding error pretty quick
	 * so we use ADC_STEPS_PER_NOTE * 2 = 17
	 * we shift the result up by ~ADC_STEPS_PER_NOTE/2 = 8
	 * to bring the note values (played by keyboard fr example) in the middle of a step, 
	 * thus increasing the note tracking over several octaves
	 */
	uint16_t quantValue = ((input<<1)+8)/17;//ADC_STEPS_PER_NOTE;

	//calculate the current active step
	uint8_t octave = quantValue/12;
	uint8_t note = quantValue-(octave*12);

	
	//quantize to active steps
	//search for the lowest matching activated note (lit led)
	int i=0;
	for(;i<13;i++)
	{
	  if( ((1<<  ((note+i)%12) ) & activeSteps) != 0) break;
	}
	
	note = note+i;
	if(note>=12)
	{
	  note -= 12;
	  octave++;
	}
	
	quantValue = octave*12+note;
	
	//store to matrix
	io_setCurrentQuantizedValue(note);
	return quantValue*2;
}
//-----------------------------------------------------------
int main(void)
{
    init();

    //read last button state from eeprom
    io_setActiveSteps( eeprom_ReadBuffer());

    while(1)
    {
      //handle IOs (buttons + LED)		
      io_processButtonsPipelined();
      io_processLedPipelined();

      checkAutosave();

      if(!GATE_IN_CONNECTED)
      {
        //no gate cable plugged in, run in continuous mode.
        //seems safer to disable interrupts in case cable is plugged?
        ATOMIC_BLOCK(ATOMIC_FORCEON)
        {
            process();
        }
      }
    }
}
//-----------------------------------------------------------
