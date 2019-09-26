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

TARGET := $(APP_ELF)
SOURCES := app/*.cpp drivers/*.cpp

T_ELF := $(TARGET_DIR)/$(TARGET)
T_MAP := $(TARGET_DIR)/$(TARGET:.elf=.map)

TGT_CC := avr-gcc
TGT_CXX := avr-g++

TGT_DEFS := \
	F_CPU=20000000UL \
	__PROG_TYPES_COMPAT__ \

TGT_INCDIRS := .

ARCHFLAGS := -mmcu=atmega168
WARNFLAGS := -Wall -Wextra -Wundef
OPTFLAGS := \
	-funsigned-char \
	-funsigned-bitfields \
	-ffast-math \
	-freciprocal-math \
	-ffunction-sections \
	-fdata-sections \
	-fpack-struct \
	-fshort-enums \
	-finline-functions-called-once \
	-finline-functions \

TGT_CFLAGS := -O3 -g $(ARCHFLAGS) $(OPTFLAGS) $(WARNFLAGS) -std=gnu11
TGT_CXXFLAGS := -O3 -g $(ARCHFLAGS) $(OPTFLAGS) $(WARNFLAGS) -std=gnu++14 \
	-fno-exceptions -fno-rtti

TGT_LDLIBS := -lm -lc -lgcc
TGT_LDFLAGS := $(ARCHFLAGS) -L$(TARGET_DIR) \
	-Wl,--gc-sections \
	-Wl,-Map=$(T_MAP) \

define TGT_POSTMAKE
	$(call ANALYZE,$(T_ELF))
endef

define TGT_POSTCLEAN
	$(RM) $(T_MAP)
	$(call CLEAN_ANALYSIS,$(T_ELF))
endef

.PHONY: app-cog
app-cog:
	cog -r app/pitch.cpp
