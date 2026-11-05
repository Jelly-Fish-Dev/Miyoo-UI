# Miyoo-UI

The shared UI code for native [OnionOS](https://github.com/OnionUI/Onion) apps on the Miyoo Mini and Mini Plus, written in C99 for SDL 1.2 and SDL_ttf. It's used by:

- RHSM: Retro Handheld Save Manager
- Life Counter: MTG-style life counter

## What's in it

| Header | What it does |
|---|---|
| `mui/gfx.h` | 640x480 display with an off-screen buffer, the standard fonts plus `gfx_font(size)` for other sizes, fills, borders, blits, text with `...` truncation, 180° rotation and rotated text, centred digits |
| `mui/image.h` | Load a PNG or JPG and scale it to fit a box with smooth area-averaging, e.g. Onion box art. Needs `-lSDL_image` |
| `mui/color.h` | `color_text_on(bg)` picks readable text for a fill; `color_mix` blends two colours (for fades) |
| `mui/theme.h` | Screen size, font path and sizes, the base colours (`C_WHITE`, `C_DIM`, `C_BG`, `C_PANEL`, `C_ACCENT`, `C_DANGER`, ...) |
| `mui/input.h` | Miyoo Mini buttons, mapped from Onion's SDL key events |
| `mui/screen.h` | `ScreenResult`: `SCREEN_OK` / `SCREEN_DONE` / `SCREEN_EXIT` |
| `mui/log.h` | `LOG_INFO` / `LOG_WARN` / `LOG_ERROR` to stderr; `LOG_DEBUG` only in debug builds |

`res/fonts/` holds the font the theme uses: Exo 2 Bold, SIL OFL 1.1.

Apps add their own layout and colours in their own `ui/theme.h`, which includes `mui/theme.h` first.

## Using it from an app

Clone Miyoo-UI next to the app (`../Miyoo-UI`). In the app's Makefile:

```make
# host side: mount the library into the toolchain container
MUI_DIR      ?= ../Miyoo-UI
MUI_HOST_DIR := $(shell cd "$(MUI_DIR)" 2>/dev/null && pwd)
DOCKER_RUN   := docker run --rm --platform linux/amd64 \
	-v "$(CURDIR)":/root/workspace -v "$(MUI_HOST_DIR)":/root/Miyoo-UI \
	-w /root/workspace $(DOCKER_IMAGE)
#   ... $(DOCKER_RUN) make MUI_DIR=/root/Miyoo-UI _dist

# container side, after CC / AR / CFLAGS / BUILD_DIR
include $(MUI_DIR)/mui.mk
CFLAGS += $(MUI_CFLAGS)

$(BUILD_DIR)/$(TARGET): $(OBJS) $(MUI_LIB)
	$(CC) $(LDFLAGS) -o $@ $^ -lSDL_ttf -lSDL -lpthread -lm

# when staging the app
	cp -r $(MUI_FONTS) $(APP_DIST)/res/
```

The library is compiled with the app's own `CFLAGS` into `build/<variant>/mui/libmui.a`, so `DEBUG=1` applies to it too. The linker only pulls in the modules an app actually uses.

## License

[MIT](LICENSE) © 2026 Jelly-Fish-Dev. The Exo 2 font keeps its own license (SIL OFL 1.1, see `res/fonts/OFL.txt`).
