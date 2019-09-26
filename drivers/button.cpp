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

#include "drivers/button.h"

#include <avr/io.h>
#include <util/delay.h>

#include "drivers/profiling.h"

#define COL0_PIN        PD0
#define COL1_PIN        PD1
#define COL2_PIN        PD2
#define COL3_PIN        PD3

#define COL_PORT        PORTD
#define COL_DDR         DDRD
#define COL_INPUT       PIND

#define ROW0_PIN        PB2
#define ROW1_PIN        PB4
#define ROW2_PIN        PC5

#define ROW0_DDR        DDRB
#define ROW1_DDR        DDRB
#define ROW2_DDR        DDRC

#define ROW0_PORT       PORTB
#define ROW1_PORT       PORTB
#define ROW2_PORT       PORTC

namespace penrose {
namespace button {

static constexpr uint8_t kNumColumns = 4;
static constexpr uint8_t kNumRows = 3;

static uint8_t column_;
static uint8_t row_;
static uint16_t toggle_state_;
static uint16_t press_state_;

static void AllRowsOff(void)
{
    // Drive row pins high
    ROW0_PORT |= _BV(ROW0_PIN);
    ROW1_PORT |= _BV(ROW1_PIN);
    ROW2_PORT |= _BV(ROW2_PIN);
}

static void RowOn(uint8_t row)
{
    // Drive selected row low
    switch (row)
    {
        case 0: ROW0_PORT &= ~_BV(ROW0_PIN); break;
        case 1: ROW1_PORT &= ~_BV(ROW1_PIN); break;
        case 2: ROW2_PORT &= ~_BV(ROW2_PIN); break;
    }
}

void Init(void)
{
    // Set column pins as input with pullup
    COL_DDR &= ~(_BV(COL0_PIN) | _BV(COL1_PIN) | _BV(COL2_PIN) | _BV(COL3_PIN));
    COL_PORT |= (_BV(COL0_PIN) | _BV(COL1_PIN) | _BV(COL2_PIN) | _BV(COL3_PIN));

    ROW0_DDR |= _BV(ROW0_PIN);
    ROW1_DDR |= _BV(ROW1_PIN);
    ROW2_DDR |= _BV(ROW2_PIN);

    AllRowsOff();

    column_ = 0;
    row_ = 0;
    toggle_state_ = 0;
    press_state_ = 0;
}

void SetButtonStates(uint16_t bitpacked_states)
{
    toggle_state_ = bitpacked_states;
}

uint16_t GetButtonStates(void)
{
    return toggle_state_;
}

bool Scan(void)
{
    prof::set(prof::BUTTON_SCAN);

    RowOn(row_);
    _delay_us(1);
    uint8_t pins = COL_INPUT;
    AllRowsOff();

    uint16_t button_bit = (~pins & _BV(column_)) << (kNumColumns * row_);
    uint8_t button_number = column_ + kNumColumns * row_;

    bool change = button_bit != (press_state_ & _BV(button_number));
    bool press = button_bit && !(press_state_ & _BV(button_number));

    if (change)
    {
        press_state_ ^= _BV(button_number);

        if (press)
        {
            toggle_state_ ^= _BV(button_number);
        }
    }

    column_++;

    if (column_ >= kNumColumns)
    {
        column_ = 0;
        row_++;

        if (row_ >= kNumRows)
        {
            row_ = 0;
        }
    }

    prof::clear(prof::BUTTON_SCAN);

    return press;
}

}
}
