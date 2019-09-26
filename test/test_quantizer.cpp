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

#include <gtest/gtest.h>
#include <cstdint>
#include <cmath>
#include "app/quantizer.h"

namespace penrose::test
{

constexpr uint32_t kNumFractionalBits = 8;

constexpr double kVRef = 5.0;
constexpr double kCVInGain = 49.9 / 100.0;
constexpr double kVoltsPerLSB = kVRef / kCVInGain / 1024;

static constexpr uint16_t GetADCCode(double volts)
{
    int32_t code = std::round(volts / kVoltsPerLSB);
    return std::max(0, std::min(1023, code));
}

static constexpr double GetVolts(uint16_t adc_code)
{
    return adc_code * kVoltsPerLSB;
}

constexpr int32_t kMaxSemitone = std::floor(GetVolts(1023) * 12);

class QuantizerTest : public ::testing::Test
{
  protected:
};

TEST_F(QuantizerTest, Correct)
{
    for (uint32_t note_mask = 1; note_mask < 0x1000; note_mask++)
    {
        for (uint32_t adc_code = 0; adc_code < 1024; adc_code++)
        {
            float volts = GetVolts(adc_code);
            double min_distance = INT32_MAX;
            int32_t nearest_semitone = -1;

            for (int32_t i = 0; i <= kMaxSemitone; i++)
            {
                if (note_mask & (1 << (i % 12)))
                {
                    double semitone_volts = i / 12.0;
                    double distance = std::fabs(semitone_volts - volts);

                    if (distance < min_distance)
                    {
                        min_distance = distance;
                        nearest_semitone = i;
                    }
                }
            }

            int32_t actual = Quantize(note_mask, adc_code);
            int32_t expected = nearest_semitone;

            // Since the quantizer uses fixed-point arithmetic, sometimes it
            // will be off by an octave because of rounding error. So instead
            // of comparing the expected and the actual values directly, we
            // compare their differences from the unquantized value and make
            // sure they are within one fixed-point LSB of each other.
            ASSERT_NEAR(std::abs(expected - volts * 12),
                        std::abs(actual   - volts * 12),
                        1.0 / (1 << kNumFractionalBits))
                << std::hex << note_mask << ", " << std::dec << adc_code
                << ", " << actual << ", " << expected
                << ", " << volts << ", " << (volts * 12);
        }
    }
}

TEST_F(QuantizerTest, Nearest)
{
    for (int32_t i = 0; i < 1024; i++)
    {
        double volts = GetVolts(i);
        int32_t expected = std::round(volts * 12);

        int32_t actual = Quantize(0xFFF, i);

        ASSERT_EQ(i, GetADCCode(volts));
        ASSERT_EQ(expected, actual)
            << i << ", " << volts << ", " << (volts * 12);
    }
}

TEST_F(QuantizerTest, Monotonic)
{
    int32_t prev_pitch = 0;

    for (int32_t i = 0; i < 1024; i++)
    {
        int32_t pitch = Quantize(0xFFF, i);
        ASSERT_GE(pitch, prev_pitch)
            << i << ", " << prev_pitch << ", " << pitch;
        prev_pitch = pitch;
    }

    for (int32_t i = 1023; i >= 0; i--)
    {
        int32_t pitch = Quantize(0xFFF, i);
        ASSERT_LE(pitch, prev_pitch)
            << i << ", " << prev_pitch << ", " << pitch;
        prev_pitch = pitch;
    }
}

TEST_F(QuantizerTest, Stable)
{
    for (int32_t i = 0; i < 1024; i++)
    {
        int32_t prev_pitch;

        for (int32_t j = 0; j < 256; j++)
        {
            int32_t pitch = Quantize(0xFFF, i);

            if (j > 0)
            {
                ASSERT_EQ(pitch, prev_pitch);
            }

            prev_pitch = pitch;
        }
    }
}

}
