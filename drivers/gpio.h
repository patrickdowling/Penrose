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

#ifndef PENROSE_DRIVERS_GPIO_H_
#define PENROSE_DRIVERS_GPIO_H_

#include <stdint.h>
#include <avr/io.h>

namespace penrose {
namespace gpio {

enum Port
{
    PORT_B,
    PORT_C,
    PORT_D,
};

enum PinState
{
    LOW,
    HIGH,
};

inline volatile uint8_t& PORTx(Port port)
{
    switch (port)
    {
    default:
    case PORT_B: return PORTB;
    case PORT_C: return PORTC;
    case PORT_D: return PORTD;
    }
}

inline volatile uint8_t& DDRx(Port port)
{
    switch (port)
    {
    default:
    case PORT_B: return DDRB;
    case PORT_C: return DDRC;
    case PORT_D: return DDRD;
    }
}

inline volatile uint8_t& PINx(Port port)
{
    switch (port)
    {
    default:
    case PORT_B: return PINB;
    case PORT_C: return PINC;
    case PORT_D: return PIND;
    }
}

template<Port kPort, uint8_t kPinNumber>
class OutputPin
{
  public:
    void Init(PinState initial_state = LOW)
    {
        DDRx(kPort) |= _BV(kPinNumber);
        (initial_state == HIGH) ? Set() : Clear();
    }

    void Set(void)
    {
        PORTx(kPort) |= _BV(kPinNumber);
    }

    void Clear(void)
    {
        PORTx(kPort) &= ~_BV(kPinNumber);
    }

    void Toggle(void)
    {
        PINx(kPort) |= _BV(kPinNumber);
    }
};

template<Port kPort, uint8_t kPinNumber>
class InputPin
{
  public:
    void Init(bool pullup = false)
    {
        DDRx(kPort) &= ~_BV(kPinNumber);

        if (pullup)
        {
            PORTx(kPort) |= _BV(kPinNumber);
        }
        else
        {
            PORTx(kPort) &= ~_BV(kPinNumber);
        }
    }

    bool Read(void)
    {
        return PINx(kPort) & _BV(kPinNumber);
    }
};

}
}

#endif
