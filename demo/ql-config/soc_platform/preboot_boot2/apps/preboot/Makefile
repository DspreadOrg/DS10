# name of your application
APPLICATION = preboot

# If no BOARD is found in the environment, use this default:
BOARD ?= crane

# If no AROM_VERSION is found in the environment, use this default:
AROM_VERSION ?= 20200730

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(realpath $(CURDIR)/../../)

BOARD_WHITELIST += crane cranew-mcu

# Include all submodules depending on
#DIRS +=
USEMODULE += bl2_arg
USEMODULE += libarom
ifeq (,$(filter x86-arom-native,$(BOARD)))
USEMODULE += psram
endif
USEMODULE += property
USEMODULE += log

ifeq ($(AROM_VERSION),20221111)
# CraneGT Z1
USEMODULE += ddr
endif

ifeq ($(AROM_VERSION),20220728)
# Fulmar DDR is initialized on the mcu side
USEMODULE += ddr
endif

ifeq ($(AROM_VERSION),20230909)
# Fulmar CR5 DDR get ddr type, write in prop for flasher
USEMODULE += ddr
endif

CFLAGS += -Wno-int-conversion
CFLAGS += -DAROM_VERSION=$(AROM_VERSION)

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

LINKER_SOURCE_SCRIPT := $(APPLICATION).lds

include $(RIOTBASE)/Makefile.include
