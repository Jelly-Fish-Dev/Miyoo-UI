#ifndef MUI_THEME_H
#define MUI_THEME_H

#include <SDL/SDL.h>

/* Screen size, font and the base colours shared by every Miyoo-UI app.
 * Apps add their own layout and extra colours in their own ui/theme.h. */

#define SCREEN_W 640
#define SCREEN_H 480

/* Relative to the app folder (launch.sh cds there). The Makefile copies
 * Miyoo-UI's res/fonts into the app's res/fonts. */
#define FONT_PATH       "res/fonts/Exo2-Bold.ttf"
#define FONT_SIZE_BIG   52 /* gfx->font_big */
#define FONT_SIZE       26 /* gfx->font */
#define FONT_SIZE_SMALL 19 /* gfx->font_small */

/* text */
static const SDL_Color C_WHITE  = {240, 240, 245, 0};
static const SDL_Color C_DIM    = {140, 145, 160, 0};
static const SDL_Color C_DARK   = {20, 22, 30, 0};
static const SDL_Color C_ACCENT = {255, 196, 60, 0};
static const SDL_Color C_DANGER = {235, 80, 80, 0};

/* fills */
static const SDL_Color C_BG        = {22, 24, 34, 0};
static const SDL_Color C_PANEL     = {32, 35, 48, 0};
static const SDL_Color C_RULE      = {60, 64, 84, 0};
static const SDL_Color C_HIGHLIGHT = {255, 196, 60, 0};

#endif
