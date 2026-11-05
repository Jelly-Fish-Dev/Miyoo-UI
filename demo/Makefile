# Miyoo-UI demo app
#
# On the host (macOS/Linux with Docker):
#   make                 cross-compile inside the toolchain container + stage dist/App/MiyooUIDemo
#   make debug           debug build (-O0 -g3, not stripped)
#   make zip             dist/MiyooUIDemo.zip, unzip onto the SD card root
#   make deploy HOST=ip  copy the app to a Mini Plus over SSH
#   make log HOST=ip     print the app's log.txt from the device
#   make clean
#
# Per-machine settings go in local.mk (ignored by git), e.g.
#   DEMO_HOST ?= 192.168.1.50
#   DEMO_SSH_PASS ?= onion

-include local.mk

APP     := MiyooUIDemo
TARGET  := muidemo
DEBUG   ?= 0
VARIANT := $(if $(filter 1,$(DEBUG)),debug,release)

DOCKER_IMAGE  ?= aemiii91/miyoomini-toolchain:latest
TOOLCHAIN_DIR := /opt/miyoomini-toolchain
SYSROOT       := $(TOOLCHAIN_DIR)/usr/arm-linux-gnueabihf/sysroot
CROSS_COMPILE ?= $(TOOLCHAIN_DIR)/usr/bin/arm-linux-gnueabihf-

BUILD_ROOT := build
BUILD_DIR  := $(BUILD_ROOT)/$(VARIANT)
DIST_DIR   := dist
APP_DIST   := $(DIST_DIR)/App/$(APP)

# The library is the folder above this one.
MUI_DIR ?= ..

HOST       ?= $(DEMO_HOST)
SSH_USER   ?= onion
DEVICE_APP := /mnt/SDCARD/App/$(APP)
SSH        := ssh -o ConnectTimeout=5 -o StrictHostKeyChecking=accept-new $(SSH_USER)@$(HOST)

ifneq ($(DEMO_SSH_PASS),)
export DEMO_SSH_PASS
export SSH_ASKPASS := $(CURDIR)/scripts/askpass.sh
export SSH_ASKPASS_REQUIRE := force
export DISPLAY ?= :0
endif

.PHONY: all build dist debug zip shell clean deploy log check-host _build _dist

ifeq ($(wildcard $(TOOLCHAIN_DIR)),)
# ---------------------------------------------------------------- host side

# Mount the whole Miyoo-UI repo so the container sees both the library and the demo.
DOCKER_RUN := docker run --rm --platform linux/amd64 \
	-v "$(CURDIR)/..":/root/Miyoo-UI -w /root/Miyoo-UI/demo $(DOCKER_IMAGE)

all: dist

build dist:
	$(DOCKER_RUN) make -j4 DEBUG=$(DEBUG) _dist

debug:
	$(MAKE) DEBUG=1 dist

zip: dist
	cd $(DIST_DIR) && rm -f $(APP).zip && zip -qr -X $(APP).zip App/$(APP) && echo "$(DIST_DIR)/$(APP).zip"

shell:
	docker run --rm -it --platform linux/amd64 \
		-v "$(CURDIR)/..":/root/Miyoo-UI -w /root/Miyoo-UI/demo $(DOCKER_IMAGE) bash

check-host:
	@test -n "$(HOST)" || { echo "Set HOST=<device ip> (or DEMO_HOST in local.mk)"; exit 1; }

deploy: check-host dist
	COPYFILE_DISABLE=1 tar -C $(DIST_DIR) -cf - App/$(APP) | \
		$(SSH) 'tar -C /mnt/SDCARD -xf - && find $(DEVICE_APP) -name "._*" -delete'
	@echo "Deployed to $(HOST):$(DEVICE_APP)"

log: check-host
	$(SSH) 'cat $(DEVICE_APP)/log.txt'

clean:
	rm -rf $(BUILD_ROOT) $(DIST_DIR)

else
# ----------------------------------------------------------- container side

CC    := $(CROSS_COMPILE)gcc
AR    := $(CROSS_COMPILE)ar
STRIP := $(CROSS_COMPILE)strip

ARCH_FLAGS := -marm -march=armv7-a -mtune=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard
ifeq ($(DEBUG),1)
OPT_FLAGS := -O0 -g3 -DDEBUG
else
OPT_FLAGS := -O2 -g -DNDEBUG
endif

CFLAGS  := -std=gnu99 $(OPT_FLAGS) -Wall -Wextra -Wno-format-truncation $(ARCH_FLAGS) \
	-Isrc -I$(SYSROOT)/usr/include -MMD -MP
LDFLAGS := -L$(SYSROOT)/usr/lib -Wl,-rpath-link,$(SYSROOT)/usr/lib
LDLIBS  := -lSDL_image -lSDL_ttf -lSDL -lpthread -lm

SRCS := $(shell find src -name '*.c')
OBJS := $(SRCS:%.c=$(BUILD_DIR)/obj/%.o)

include $(MUI_DIR)/mui.mk
CFLAGS += $(MUI_CFLAGS)

_build: $(BUILD_DIR)/$(TARGET)

$(BUILD_DIR)/$(TARGET): $(OBJS) $(MUI_LIB)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(BUILD_DIR)/obj/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

_dist: _build
	rm -rf $(APP_DIST)
	mkdir -p $(APP_DIST)/bin $(APP_DIST)/res
	cp app/config.json app/launch.sh app/icon.png $(APP_DIST)/
	chmod +x $(APP_DIST)/launch.sh
	cp -r $(MUI_FONTS) res/images $(APP_DIST)/res/
ifeq ($(DEBUG),1)
	cp $(BUILD_DIR)/$(TARGET) $(APP_DIST)/bin/$(TARGET)
else
	$(STRIP) -o $(APP_DIST)/bin/$(TARGET) $(BUILD_DIR)/$(TARGET)
endif
	@echo "Staged $(APP_DIST) ($(VARIANT))"

-include $(OBJS:.o=.d)

endif
