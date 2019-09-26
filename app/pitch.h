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

#ifndef PENROSE_APP_PITCH_H_
#define PENROSE_APP_PITCH_H_

#include <stdint.h>
#include <math.h>
#ifdef __AVR__
#include <avr/pgmspace.h>
#else
#define PROGMEM
#endif

namespace penrose {
namespace pitch {

constexpr float kVRef = 5.0;
constexpr float kCVInGain = 49.9 / 100.0;
constexpr float kVoltsPerLSB = kVRef / kCVInGain / 1024;
constexpr float kLSBsPerSemitone = 1 / (kVoltsPerLSB * 12);
constexpr int16_t kMaxSemitone = floor(1023 * kVoltsPerLSB * 12);

extern const int16_t PitchLookup[] PROGMEM;
extern const int8_t OctaveLookup[] PROGMEM;
extern const int8_t NoteLookup[] PROGMEM;

// Returns an 8.8 fixed point representation of the number of semitones
// corresponding to the given 10-bit ADC sample.
inline int16_t Lookup(uint16_t adc_code)
{
  #ifdef __AVR__
    return pgm_read_word((uint16_t)(&PitchLookup[adc_code]));
  #else
    return PitchLookup[adc_code];
  #endif
}

inline int8_t Octave(int8_t pitch)
{
  #ifdef __AVR__
    return pgm_read_byte((uint16_t)(&OctaveLookup[pitch]));
  #else
    return OctaveLookup[pitch];
  #endif
}

inline int8_t Note(int8_t pitch)
{
  #ifdef __AVR__
    return pgm_read_byte((uint16_t)(&NoteLookup[pitch]));
  #else
    return NoteLookup[pitch];
  #endif
}

}
}

#endif
