# Miyoo-UI build fragment. Include it from an app Makefile's container side,
# after CC, AR, CFLAGS and BUILD_DIR are set:
#
#   MUI_DIR ?= ../Miyoo-UI
#   include $(MUI_DIR)/mui.mk
#   CFLAGS  += $(MUI_CFLAGS)
#   $(BUILD_DIR)/$(TARGET): $(OBJS) $(MUI_LIB)   # link $(MUI_LIB) after the app objects
#
# Provides:
#   MUI_CFLAGS  include path, so apps write #include "mui/gfx.h"
#   MUI_LIB     static library, built from src/mui/*.c with the app's CFLAGS.
#               Only the modules an app uses get linked in, so an app that never
#               calls image_* doesn't need -lSDL_image.
#   MUI_FONTS   folder to copy to <app>/res/fonts (FONT_PATH in mui/theme.h)
#
# Link libraries: -lSDL_ttf -lSDL -lm, plus -lSDL_image if the app uses mui/image.h.

MUI_SRC_DIR := $(MUI_DIR)/src
MUI_SRCS    := $(wildcard $(MUI_SRC_DIR)/mui/*.c)
MUI_OBJS    := $(patsubst $(MUI_SRC_DIR)/mui/%.c,$(BUILD_DIR)/mui/%.o,$(MUI_SRCS))
MUI_LIB     := $(BUILD_DIR)/mui/libmui.a
MUI_CFLAGS  := -I$(MUI_SRC_DIR)
MUI_FONTS   := $(MUI_DIR)/res/fonts

ifeq ($(MUI_SRCS),)
$(error Miyoo-UI not found at MUI_DIR=$(MUI_DIR))
endif

$(MUI_LIB): $(MUI_OBJS)
	rm -f $@
	$(AR) rcs $@ $^

$(BUILD_DIR)/mui/%.o: $(MUI_SRC_DIR)/mui/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(MUI_CFLAGS) -c -o $@ $<

-include $(MUI_OBJS:.o=.d)
