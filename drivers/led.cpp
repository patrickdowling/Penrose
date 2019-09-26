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

#include "drivers/led.h"

#include <avr/pgmspace.h>

#include "drivers/profiling.h"

namespace penrose {
namespace led {

struct Pin
{
    volatile uint8_t* port;
    volatile uint8_t* ddr;
    uint8_t pin_number;
} __attribute__ ((aligned (2)));

struct CharlieplexPair
{
    Pin pin_a;
    Pin pin_b;
};

static constexpr Pin kLEDPin1 = { &PORTC, &DDRC, 1 };
static constexpr Pin kLEDPin2 = { &PORTC, &DDRC, 2 };
static constexpr Pin kLEDPin3 = { &PORTC, &DDRC, 3 };
static constexpr Pin kLEDPin4 = { &PORTD, &DDRD, 4 };
static constexpr Pin kLEDPin5 = { &PORTD, &DDRD, 5 };
static constexpr Pin kLEDPin6 = { &PORTD, &DDRD, 6 };
static constexpr uint8_t kNumLEDs = 12;

static const CharlieplexPair kLEDPinTable[kNumLEDs] =
{
    { kLEDPin1, kLEDPin2 },
    { kLEDPin1, kLEDPin3 },
    { kLEDPin1, kLEDPin4 },
    { kLEDPin2, kLEDPin3 },
    { kLEDPin2, kLEDPin4 },
    { kLEDPin2, kLEDPin5 },
    { kLEDPin3, kLEDPin4 },
    { kLEDPin3, kLEDPin5 },
    { kLEDPin3, kLEDPin6 },
    { kLEDPin4, kLEDPin5 },
    { kLEDPin4, kLEDPin6 },
    { kLEDPin5, kLEDPin6 },
};

static int8_t active_;
static uint8_t current_;
static uint16_t states_;

static void TurnAllOff(void)
{
    // All pins low / no pullup
    PORTC &= ~(_BV(PC1) | _BV(PC2) | _BV(PC3));
    PORTD &= ~(_BV(PD4) | _BV(PD5) | _BV(PD6));

    // All LED pins as inputs
    DDRC &= ~(_BV(PC1) | _BV(PC2) | _BV(PC3));
    DDRD &= ~(_BV(PD4) | _BV(PD5) | _BV(PD6));
}

static void TurnOn(uint8_t led_number, bool reverse)
{
    Pin pin_a = kLEDPinTable[led_number].pin_a;
    Pin pin_b = kLEDPinTable[led_number].pin_b;

    // Set pins as outputs
    *pin_a.ddr |= _BV(pin_a.pin_number);
    *pin_b.ddr |= _BV(pin_b.pin_number);

    // Assuming all pins are currently low
    if (reverse)
    {
        // Set pin A high
        *pin_a.port |= _BV(pin_a.pin_number);
    }
    else
    {
        // Set pin B high
        *pin_b.port |= _BV(pin_b.pin_number);
    }
}

void Init(void)
{
    TurnAllOff();

    active_ = -1;
    current_ = 0;
    states_ = 0xFFF;
}

void SetLEDs(uint16_t bitpacked_states)
{
    states_ = bitpacked_states;
}

void SetActive(int16_t led_number)
{
    active_ = led_number;
}

void Scan(void)
{
    prof::set(prof::LED_SCAN);

    TurnAllOff();

    if (states_ & (1 << current_))
    {
        TurnOn(current_, (current_ == active_));
    }

    current_++;

    if (current_ >= kNumLEDs)
    {
        current_ = 0;
    }

    prof::clear(prof::LED_SCAN);
}

}
}
