#ifndef MUI_GLYPHS_H
#define MUI_GLYPHS_H

#include <SDL/SDL_ttf.h>

#include "mui/gfx.h"

/* Button icons from the Esquire Generic Gamepad font (Miyoo-UI res/fonts, Apache 2.0).
 * Only the face buttons, D-pad and shoulder buttons have icons; START, SELECT
 * and MENU stay as words.
 *
 * Face buttons are drawn as the four-button diamond with one filled, so they
 * show position, not letter. On the Miyoo Mini: X top, A right, B bottom, Y left. */

#define GLYPH_FONT_PATH "res/fonts/EsquireGenericGamepdFont_CL.ttf"

typedef enum {
    GLYPH_A, GLYPH_B, GLYPH_X, GLYPH_Y,                   /* face buttons */
    GLYPH_DPAD, GLYPH_UP, GLYPH_DOWN, GLYPH_LEFT, GLYPH_RIGHT, /* D-pad */
    GLYPH_UP_DOWN, GLYPH_LEFT_RIGHT,
    GLYPH_L1, GLYPH_R1, GLYPH_L2, GLYPH_R2,               /* shoulders */
    GLYPH_COUNT
} Glyph;

/* The font character for an icon. */
const char *glyph_char(Glyph g);

/* GLYPH_FONT_PATH at a pixel size (GLYPH_SIZE_SMALL next to font_small), or
 * NULL (logged). Close with TTF_CloseFont. */
TTF_Font *glyph_font_open(int size);

/* Hint strings mix text and icon tags, e.g. "{DPAD} move   {A} pick".
 * Tags: {A} {B} {X} {Y}  {DPAD} {UP} {DOWN} {LEFT} {RIGHT} {UPDOWN} {LEFTRIGHT}
 *       {L1} {R1} {L2} {R2}
 * Icons are centred vertically on the text's capitals. If icons is NULL (font
 * missing) each tag is drawn as its name instead. */

int glyph_hint_width(TTF_Font *text, TTF_Font *icons, const char *hint);

/* Centred on cx, top at y. */
void glyph_hint(Gfx *gfx, TTF_Font *text, TTF_Font *icons, const char *hint, int cx, int y,
                SDL_Color color);
void glyph_hint_on(SDL_Surface *dst, TTF_Font *text, TTF_Font *icons, const char *hint, int cx,
                   int y, SDL_Color color);

/* Rotated 180 degrees for a reader across the table: centred on cx, with the
 * rotated line's top edge at y (same placement as gfx_text_rot180). */
void glyph_hint_rot180(Gfx *gfx, TTF_Font *text, TTF_Font *icons, const char *hint, int cx,
                       int y, SDL_Color color);

#endif
