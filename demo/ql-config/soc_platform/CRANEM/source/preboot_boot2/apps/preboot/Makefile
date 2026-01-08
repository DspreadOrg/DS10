# name of your application
APPLICATION = preboot

# If no BOARD is found in the environment, use this default:
BOARD ?= cortexr-arom-crane

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../../

BOARD_WHITELIST += cortexr-arom-crane

# Include all submodules depending on
#DIRS +=
#USEMODULE += libarom
ifeq (,$(filter x86-arom-native,$(BOARD)))
USEMODULE += psram
endif
USEMODULE += property
USEMODULE += log

CFLAGS += -Wno-int-conversion

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

include $(RIOTBASE)/Makefile.include

ifeq ($(CPU_MODEL),cortexr)
LINKER_SCRIPT := $(APPLICATION).ld
endif
