# name of your application
APPLICATION = boot2
DISABLE_UART_TRY_DOWNLOAD ?= 1
LOG_CONF_LEVEL_MAIN ?= LOG_LEVEL_INFO

# If no BOARD is found in the environment, use this default:
BOARD ?= cortexr-arom-crane

# If no AROMVERSION is found in the environment, use this default:
export AROMVERSION ?= 2019.01.15

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../../

BOARD_WHITELIST += cortexr-arom-crane

# Include all submodules depending on
USEMODULE += libarom
USEMODULE += ptable
USEMODULE += crc32
USEMODULE += property
USEMODULE += log
USEMODULE += patchlib

CFLAGS += -DDISABLE_UART_TRY_DOWNLOAD=$(DISABLE_UART_TRY_DOWNLOAD)
CFLAGS += -Wno-int-conversion
CFLAGS += -DLOG_CONF_LEVEL_MAIN=$(LOG_CONF_LEVEL_MAIN)

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

OFLAGS = -O binary
HEXFILE = $(ELFFILE:.elf=.bin)

include $(RIOTBASE)/Makefile.include

ifeq ($(CPU_MODEL),cortexr)
LINKER_SCRIPT := $(APPLICATION).ld
endif
