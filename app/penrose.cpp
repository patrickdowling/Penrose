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

#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <avr/wdt.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "drivers/adc.h"
#include "drivers/eeprom.h"
#include "drivers/dac.h"
#include "drivers/profiling.h"
#include "drivers/led.h"
#include "drivers/button.h"
#include "drivers/gate.h"

#include "app/timeout_counter.h"
#include "app/filter.h"
#include "app/pitch.h"
#include "app/quantizer.h"

namespace penrose
{

static constexpr uint32_t kSampleRate = 8000;
static constexpr uint32_t kFilterCutoff = 1000;
static constexpr uint32_t kGateLength = 12; // ms
static constexpr uint32_t kAutosaveTime = 15000; // ms
static constexpr uint32_t kGateTimeout = kGateLength * kSampleRate / 1000;
static constexpr uint32_t kAutosaveTimeout = kAutosaveTime * kSampleRate / 1000;
static constexpr int16_t kDeadbandThreshold = pitch::kLSBsPerSemitone / 2;
static constexpr uint8_t kDACLSBsPerSemitone = 2;

static OnePoleLowpass<kFilterCutoff, kSampleRate> sample_filter;

static_assert(kGateTimeout <= UINT16_MAX, "Gate timer overflow");
static TimeoutCounter<uint16_t, kGateTimeout> gate_counter;

static_assert(kAutosaveTimeout <= (UINT32_MAX >> 8), "Autosave timer overflow");
static TimeoutCounter<__uint24, kAutosaveTimeout> autosave_counter;

static volatile bool autosave_flag;
static uint16_t prev_adc_code = 0;
static uint8_t prev_dac_code = 0;

static uint8_t ProcessQuantizer(uint16_t enabled_notes, uint16_t adc_code)
{
    if (abs(adc_code - prev_adc_code) < kDeadbandThreshold)
    {
        adc_code = prev_adc_code;
    }
    else
    {
        prev_adc_code = adc_code;
    }

    return Quantize(enabled_notes, adc_code);
}

static uint16_t ProcessUI(void)
{
    bool touch = button::Scan();
    uint16_t buttons = button::GetButtonStates();
    led::SetLEDs(buttons);
    led::Scan();

    if (touch)
    {
        autosave_counter.Start();
        autosave_flag = false;
    }

    if (autosave_counter.Tick())
    {
        autosave_flag = true;
    }

    return buttons;
}

void ADCCallback(uint16_t sample)
{
    prof::set(prof::SAMPLE_INTERRUPT);

    uint16_t buttons = ProcessUI();

    sample = sample_filter.Process(sample);

    if (!gate::Detect() || gate::Triggered())
    {
        prof::set(prof::DSP_PROCESS);

        uint8_t pitch;

        if (buttons == 0)
        {
            pitch = 0;
            led::SetActive(-1);
        }
        else
        {
            pitch = ProcessQuantizer(buttons, sample);
            led::SetActive(pitch::Note(pitch));
        }

        uint8_t dac_code = pitch * kDACLSBsPerSemitone;

        if (dac_code != prev_dac_code)
        {
            dac::BeginWrite(dac_code);
            gate_counter.Start();
            prev_dac_code = dac_code;
        }

        prof::clear(prof::DSP_PROCESS);
    }

    if (gate_counter.Tick())
    {
        dac::FinishWrite();
    }

    prof::clear(prof::SAMPLE_INTERRUPT);
}

static void Init(void)
{
    cli();
    wdt_disable();

    // Disable analog comparator
    ACSR |= _BV(ACD);

    dac::Init();
    adc::Init(F_CPU / kSampleRate, ADCCallback);
    led::Init();
    button::Init();
    gate::Init();
    prof::init();

    // Restore saved settings
    button::SetButtonStates(eeprom::ReadBuffer());

    gate_counter.Init();
    autosave_counter.Init();
    autosave_flag = false;
    sample_filter.Init();

    sei();
}

extern "C"
int main(void)
{
    Init();

    while (1)
    {
        while (!autosave_flag);
        eeprom::WriteBuffer(button::GetButtonStates());
        autosave_flag = false;
    }
}


}
