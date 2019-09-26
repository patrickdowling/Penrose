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

#ifndef PENROSE_APP_FILTER_H_
#define PENROSE_APP_FILTER_H_

#include <stdint.h>
#include <math.h>

namespace penrose
{

template <uint32_t cutoff, uint32_t sample_rate>
class OnePoleLowpass
{
  public:
    void Init(void)
    {
        history_q8_ = 0;
    }

    int16_t Process(int16_t sample)
    {
        __int24 sample_q8 = static_cast<__int24>(sample) << 8;
        int32_t delta_q16 = kFactorQ8 * (sample_q8 - history_q8_);
        history_q8_ += delta_q16 >> 8;
        return (history_q8_ + 128) >> 8;
    }

  protected:
    static constexpr float kFactor = 1 - exp(-2 * M_PI * cutoff / sample_rate);
    static constexpr int32_t kFactorQ8 = round(kFactor * 256);
    __int24 history_q8_;
};

}

#endif
