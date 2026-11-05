#ifndef MUI_GFX_H
#define MUI_GFX_H

#include <stdbool.h>

#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>

/* Display, fonts and drawing primitives. Screens draw into gfx->screen (or
 * into an off-screen surface from gfx_new_surface) and call gfx_present()
 * when a frame is done.
 *
 * The *_on variants draw into any surface; the plain ones draw into gfx->screen. */

#define GFX_MAX_FONTS 8 /* extra font sizes gfx_font() can keep open */

typedef struct {
    int size;
    TTF_Font *font;
} GfxFont;

typedef struct {
    SDL_Surface *video;  /* real display surface */
    SDL_Surface *screen; /* off-screen buffer screens draw into */
    TTF_Font *font_big;   /* FONT_SIZE_BIG */
    TTF_Font *font;       /* FONT_SIZE */
    TTF_Font *font_small; /* FONT_SIZE_SMALL */
    GfxFont extra[GFX_MAX_FONTS]; /* other sizes, opened by gfx_font() */
    int extra_count;
} Gfx;

typedef enum { ALIGN_LEFT, ALIGN_CENTER, ALIGN_RIGHT } TextAlign;

/* Opens the 640x480 display and loads the three standard fonts.
 * On failure, call gfx_quit(). */
bool gfx_init(Gfx *gfx);
void gfx_quit(Gfx *gfx);

void gfx_present(Gfx *gfx);

/* FONT_PATH at any pixel size. Opened on first use and kept until gfx_quit().
 * Returns NULL if the font can't be opened or GFX_MAX_FONTS sizes are open. */
TTF_Font *gfx_font(Gfx *gfx, int size);

/* A w x h surface in the same pixel format as gfx->screen. Free with SDL_FreeSurface. */
SDL_Surface *gfx_new_surface(Gfx *gfx, int w, int h);

/* Rotates a 32bpp surface by 180 degrees in place. */
void gfx_rotate180(SDL_Surface *surf);

/* Fills rect (NULL = whole surface) with a colour. */
void gfx_fill_on(SDL_Surface *dst, const SDL_Rect *rect, SDL_Color color);
void gfx_fill(Gfx *gfx, const SDL_Rect *rect, SDL_Color color);

/* Draws a border of thickness t just inside rect. */
void gfx_border_on(SDL_Surface *dst, const SDL_Rect *rect, int t, SDL_Color color);

/* Draws a surface at x,y. */
void gfx_blit_on(SDL_Surface *dst, SDL_Surface *surf, int x, int y);
void gfx_blit(Gfx *gfx, SDL_Surface *surf, int x, int y);

int gfx_text_width(TTF_Font *font, const char *text);

/* Draws text anchored at x (left / centre / right edge), top at y. If max_w > 0
 * and the text is wider, it is cut on a UTF-8 boundary and ends with "...". */
void gfx_text_on(SDL_Surface *dst, TTF_Font *font, const char *text, int x, int y,
                 SDL_Color color, int max_w, TextAlign align);
void gfx_text(Gfx *gfx, TTF_Font *font, const char *text, int x, int y, SDL_Color color,
              int max_w, TextAlign align);

/* Draws text rotated 180 degrees on gfx->screen, centred on x, with the
 * rotated text's top edge at y (for a reader across the table). */
void gfx_text_rot180(Gfx *gfx, TTF_Font *font, const char *text, int x, int y, SDL_Color color);

/* Draws a string of digits (and '-') centred horizontally on cx, with the
 * digits' visible height centred vertically on cy. */
void gfx_digits_centered_on(SDL_Surface *dst, TTF_Font *font, const char *text, int cx, int cy,
                            SDL_Color color);

#endif
