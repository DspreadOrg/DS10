# name of your application
APPLICATION = boot2
DISABLE_UART_TRY_DOWNLOAD ?= 1
LOG_CONF_LEVEL_MAIN ?= LOG_LEVEL_INFO

# If no BOARD is found in the environment, use this default:
BOARD ?= crane

# If no AROM_VERSION is found in the environment, use this default:
AROM_VERSION ?= 20200730

# If no AROMVERSION is found in the environment, use this default:
export AROMVERSION ?= 2019.01.15

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(realpath $(CURDIR)/../../)

BOARD_WHITELIST += crane cranew-mcu

# Include all submodules depending on
USEMODULE += libarom
USEMODULE += ptable
USEMODULE += crc32
USEMODULE += property
USEMODULE += log
USEMODULE += bl2_arg
USEMODULE += fip
ifeq ($(AROM_VERSION),20200730)
# CraneG/CraneM A0
USEMODULE += patchlib
USEMODULE += secureboot_arom_patch
USEMODULE += arom_syscall
USEMODULE += decompress
USEMODULE += loadtable
endif

ifneq ($(findstring $(AROM_VERSION), 20200730, 20211016, 20221106, 20221111),)
CFLAGS += -DUSE_ARB
endif

CFLAGS += -DDISABLE_UART_TRY_DOWNLOAD=$(DISABLE_UART_TRY_DOWNLOAD)
CFLAGS += -Wno-int-conversion
CFLAGS += -DLOG_CONF_LEVEL_MAIN=$(LOG_CONF_LEVEL_MAIN)
CFLAGS += -DAROM_VERSION=$(AROM_VERSION)

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

OFLAGS = -O binary
HEXFILE = $(ELFFILE:.elf=.bin)

LINKER_SOURCE_SCRIPT := $(APPLICATION).lds

include $(RIOTBASE)/Makefile.include
