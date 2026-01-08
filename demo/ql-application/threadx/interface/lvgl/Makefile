
#-------------------------------------------------------------------------------
# Configure variable
#-------------------------------------------------------------------------------
TOP_DIR:=$(TOP_DIR)
ROOT_DIR:=$(TOP_DIR)/../..
MAKEFILE_PATH :=$(subst $(strip \),/,$(abspath $(lastword $(MAKEFILE_LIST))))


#-------------------------------------------------------------------------------
# Configure source code files
#-------------------------------------------------------------------------------
SRC_FILES += ${wildcard *.[csS]}
SRC_FILES += ${wildcard core/*.c}
SRC_FILES += ${wildcard draw/*.c}
SRC_FILES += ${wildcard font/*.c}
SRC_FILES += ${wildcard hal/*.c}
SRC_FILES += ${wildcard misc/*.c}
SRC_FILES += ${wildcard widgets/*.c}
SRC_FILES += ${wildcard extra/*.c}
SRC_FILES += ${wildcard extra/layouts/*.c}
SRC_FILES += ${wildcard extra/layouts/flex/*.c}
SRC_FILES += ${wildcard extra/layouts/grid/*.c}
SRC_FILES += ${wildcard extra/libs/*.c}
SRC_FILES += ${wildcard extra/libs/bmp/*.c}
SRC_FILES += ${wildcard extra/libs/freetype/*.c}
SRC_FILES += ${wildcard extra/libs/fsdrv/*.c}
SRC_FILES += ${wildcard extra/libs/gif/*.c}
SRC_FILES += ${wildcard extra/libs/png/*.c}
SRC_FILES += ${wildcard extra/libs/qrcode/*.c}
SRC_FILES += ${wildcard extra/libs/rlottie/*.c}
SRC_FILES += ${wildcard extra/libs/sjpg/*.c}
SRC_FILES += ${wildcard extra/others/*.c}
SRC_FILES += ${wildcard extra/others/snapshot/*.c}
SRC_FILES += ${wildcard extra/widgets/*.c}
SRC_FILES += ${wildcard extra/widgets/animimg/*.c}
SRC_FILES += ${wildcard extra/widgets/calendar/*.c}
SRC_FILES += ${wildcard extra/widgets/chart/*.c}
SRC_FILES += ${wildcard extra/widgets/colorwheel/*.c}
SRC_FILES += ${wildcard extra/widgets/imgbtn/*.c}
SRC_FILES += ${wildcard extra/widgets/keyboard/*.c}
SRC_FILES += ${wildcard extra/widgets/led/*.c}
SRC_FILES += ${wildcard extra/widgets/list/*.c}
SRC_FILES += ${wildcard extra/widgets/meter/*.c}
SRC_FILES += ${wildcard extra/widgets/msgbox/*.c}
SRC_FILES += ${wildcard extra/widgets/span/*.c}
SRC_FILES += ${wildcard extra/widgets/spinbox/*.c}
SRC_FILES += ${wildcard extra/widgets/spinner/*.c}
SRC_FILES += ${wildcard extra/widgets/tabview/*.c}
SRC_FILES += ${wildcard extra/widgets/tileview/*.c}
SRC_FILES += ${wildcard extra/widgets/win/*.c}
SRC_FILES += ${wildcard extra/themes/*.c}
SRC_FILES += ${wildcard extra/themes/basic/*.c}
SRC_FILES += ${wildcard extra/themes/default/*.c}
SRC_FILES += ${wildcard extra/themes/mono/*.c}



#-------------------------------------------------------------------------------
# the include directories
#-------------------------------------------------------------------------------
INC_DIRS+= \
  -I${TOP_DIR}/ \
  -I${TOP_DIR}/src/lvgl/src \
  -I${TOP_DIR}/src/lvgl/src/core \
  -I${TOP_DIR}/src/lvgl/src/draw \
  -I${TOP_DIR}/src/lvgl/src/font \
  -I${TOP_DIR}/src/lvgl/src/hal \
  -I${TOP_DIR}/src/lvgl/src/misc \
  -I${TOP_DIR}/src/lvgl/src/widgets \
  -I${TOP_DIR}/common/include \


#-------------------------------------------------------------------------------
# Configure compile flag for C
#-------------------------------------------------------------------------------
U_FLAGS:=

#-------------------------------------------------------------------------------
# Configure compile flag for C++
#-------------------------------------------------------------------------------
U_CPPFLAGS:=

#-------------------------------------------------------------------------------
# Configure default macro define
#-------------------------------------------------------------------------------
DFLAGS+= \
  -D_REENT_ONLY \
  -D__OCPU_COMPILER_GCC__ \
  -D_WANT_USE_LONG_TIME_T \


#-------------------------------------------------------------------------------
# Configure link library
#-------------------------------------------------------------------------------
U_LIBS:=

#-------------------------------------------------------------------------------
# include mk file
#-------------------------------------------------------------------------------
include ${TOP_DIR}/config/common/makefile.mk
