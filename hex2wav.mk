# Copyright 2019 Tyler Coy
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

TARGET := $(HEX2WAV_ELF)
SOURCES := hex2wav/*.cpp

T_ELF := $(TARGET_DIR)/$(TARGET)
T_MAP := $(TARGET_DIR)/$(TARGET:.elf=.map)
T_HEX := $(TARGET_DIR)/$(TARGET:.elf=.hex)

TGT_CC := gcc
TGT_CXX := g++

TGT_DEFS :=

TGT_INCDIRS := ./hex2wav

ARCHFLAGS :=
WARNFLAGS := -Wall -Wextra -Wundef
OPTFLAGS := \
	-funsigned-char \
	-funsigned-bitfields \
	-ffast-math \
	-freciprocal-math \
	-ffunction-sections \
	-fdata-sections \
	-fshort-enums \

TGT_CFLAGS := -O0 -ggdb $(ARCHFLAGS) $(OPTFLAGS) $(WARNFLAGS) -std=gnu11
TGT_CXXFLAGS := -O0 -ggdb $(ARCHFLAGS) $(OPTFLAGS) $(WARNFLAGS) -std=gnu++17

TGT_LDLIBS := -lm -lc -lgcc
TGT_LDFLAGS := $(ARCHFLAGS) -L$(TARGET_DIR) \
	-Wl,--gc-sections \
	-Wl,-Map=$(T_MAP) \

define TGT_POSTMAKE
endef

define TGT_POSTCLEAN
	$(RM) $(T_MAP)
endef
