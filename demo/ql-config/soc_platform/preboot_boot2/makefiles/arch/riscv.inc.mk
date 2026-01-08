# Target triple for the build. Use riscv-none-elf if you are unsure.
export TARGET_ARCH ?= riscv-none-elf

export CFLAGS_LINK  = -ffunction-sections -fdata-sections -fno-builtin -fshort-enums
export CFLAGS_DBG  ?= -ggdb -g
export CFLAGS_OPT  ?= -Os
export CPU_ARCH = riscv

export CFLAGS += $(CFLAGS_CPU) $(CFLAGS_LINK) $(CFLAGS_DBG) $(CFLAGS_OPT)

export ASFLAGS += $(CFLAGS_CPU) $(CFLAGS_DBG)
export LINKER_SCRIPT ?= $(LDSCRIPT)
export LINKFLAGS += -T$(LINKER_SCRIPT) -Wl,--fatal-warnings

export LINKFLAGS += $(CFLAGS_CPU) $(CFLAGS_DBG) $(CFLAGS_OPT) -static -lgcc -nostartfiles
export LINKFLAGS += -Wl,--gc-sections -Wl,--no-warn-rwx-segments -Wl,--no-warn-mismatch

# all cortex MCU's use newlib as libc
export USEMODULE += newlib

# set default for CPU_MODEL
export CPU_MODEL ?= $(CPU)

# export the CPU model and architecture
MODEL = $(shell echo $(CPU_MODEL) | tr 'a-z' 'A-Z')
export CFLAGS += -DCPU_MODEL_$(MODEL)
ARCH = $(shell echo $(CPU_ARCH) | tr 'a-z-+' 'A-Z__')
export CFLAGS += -DCPU_ARCH_$(ARCH)

# set the compiler specific CPU and FPU options
export MCPU ?= $(CPU_ARCH)

export SOURCE_LDSCRIPT = $(RIOTCPU)/$(CPU)/ldscripts/$(LINKER_SOURCE_SCRIPT)
$(inifo $(SOURCE_LDSCRIPT))
export LDSCRIPT = $(BINDIR)/$(CPU_MODEL).ld

# use the nano-specs of Newlib when available
USEMODULE += newlib_nano
export USE_NANO_SPECS = 1

# Avoid overriding the default rule:
all:

# Rule to generate assembly listings from ELF files:
%.lst: %.elf
	$(OBJDUMP) $(OBJDUMPFLAGS) $< > $@

### This rule is used to generate the correct linker script
LDGENFLAGS += $(CFLAGS)
LDGENFLAGS += -x c -P -E

# NB: Assumes LDSCRIPT was not overridden and is in $(OBJECTDIR)
$(LDSCRIPT): $(SOURCE_LDSCRIPT) $(RIOTBUILD_CONFIG_HEADER_C) | $(BINDIR)
	$(Q)$(CC) $(LDGENFLAGS) $< | grep -v '^\s*#\s*pragma\>' > $@
