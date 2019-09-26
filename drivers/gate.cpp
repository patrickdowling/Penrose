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

#include "drivers/gate.h"

#include <avr/io.h>

#include "drivers/gpio.h"

namespace penrose {
namespace gate {

gpio::InputPin<gpio::PORT_D, 7> gate_;
gpio::InputPin<gpio::PORT_C, 4> detect_;
bool state_;

void Init(void)
{
    gate_.Init(false);
    detect_.Init(true);
    state_ = false;
}

bool Triggered(void)
{
    bool prev = state_;
    state_ = !gate_.Read();
    return state_ && !prev;
}

bool Detect(void)
{
    return !detect_.Read();
}

}
}
