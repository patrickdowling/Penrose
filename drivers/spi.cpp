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

#include "drivers/spi.h"

#include <avr/io.h>

#include "drivers/gpio.h"

#define SPI_PORT    PORTB
#define SPI_DDR     DDRB
#define MOSI_PIN    PB3
#define SCK_PIN     PB5

namespace penrose {
namespace spi {

static gpio::OutputPin<gpio::PORT_B, 0> chip_select_;

void Init(bool enable)
{
    SPI_DDR |= _BV(MOSI_PIN) | _BV(SCK_PIN);

    chip_select_.Init(gpio::HIGH);

    // Set SPI clock as fast as possible. FCPU * 2 / 4 = 10MHz
    SPCR = _BV(MSTR);
    SPSR = _BV(SPI2X);

    if (enable)
    {
        Enable();
    }
};

void TransmitByte(uint8_t data)
{
    chip_select_.Clear();

    SPDR = data;
    while (!(SPSR & _BV(SPIF)));

    chip_select_.Set();
}

void TransmitWord(uint16_t data)
{
    chip_select_.Clear();

    SPDR = data >> 8;
    while (!(SPSR & _BV(SPIF)));

    SPDR = data & 0xFF;
    while (!(SPSR & _BV(SPIF)));

    chip_select_.Set();
}

void Enable(void)
{
    SPCR |= _BV(SPE);
}

void Disable(void)
{
    SPCR &= ~_BV(SPE);
}

}
}
