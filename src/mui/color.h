#ifndef MUI_COLOR_H
#define MUI_COLOR_H

#include <SDL/SDL.h>

/* A readable text colour for text drawn on bg: C_DARK on light fills,
 * C_WHITE on dark ones. */
SDL_Color color_text_on(SDL_Color bg);

/* Blends from -> to. t256 = 0 gives from, 256 gives to. Handy for fading
 * text, since SDL 1.2 can't fade a per-pixel-alpha surface. */
SDL_Color color_mix(SDL_Color from, SDL_Color to, int t256);

#endif
