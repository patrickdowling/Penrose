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

#include "drivers/dac.h"

#include "drivers/spi.h"
#include "drivers/gpio.h"
#include "drivers/profiling.h"

#define DAC_A               0
#define DAC_B               1

#define GAIN_X2             0
#define GAIN_X1             1

#define CHANNEL_ACTIVE          1
#define CHANNEL_SHUTDOWN        0

namespace penrose {
namespace dac {

static gpio::OutputPin<gpio::PORT_B, 1> ldac_;

void Init(void)
{
    spi::Init(false);
    /* SPI is not enabled here
     * it is just enabled before sending out data to the DAC
     * since the MISO and SS pins are also used for the button matrix
     * a continuous activated SPI port would conflic with the matrix and break functionality
     * */

    ldac_.Init(gpio::HIGH);
};

/*
The write command is initiated by driving the CS pin
low, followed by clocking the four Configuration bits and
the 12 data bits into the SDI pin on the rising edge of
SCK. The CSpin is then raised, causing the data to be
latched into the selected DACs input registers.

bit 15 A/B:DAC A or DAC B Selection bit
1= Write to DACB
0= Write to DACA

bit 14  Don't Care

bit 13 GA:Output Gain Selection bit
1=1x (VOUT= VREF* D/4096)
0=2x (VOUT= 2 * VREF* D/4096), where internal VREF= 2.048V.

bit 12 SHDN:Output Shutdown Control bit
1= Active mode operation. VOUTis available. ?
0= Shutdown the selected DAC channel. Analog output is not available at the channel that was shut down.
VOUTpin is connected to 500 k???typical)?

bit 11-0 D11:D0:DAC Input Data bits. Bit x is ignored.
*/
void BeginWrite(uint8_t code)
{
    prof::set(prof::DAC_WRITE);

    ldac_.Set();

    //data for output A
    uint16_t data = (DAC_A << 15) |
                    (GAIN_X2 << 13) |
                    (CHANNEL_ACTIVE << 12) |
                    (code << 4);

    spi::Enable();
    spi::TransmitWord(data);
    spi::Disable();

    ldac_.Clear();

    prof::clear(prof::DAC_WRITE);
};

void FinishWrite(void)
{
    ldac_.Set();
}

}
}
