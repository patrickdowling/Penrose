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

#ifndef PENROSE_APP_QUANTIZER_H_
#define PENROSE_APP_QUANTIZER_H_

#include <stdint.h>
#include "app/pitch.h"

namespace penrose
{

inline uint8_t Quantize(uint16_t enabled_notes, uint16_t adc_code)
{
    int16_t semitones_q8 = pitch::Lookup(adc_code);
    int16_t octave_q8 = pitch::Octave(semitones_q8 >> 8);
    octave_q8 <<= 8;
    int16_t base_q8 = octave_q8 * 12;

    // Find nearest enabled note.
    int16_t min_q8 = 0xC00;
    int16_t nearest_q8 = 0;

    for (int8_t i = 0; i < 12; i++)
    {
        if (enabled_notes & 1)
        {
            __int24 candidate_q8 = 0x100 * i + base_q8;

            if (candidate_q8 > (pitch::kMaxSemitone << 8))
            {
                candidate_q8 -= 0xC00;
            }

            int16_t delta_q8 = candidate_q8 - semitones_q8;

            if (delta_q8 < -0x600)
            {
                if (candidate_q8 <= (pitch::kMaxSemitone << 8) - 0xC00)
                {
                    delta_q8 += 0xC00;
                }
            }
            else if (delta_q8 > 0x600)
            {
                if (candidate_q8 >= 0xC00)
                {
                    delta_q8 -= 0xC00;
                }
            }

            int16_t distance_q8 = abs(delta_q8);

            if (distance_q8 < min_q8)
            {
                min_q8 = distance_q8;
                nearest_q8 = delta_q8;
            }
        }

        enabled_notes >>= 1;
    }

    return (semitones_q8 + nearest_q8) >> 8;
}

}

#endif
