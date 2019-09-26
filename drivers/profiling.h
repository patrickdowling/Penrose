/*
 *  Copyright 2019 Tyler Coy
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.

 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.

 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

// Use pin C6 as a profiling output. C6 is the reset pin so the RSTDISBL fuse
// must be programmed in order to use it as an IO pin. However, this will
// also disable the programming interface.

#ifndef PENROSE_DRIVERS_PROFILING_H_
#define PENROSE_DRIVERS_PROFILING_H_

#include <stdint.h>
#include <avr/io.h>

namespace penrose {
namespace prof {

enum ProfilingPin
{
    SAMPLE_INTERRUPT,
    DSP_PROCESS,
    EEPROM_WRITE,
    BUTTON_SCAN,
    LED_SCAN,
    DAC_WRITE,
    ADC_READ,
};

inline void init(void)
{
    #ifndef NDEBUG

    DDRC |= 1 << 6;
    PORTC &= ~0x40;

    #endif
}

inline void set(ProfilingPin pin)
{
    #ifndef NDEBUG

    if (pin == 0)
    {
        PORTC |= 0x40;
    }

    #else
    (void)pin;
    #endif
}

inline void clear(ProfilingPin pin)
{
    #ifndef NDEBUG

    if (pin == 0)
    {
        PORTC &= ~0x40;
    }

    #else
    (void)pin;
    #endif
}

}
}

#endif
