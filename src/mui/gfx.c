#include "mui/gfx.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "mui/log.h"
#include "mui/theme.h"

/* Opens the three standard sizes of path into gfx. */
static bool open_fonts(Gfx *gfx, const char *path)
{
    gfx->font_big = TTF_OpenFont(path, gfx->sizes.big);
    gfx->font = TTF_OpenFont(path, gfx->sizes.normal);
    gfx->font_small = TTF_OpenFont(path, gfx->sizes.small);
    if (gfx->font_big && gfx->font && gfx->font_small) {
        snprintf(gfx->font_path, sizeof gfx->font_path, "%s", path);
        return true;
    }
    LOG_ERROR("TTF_OpenFont(%s) failed: %s", path, TTF_GetError());
    TTF_Font *fonts[] = {gfx->font_big, gfx->font, gfx->font_small};
    for (size_t i = 0; i < sizeof fonts / sizeof fonts[0]; i++)
        if (fonts[i]) TTF_CloseFont(fonts[i]);
    gfx->font_big = gfx->font = gfx->font_small = NULL;
    return false;
}

bool gfx_init(Gfx *gfx)
{
    return gfx_init_font(gfx, NULL, NULL);
}

bool gfx_init_font(Gfx *gfx, const char *font_path, const GfxFontSizes *sizes)
{
    static const GfxFontSizes defaults = GFX_FONT_SIZES_DEFAULT;
    memset(gfx, 0, sizeof *gfx);
    gfx->sizes = sizes ? *sizes : defaults;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        LOG_ERROR("SDL_Init failed: %s", SDL_GetError());
        return false;
    }
    SDL_ShowCursor(SDL_DISABLE);
    SDL_EnableKeyRepeat(300, 60);

    /* Same pattern as Onion's own apps: draw into a buffer, blit to the video surface. */
    gfx->video = SDL_SetVideoMode(SCREEN_W, SCREEN_H, 32, SDL_HWSURFACE);
    if (!gfx->video) {
        LOG_ERROR("SDL_SetVideoMode failed: %s", SDL_GetError());
        return false;
    }
    gfx->screen = SDL_CreateRGBSurface(SDL_HWSURFACE, SCREEN_W, SCREEN_H, 32, 0, 0, 0, 0);
    if (!gfx->screen) {
        LOG_ERROR("SDL_CreateRGBSurface failed: %s", SDL_GetError());
        return false;
    }

    if (TTF_Init() != 0) {
        LOG_ERROR("TTF_Init failed: %s", TTF_GetError());
        return false;
    }
    bool ok = font_path && font_path[0] && open_fonts(gfx, font_path);
    if (!ok && !open_fonts(gfx, FONT_PATH))
        return false;
    LOG_INFO("font %s (%d/%d/%d px)", gfx->font_path, gfx->sizes.big, gfx->sizes.normal,
             gfx->sizes.small);

    LOG_INFO("video %dx%d %dbpp", gfx->video->w, gfx->video->h,
             gfx->video->format->BitsPerPixel);
    return true;
}

void gfx_quit(Gfx *gfx)
{
    for (int i = 0; i < gfx->extra_count; i++)
        TTF_CloseFont(gfx->extra[i].font);
    TTF_Font *fonts[] = {gfx->font_small, gfx->font, gfx->font_big};
    for (size_t i = 0; i < sizeof fonts / sizeof fonts[0]; i++)
        if (fonts[i]) TTF_CloseFont(fonts[i]);
    if (TTF_WasInit()) TTF_Quit();
    if (gfx->screen) SDL_FreeSurface(gfx->screen);
    SDL_Quit();
    memset(gfx, 0, sizeof *gfx);
}

void gfx_present(Gfx *gfx)
{
    SDL_BlitSurface(gfx->screen, NULL, gfx->video, NULL);
    SDL_Flip(gfx->video);
}

TTF_Font *gfx_font(Gfx *gfx, int size)
{
    if (size == gfx->sizes.big) return gfx->font_big;
    if (size == gfx->sizes.normal) return gfx->font;
    if (size == gfx->sizes.small) return gfx->font_small;
    for (int i = 0; i < gfx->extra_count; i++)
        if (gfx->extra[i].size == size)
            return gfx->extra[i].font;

    if (gfx->extra_count == GFX_MAX_FONTS) {
        LOG_ERROR("gfx_font(%d): already %d extra sizes open", size, GFX_MAX_FONTS);
        return NULL;
    }
    TTF_Font *f = TTF_OpenFont(gfx->font_path, size);
    if (!f) {
        LOG_ERROR("TTF_OpenFont(%s, %d) failed: %s", gfx->font_path, size, TTF_GetError());
        return NULL;
    }
    gfx->extra[gfx->extra_count].size = size;
    gfx->extra[gfx->extra_count].font = f;
    gfx->extra_count++;
    return f;
}

SDL_Surface *gfx_new_surface(Gfx *gfx, int w, int h)
{
    SDL_PixelFormat *f = gfx->screen->format;
    SDL_Surface *s = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 32, f->Rmask, f->Gmask, f->Bmask, 0);
    if (!s)
        LOG_ERROR("SDL_CreateRGBSurface(%dx%d) failed: %s", w, h, SDL_GetError());
    return s;
}

void gfx_rotate180(SDL_Surface *surf)
{
    if (surf->format->BytesPerPixel != 4) {
        LOG_WARN("gfx_rotate180: unsupported %dbpp surface", surf->format->BitsPerPixel);
        return;
    }
    if (SDL_MUSTLOCK(surf) && SDL_LockSurface(surf) != 0)
        return;

    int w = surf->w, h = surf->h;
    uint8_t *base = surf->pixels;
    /* Pixel (x, y) swaps with (w-1-x, h-1-y). Walk the top half of the rows;
     * for the middle row of an odd height, only its first half. */
    for (int y = 0; y < (h + 1) / 2; y++) {
        uint32_t *top = (uint32_t *)(base + y * surf->pitch);
        uint32_t *bot = (uint32_t *)(base + (h - 1 - y) * surf->pitch);
        int n = (top == bot) ? w / 2 : w;
        for (int x = 0; x < n; x++) {
            uint32_t t = top[x];
            top[x] = bot[w - 1 - x];
            bot[w - 1 - x] = t;
        }
    }

    if (SDL_MUSTLOCK(surf))
        SDL_UnlockSurface(surf);
}

void gfx_fill_on(SDL_Surface *dst, const SDL_Rect *rect, SDL_Color c)
{
    SDL_Rect r;
    if (rect)
        r = *rect; /* SDL_FillRect may clip the rect it is given */
    SDL_FillRect(dst, rect ? &r : NULL, SDL_MapRGB(dst->format, c.r, c.g, c.b));
}

void gfx_fill(Gfx *gfx, const SDL_Rect *rect, SDL_Color c)
{
    gfx_fill_on(gfx->screen, rect, c);
}

void gfx_border_on(SDL_Surface *dst, const SDL_Rect *r, int t, SDL_Color c)
{
    SDL_Rect top = {r->x, r->y, r->w, (Uint16)t};
    SDL_Rect bot = {r->x, (Sint16)(r->y + r->h - t), r->w, (Uint16)t};
    SDL_Rect left = {r->x, r->y, (Uint16)t, r->h};
    SDL_Rect right = {(Sint16)(r->x + r->w - t), r->y, (Uint16)t, r->h};
    gfx_fill_on(dst, &top, c);
    gfx_fill_on(dst, &bot, c);
    gfx_fill_on(dst, &left, c);
    gfx_fill_on(dst, &right, c);
}

void gfx_blit_on(SDL_Surface *dst, SDL_Surface *surf, int x, int y)
{
    SDL_Rect pos = {(Sint16)x, (Sint16)y, 0, 0};
    SDL_BlitSurface(surf, NULL, dst, &pos);
}

void gfx_blit(Gfx *gfx, SDL_Surface *surf, int x, int y)
{
    gfx_blit_on(gfx->screen, surf, x, y);
}

int gfx_text_width(TTF_Font *font, const char *text)
{
    int w = 0, h = 0;
    TTF_SizeUTF8(font, text, &w, &h);
    return w;
}

void gfx_text_on(SDL_Surface *dst, TTF_Font *font, const char *text, int x, int y,
                 SDL_Color color, int max_w, TextAlign align)
{
    char buf[320];
    snprintf(buf, sizeof buf, "%s", text);

    if (max_w > 0 && gfx_text_width(font, buf) > max_w) {
        size_t len = strlen(buf);
        while (len > 0) {
            do {
                len--;
            } while (len > 0 && ((unsigned char)buf[len] & 0xC0) == 0x80);
            buf[len] = '\0';
            char tmp[330];
            snprintf(tmp, sizeof tmp, "%s...", buf);
            if (gfx_text_width(font, tmp) <= max_w) {
                memcpy(buf, tmp, strlen(tmp) + 1); /* fits: len shrank by >= 1 */
                break;
            }
        }
    }
    if (!buf[0])
        return;

    SDL_Surface *surf = TTF_RenderUTF8_Blended(font, buf, color);
    if (!surf) {
        LOG_WARN("TTF_RenderUTF8_Blended failed: %s", TTF_GetError());
        return;
    }
    int px = align == ALIGN_CENTER ? x - surf->w / 2 : align == ALIGN_RIGHT ? x - surf->w : x;
    gfx_blit_on(dst, surf, px, y);
    SDL_FreeSurface(surf);
}

void gfx_text(Gfx *gfx, TTF_Font *font, const char *text, int x, int y, SDL_Color color,
              int max_w, TextAlign align)
{
    gfx_text_on(gfx->screen, font, text, x, y, color, max_w, align);
}

void gfx_utf8_prefix(const char *text, int max_chars, char *out, size_t n)
{
    size_t i = 0;
    int chars = 0;
    while (text[i] && i + 1 < n) {
        if (((unsigned char)text[i] & 0xC0) != 0x80) { /* start of a character */
            if (chars == max_chars)
                break;
            chars++;
        }
        i++;
    }
    /* don't end on half a character if out was too small */
    while (i > 0 && text[i] && ((unsigned char)text[i] & 0xC0) == 0x80)
        i--;
    memcpy(out, text, i);
    out[i] = '\0';
}

bool gfx_text_scroll(Gfx *gfx, TTF_Font *font, const char *text, int x, int y, int w,
                     SDL_Color color, TextAlign align, Uint32 t_ms)
{
    if (!text[0])
        return false;
    SDL_Surface *surf = TTF_RenderUTF8_Blended(font, text, color);
    if (!surf) {
        LOG_WARN("TTF_RenderUTF8_Blended failed: %s", TTF_GetError());
        return false;
    }
    if (surf->w <= w) {
        int px = align == ALIGN_CENTER ? x + (w - surf->w) / 2
               : align == ALIGN_RIGHT  ? x + w - surf->w
                                       : x;
        gfx_blit(gfx, surf, px, y);
        SDL_FreeSurface(surf);
        return false;
    }

    Uint32 travel = (Uint32)(surf->w - w);
    Uint32 scroll_ms = travel * 1000 / GFX_SCROLL_SPEED;
    Uint32 p = t_ms % (GFX_SCROLL_PAUSE_MS + scroll_ms + GFX_SCROLL_PAUSE_MS);
    Uint32 off = p < GFX_SCROLL_PAUSE_MS ? 0
               : p < GFX_SCROLL_PAUSE_MS + scroll_ms ? (p - GFX_SCROLL_PAUSE_MS) * GFX_SCROLL_SPEED / 1000
               : travel;

    SDL_Rect src = {(Sint16)off, 0, (Uint16)w, (Uint16)surf->h};
    SDL_Rect dst = {(Sint16)x, (Sint16)y, 0, 0};
    SDL_BlitSurface(surf, &src, gfx->screen, &dst);
    SDL_FreeSurface(surf);
    return true;
}

void gfx_text_rot180(Gfx *gfx, TTF_Font *font, const char *text, int x, int y, SDL_Color color)
{
    if (!text[0])
        return;
    SDL_Surface *surf = TTF_RenderUTF8_Blended(font, text, color);
    if (!surf) {
        LOG_WARN("TTF_RenderUTF8_Blended failed: %s", TTF_GetError());
        return;
    }
    gfx_rotate180(surf); /* 32bpp with per-pixel alpha; rotation keeps alpha */
    gfx_blit(gfx, surf, x - surf->w / 2, y);
    SDL_FreeSurface(surf);
}

void gfx_digits_centered_on(SDL_Surface *dst, TTF_Font *font, const char *text, int cx, int cy,
                            SDL_Color color)
{
    /* Use the glyph '0' to find where digits sit inside the rendered line:
     * its top is (ascent - maxy) below the surface top, and it is (maxy - miny) tall. */
    int minx, maxx, miny, maxy, adv;
    int top_y;
    if (TTF_GlyphMetrics(font, '0', &minx, &maxx, &miny, &maxy, &adv) == 0)
        top_y = cy - (TTF_FontAscent(font) - maxy) - (maxy - miny) / 2;
    else
        top_y = cy - TTF_FontHeight(font) / 2;
    gfx_text_on(dst, font, text, cx, top_y, color, 0, ALIGN_CENTER);
}
