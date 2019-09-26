/*
 *  Copyright 2015 Julian Schmidt, Sonic Potions <julian@sonic-potions.com>
 *  Web: www.sonic-potions.com/penrose
 *
 *  Copyright 2019 Tyler Coy
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

#include "drivers/adc.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include "drivers/profiling.h"

namespace penrose {
namespace adc {

static Callback callback_;

void Init(uint16_t sample_period, Callback callback)
{
    callback_ = callback;

    // Set PC0 as input, without pullup.
    DDRC &= ~_BV(PC0);
    PORTC &= ~_BV(PC0);

    // Use AVCC as the reference voltage and select ADC channel 0.
    ADMUX = _BV(REFS0);

    // The ADC clock must be between 50 kHz and 200 kHz.
    // Our system clock is 20MHz, so prescale by 128.
    // This gives us an ADC clock of 156.25 kHz.
    ADCSRA = _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0);

    // ADC enable
    ADCSRA |= _BV(ADEN);

    // Do the first conversion now. It takes longer than normal because it
    // performs some initialization.
    Convert();

    // Select timer1 compare match B as the ADC auto trigger source.
    ADCSRB = _BV(ADTS2) | _BV(ADTS0);

    // Enable auto trigger, clear interrupt flag, and enable the interrupt.
    ADCSRA |= _BV(ADIF) | _BV(ADATE) | _BV(ADIE);

    // Set timer1 to CTC mode and set OCA to the sample period
    TCCR1A = 0;
    TCCR1B = _BV(WGM12) | _BV(CS10);
    TCCR1C = 0;
    OCR1A = sample_period;
    OCR1B = 0;
    TIMSK1 = 0;
    TIFR1 = _BV(OCF1B);
};

uint16_t Convert(void)
{
    StartConversion();
    return GetResult();
};

void StartConversion(void)
{
    // Start a conversion.
    ADCSRA |= _BV(ADSC);

    prof::set(prof::ADC_READ);
}

uint16_t GetResult(void)
{
    // Wait for the conversion to finish.
    while (ADCSRA & _BV(ADSC));

    prof::clear(prof::ADC_READ);

    return ADCW;
}

ISR(ADC_vect)
{
    TIFR1 = _BV(OCF1B);
    callback_(ADCW);
}

}
}
