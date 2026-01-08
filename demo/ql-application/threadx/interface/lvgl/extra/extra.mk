#SRC_FILES += $(shell find -L $(LVGL_DIR)/$(LVGL_DIR_NAME)/src/extra -name \*.c)
SRC_FILES += $(foreach dir, $(DIRS), $(wildcard $(dir)/*.c))