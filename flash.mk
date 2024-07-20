# Author: Liwei Xue 2024.3.20
# Flash script for makefile project, using OPENOCD.

OCDSCRIPT_PATH = $(subst \,/,$(OPENOCD_HOME))/share/openocd/scripts
#On windows "\" in file path will cause problems in OPENOCD

PROJECT = supercapv4
TARGET = stm32g4x
INTERFACE = stlink-v2

OCD = openocd
ELF = build/$(PROJECT).elf

default: load

load: build_code
	$(OCD) -f $(OCDSCRIPT_PATH)/interface/$(INTERFACE).cfg \
	 -f  $(OCDSCRIPT_PATH)/target/$(TARGET).cfg \
	 -c "program $(ELF) verify reset exit"

build_code:
	make -C ./ -j

showpath:
	@echo $(OPENOCD_HOME)
	@echo $(OCDSCRIPT_PATH)

.PHONY: build_code load
