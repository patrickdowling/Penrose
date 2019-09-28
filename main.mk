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

BUILD_DIR := build
TARGET_DIR := $(BUILD_DIR)/artifact

DEFS :=

RELEASE_VERSION := 0.1.1

# ------------------------------------------------------------------------------
# Artifacts
# ------------------------------------------------------------------------------

SUBMAKEFILES := app.mk bootloader.mk hex2wav.mk test.mk artifact.mk
APP_ELF := app.elf
BL_ELF := bootloader.elf
MERGE_HEX := penrose.hex
HEX2WAV_ELF := hex2wav.elf
DIST_ZIP := penrose-$(RELEASE_VERSION).zip

.DEFAULT_GOAL := app

%.bin: %.hex
	avr-objcopy --gap-fill 0xFF -O binary -I ihex $^ $@

%.wav: $(TARGET_DIR)/$(HEX2WAV_ELF) %.hex
	$^ $@

%.hex: %.elf
	avr-objcopy -O ihex -R .eeprom -R .fuse -R .lock -R .signature $^ $@

%.lss: %.elf
	avr-objdump -CdhtS $^ > $@

define ANALYZE
@$(MAKE) $(1:.elf=.lss)
@echo 'BINARY SIZE:' \
	$$(avr-size $(1) | tail -n1 | \
	awk '{ printf "0x%05X %d", $$1+$$2, $$1+$$2 }')
@echo 'RAM USAGE:  ' \
	$$(avr-size $(1) | tail -n1 | \
	awk '{ printf "0x%05X %d", $$2+$$3, $$2+$$3 }')
endef

define CLEAN_ANALYSIS
$(RM) $(1:.elf=.lss)
endef

.PHONY: sym
sym: $(TARGET_DIR)/$(APP_ELF)
	avr-nm -CnS $< | less

.PHONY: top
top: $(TARGET_DIR)/$(APP_ELF)
	avr-nm -CrS --size-sort $< | less

.PHONY: app
app: $(TARGET_DIR)/$(APP_ELF)

.PHONY: bl
bl: $(TARGET_DIR)/$(BL_ELF)

.PHONY: hex2wav
hex2wav: $(TARGET_DIR)/$(HEX2WAV_ELF)

.PHONY: cog
cog: app-cog
