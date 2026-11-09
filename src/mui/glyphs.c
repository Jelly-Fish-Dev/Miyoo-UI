#include "mui/glyphs.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "mui/log.h"

/* Tag name and font character for each icon (characters from the font's
 * IconMapping.txt, e.g. "Face Button Bottom" is ','). */
static const struct {
    const char *tag;
    const char *ch;
} GLYPHS[GLYPH_COUNT] = {
    [GLYPH_A] = {"A", "."},      /* face right */
    [GLYPH_B] = {"B", ","},      /* face bottom */
    [GLYPH_X] = {"X", "/"},      /* face top */
    [GLYPH_Y] = {"Y", "Q"},      /* face left */
    [GLYPH_DPAD] = {"DPAD", "Y"},
    [GLYPH_UP] = {"UP", "O"},
    [GLYPH_DOWN] = {"DOWN", "U"},
    [GLYPH_LEFT] = {"LEFT", "P"},
    [GLYPH_RIGHT] = {"RIGHT", "I"},
    [GLYPH_UP_DOWN] = {"UPDOWN", "A"},
    [GLYPH_LEFT_RIGHT] = {"LEFTRIGHT", "S"},
    [GLYPH_L1] = {"L1", "d"},    /* generic left button */
    [GLYPH_R1] = {"R1", "'"},    /* generic right button */
    [GLYPH_L2] = {"L2", "j"},    /* generic left trigger */
    [GLYPH_R2] = {"R2", "v"},    /* generic right trigger */
};

const char *glyph_char(Glyph g)
{
    return (g >= 0 && g < GLYPH_COUNT) ? GLYPHS[g].ch : "";
}

TTF_Font *glyph_font_open(int size)
{
    TTF_Font *f = TTF_OpenFont(GLYPH_FONT_PATH, size);
    if (!f)
        LOG_ERROR("TTF_OpenFont(%s, %d) failed: %s", GLYPH_FONT_PATH, size, TTF_GetError());
    return f;
}

/* If s starts with a known "{TAG}", sets *g and returns its length, else 0. */
static size_t parse_tag(const char *s, Glyph *g)
{
    if (s[0] != '{')
        return 0;
    const char *end = strchr(s, '}');
    if (!end)
        return 0;
    size_t n = (size_t)(end - s - 1);
    for (int i = 0; i < GLYPH_COUNT; i++) {
        if (strlen(GLYPHS[i].tag) == n && strncmp(s + 1, GLYPHS[i].tag, n) == 0) {
            *g = (Glyph)i;
            return n + 2;
        }
    }
    return 0;
}

/* Vertical centre of a character's ink, measured from the top of its line. */
static int ink_mid(TTF_Font *font, Uint16 ch)
{
    int minx, maxx, miny, maxy, adv;
    if (TTF_GlyphMetrics(font, ch, &minx, &maxx, &miny, &maxy, &adv) != 0)
        return TTF_FontHeight(font) / 2;
    return TTF_FontAscent(font) - (maxy + miny) / 2;
}

/* One piece of a hint: a text run or an icon. */
typedef struct {
    TTF_Font *font;
    char str[128];
    int dy; /* offset from the line's top */
} Piece;

/* Splits the hint into its next piece starting at *p. Returns false at the end. */
static bool next_piece(const char **p, TTF_Font *text, TTF_Font *icons, Piece *out)
{
    if (!**p)
        return false;
    Glyph g;
    size_t n = parse_tag(*p, &g);
    if (n) {
        *p += n;
        if (icons) {
            out->font = icons;
            snprintf(out->str, sizeof out->str, "%s", GLYPHS[g].ch);
            out->dy = ink_mid(text, 'X') - ink_mid(icons, (Uint16)GLYPHS[g].ch[0]);
        } else {
            out->font = text;
            snprintf(out->str, sizeof out->str, "%s", GLYPHS[g].tag);
            out->dy = 0;
        }
        return true;
    }
    size_t len = 0;
    while ((*p)[len] && !parse_tag(*p + len, &g) && len < sizeof out->str - 1)
        len++;
    memcpy(out->str, *p, len);
    out->str[len] = '\0';
    *p += len;
    out->font = text;
    out->dy = 0;
    return true;
}

int glyph_hint_width(TTF_Font *text, TTF_Font *icons, const char *hint)
{
    int w = 0;
    Piece pc;
    for (const char *p = hint; next_piece(&p, text, icons, &pc);)
        w += gfx_text_width(pc.font, pc.str);
    return w;
}

/* Draws the hint with its left edge at x. With rot180 every piece is rotated
 * and placed mirrored inside the line's box. */
static void draw(SDL_Surface *dst, TTF_Font *text, TTF_Font *icons, const char *hint, int x, int y,
                 SDL_Color color, bool rot180)
{
    int line_w = glyph_hint_width(text, icons, hint);
    int line_h = TTF_FontHeight(text);
    int ox = 0;
    Piece pc;
    for (const char *p = hint; next_piece(&p, text, icons, &pc);) {
        if (!pc.str[0])
            continue;
        SDL_Surface *s = TTF_RenderUTF8_Blended(pc.font, pc.str, color);
        if (!s) {
            LOG_WARN("TTF_RenderUTF8_Blended failed: %s", TTF_GetError());
            continue;
        }
        if (rot180) {
            gfx_rotate180(s);
            gfx_blit_on(dst, s, x + line_w - ox - s->w, y + line_h - pc.dy - s->h);
        } else {
            gfx_blit_on(dst, s, x + ox, y + pc.dy);
        }
        ox += s->w;
        SDL_FreeSurface(s);
    }
}

void glyph_hint(Gfx *gfx, TTF_Font *text, TTF_Font *icons, const char *hint, int cx, int y,
                SDL_Color color)
{
    glyph_hint_on(gfx->screen, text, icons, hint, cx, y, color);
}

void glyph_hint_on(SDL_Surface *dst, TTF_Font *text, TTF_Font *icons, const char *hint, int cx,
                   int y, SDL_Color color)
{
    draw(dst, text, icons, hint, cx - glyph_hint_width(text, icons, hint) / 2, y, color, false);
}

void glyph_hint_rot180(Gfx *gfx, TTF_Font *text, TTF_Font *icons, const char *hint, int cx,
                       int y, SDL_Color color)
{
    draw(gfx->screen, text, icons, hint, cx - glyph_hint_width(text, icons, hint) / 2, y, color,
         true);
}
