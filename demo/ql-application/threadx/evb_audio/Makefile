
#-------------------------------------------------------------------------------
# Configure variable
#-------------------------------------------------------------------------------
TOP_DIR:=$(TOP_DIR)
ROOT_DIR:=$(TOP_DIR)/../..
MAKEFILE_PATH :=$(subst $(strip \),/,$(abspath $(lastword $(MAKEFILE_LIST))))


#-------------------------------------------------------------------------------
# Configure source code files
#-------------------------------------------------------------------------------
#SRC_DIRS := $(shell find . -type d)
#SRC_FILESB := $(basename $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.[csS])))

SRC_FILES += ${wildcard *.[csS]}
SRC_FILES += ${wildcard audiopack/*.c}
SRC_FILES += ${wildcard wifi808/*.c}
SRC_FILES += ${wildcard disp/*.c}

#-------------------------------------------------------------------------------
# the include directories
#-------------------------------------------------------------------------------
INC_DIRS+= \
  -I${TOP_DIR}/ \
  -I${TOP_DIR}/common/include/fs \
  -I${TOP_DIR}/common/include/lwipv4v6 \
  -I${TOP_DIR}/common/include \
  -I${TOP_DIR}/common/include/dsp \
  -I${TOP_DIR}/interface/lvgl \
  -I${TOP_DIR}/common/include/net \
  -I${TOP_DIR}/common/include/net/HTTPClient \
  -I${TOP_DIR}/common/include/net/cloud/aliyun \
  -I${TOP_DIR}/common/include/net/mqtt/MQTTPacket \
  -I${TOP_DIR}/common/include/net/mqtt/MQTTClient-C \
  -I${TOP_DIR}/evb_audio/audiopack \
  -I${TOP_DIR}/evb_audio/disp \
  -I${TOP_DIR}/evb_audio \

#-------------------------------------------------------------------------------
# Configure compile flag
#-------------------------------------------------------------------------------
U_FLAGS:=

#-------------------------------------------------------------------------------
# Configure default macro define
#-------------------------------------------------------------------------------
DFLAGS+= \
  -D__OCPU_COMPILER_GCC__ \
  -D_WANT_USE_LONG_TIME_T \


#-------------------------------------------------------------------------------
# include mk file
#-------------------------------------------------------------------------------
include ${TOP_DIR}\config\common\makefile.mk
