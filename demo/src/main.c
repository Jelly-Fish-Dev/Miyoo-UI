/* Miyoo-UI demo: one page per part of the library.
 *
 *   L1 / L2   previous page
 *   R1 / R2   next page
 *   MENU      exit
 */

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <SDL/SDL.h>

#include "mui/color.h"
#include "mui/gfx.h"
#include "mui/glyphs.h"
#include "mui/image.h"
#include "mui/input.h"
#include "mui/log.h"
#include "mui/theme.h"

#define LENNA_PATH "res/images/lenna.png"

#define CONTENT_Y 64  /* below the header rule */
#define FOOTER_Y  450

typedef struct {
    int page;
    Button last;              /* last button pressed (input page) */
    int presses[BTN_COUNT];
    SDL_Surface *lenna_big;   /* 300x300 */
    SDL_Surface *lenna_small; /* 120x120 */
    SDL_Surface *lenna_wide;  /* fitted into a 220x90 box */
    SDL_Surface *panel;       /* off-screen buffer for the rotation page */
    TTF_Font *glyphs;         /* button icons for the footer; NULL falls back to words */
} Demo;

/* ------------------------------------------------------------ pages */

static void page_hello(Demo *d, Gfx *g)
{
    (void)d;
    gfx_text(g, g->font_big, "Hello, world!", SCREEN_W / 2, 84, C_WHITE, 0, ALIGN_CENTER);

    gfx_text(g, g->font, "gfx->font (26 px)", 40, 170, C_WHITE, 0, ALIGN_LEFT);
    gfx_text(g, g->font_small, "gfx->font_small (19 px)", 40, 206, C_DIM, 0, ALIGN_LEFT);
    TTF_Font *f36 = gfx_font(g, 36);
    if (f36)
        gfx_text(g, f36, "gfx_font(g, 36)", SCREEN_W - 40, 176, C_ACCENT, 0, ALIGN_RIGHT);

    /* the three alignments, anchored on one vertical line */
    int ax = SCREEN_W / 2;
    SDL_Rect guide = {(Sint16)ax, 250, 1, 96};
    gfx_fill(g, &guide, C_RULE);
    gfx_text(g, g->font_small, "ALIGN_LEFT", ax, 254, C_WHITE, 0, ALIGN_LEFT);
    gfx_text(g, g->font_small, "ALIGN_CENTER", ax, 284, C_WHITE, 0, ALIGN_CENTER);
    gfx_text(g, g->font_small, "ALIGN_RIGHT", ax, 314, C_WHITE, 0, ALIGN_RIGHT);

    /* truncation with max_w */
    SDL_Rect box = {40, 370, 280, 30};
    gfx_fill(g, &box, C_PANEL);
    gfx_text(g, g->font_small, "This sentence is far too long for its box", box.x + 8, box.y + 4,
             C_WHITE, box.w - 16, ALIGN_LEFT);
    gfx_text(g, g->font_small, "<- max_w cuts with \"...\"", 336, 374, C_DIM, 0, ALIGN_LEFT);
}

static void page_colors(Demo *d, Gfx *g)
{
    (void)d;
    static const struct {
        const char *name;
        const SDL_Color *c;
    } swatches[] = {
        {"C_WHITE", &C_WHITE}, {"C_DIM", &C_DIM},       {"C_DARK", &C_DARK},
        {"C_ACCENT", &C_ACCENT}, {"C_DANGER", &C_DANGER}, {"C_HIGHLIGHT", &C_HIGHLIGHT},
        {"C_BG", &C_BG},       {"C_PANEL", &C_PANEL},   {"C_RULE", &C_RULE},
    };
    for (int i = 0; i < 9; i++) {
        SDL_Rect r = {(Sint16)(40 + (i % 3) * 190), (Sint16)(CONTENT_Y + 8 + (i / 3) * 72), 180, 62};
        gfx_fill(g, &r, *swatches[i].c);
        gfx_border_on(g->screen, &r, 1, C_RULE);
        /* color_text_on picks dark or light text for the fill */
        gfx_text(g, g->font_small, swatches[i].name, r.x + r.w / 2, r.y + 20,
                 color_text_on(*swatches[i].c), 0, ALIGN_CENTER);
    }

    gfx_text(g, g->font_small, "color_mix(C_BG, C_ACCENT, t) and (C_DANGER, C_WHITE, t)", 40, 296,
             C_DIM, SCREEN_W - 80, ALIGN_LEFT);
    const int steps = 28, step_w = 20;
    for (int i = 0; i < steps; i++) {
        int t = i * 256 / (steps - 1);
        SDL_Rect a = {(Sint16)(40 + i * step_w), 326, step_w, 40};
        SDL_Rect b = {(Sint16)(40 + i * step_w), 376, step_w, 40};
        gfx_fill(g, &a, color_mix(C_BG, C_ACCENT, t));
        gfx_fill(g, &b, color_mix(C_DANGER, C_WHITE, t));
    }
}

/* Blits surf centred in box, or "missing" text if it didn't load. */
static void draw_in_box(Gfx *g, SDL_Surface *surf, SDL_Rect box, const char *caption)
{
    gfx_fill(g, &box, C_PANEL);
    if (surf)
        gfx_blit(g, surf, box.x + (box.w - surf->w) / 2, box.y + (box.h - surf->h) / 2);
    else
        gfx_text(g, g->font_small, "not loaded", box.x + box.w / 2, box.y + box.h / 2 - 12, C_DANGER,
                 box.w - 8, ALIGN_CENTER);
    gfx_text(g, g->font_small, caption, box.x + box.w / 2, box.y + box.h + 6, C_DIM, 0,
             ALIGN_CENTER);
}

static void page_images(Demo *d, Gfx *g)
{
    SDL_Rect big = {40, CONTENT_Y + 10, 300, 300};
    SDL_Rect small = {400, CONTENT_Y + 10, 120, 120};
    SDL_Rect wide = {370, CONTENT_Y + 190, 220, 90};
    draw_in_box(g, d->lenna_big, big, "image_load_fit(path, 300, 300)");
    draw_in_box(g, d->lenna_small, small, "fit 120 x 120");
    draw_in_box(g, d->lenna_wide, wide, "fit 220 x 90: aspect kept");
    gfx_border_on(g->screen, &wide, 1, C_RULE);
}

/* One square "player" panel, drawn upright into d->panel. */
static void draw_panel(Demo *d, Gfx *g, SDL_Color fill, const char *label)
{
    SDL_Surface *p = d->panel;
    SDL_Rect all = {0, 0, (Uint16)p->w, (Uint16)p->h};
    gfx_fill_on(p, &all, fill);
    gfx_border_on(p, &all, 3, C_RULE);
    gfx_digits_centered_on(p, g->font_big, "20", p->w / 2, p->h / 2, color_text_on(fill));
    gfx_text_on(p, g->font_small, label, p->w / 2, p->h - 34, color_text_on(fill), 0, ALIGN_CENTER);
}

static void page_rotation(Demo *d, Gfx *g)
{
    if (!d->panel)
        return;
    int y = CONTENT_Y + 40;
    draw_panel(d, g, C_DANGER, "gfx_rotate180");
    gfx_rotate180(d->panel);
    gfx_blit(g, d->panel, SCREEN_W / 2 - 10 - d->panel->w, y);

    draw_panel(d, g, C_ACCENT, "upright");
    gfx_blit(g, d->panel, SCREEN_W / 2 + 10, y);

    /* text for someone sitting across the table, above the panels */
    gfx_text_rot180(g, g->font_small, "gfx_text_rot180: readable from the other side",
                    SCREEN_W / 2, CONTENT_Y + 6, C_DIM);
    gfx_text(g, g->font_small, "gfx_digits_centered_on centres the digits' ink, not the line",
             SCREEN_W / 2, y + d->panel->h + 14, C_DIM, SCREEN_W - 40, ALIGN_CENTER);
}

static void page_input(Demo *d, Gfx *g)
{
    const char *last = d->last == BTN_NONE ? "press a button" : input_button_name(d->last);
    gfx_text(g, g->font_big, last, SCREEN_W / 2, CONTENT_Y + 4, d->last == BTN_NONE ? C_DIM : C_ACCENT,
             SCREEN_W - 40, ALIGN_CENTER);

    for (int b = 0; b < BTN_COUNT; b++) {
        SDL_Rect r = {(Sint16)(28 + (b % 4) * 148), (Sint16)(CONTENT_Y + 90 + (b / 4) * 66), 140, 56};
        bool on = b == (int)d->last;
        gfx_fill(g, &r, on ? C_HIGHLIGHT : C_PANEL);
        SDL_Color fg = on ? C_DARK : C_WHITE;
        gfx_text(g, g->font_small, input_button_name((Button)b), r.x + 10, r.y + 6, fg, 0,
                 ALIGN_LEFT);
        char n[16];
        snprintf(n, sizeof n, "%d", d->presses[b]);
        gfx_text(g, g->font_small, n, r.x + r.w - 10, r.y + 28, on ? C_DARK : C_DIM, 0, ALIGN_RIGHT);
    }
}

static const struct {
    const char *title;
    void (*render)(Demo *, Gfx *);
} PAGES[] = {
    {"Hello world", page_hello},
    {"Colours", page_colors},
    {"Images", page_images},
    {"Rotation", page_rotation},
    {"Input", page_input},
};
#define PAGE_COUNT ((int)(sizeof PAGES / sizeof PAGES[0]))

/* ------------------------------------------------------------ app */

static void render(Demo *d, Gfx *g)
{
    gfx_fill(g, NULL, C_BG);

    gfx_text(g, g->font, "Miyoo-UI", 16, 10, C_WHITE, 0, ALIGN_LEFT);
    gfx_text(g, g->font, PAGES[d->page].title, SCREEN_W / 2, 10, C_ACCENT, 0, ALIGN_CENTER);
    char pos[16];
    snprintf(pos, sizeof pos, "%d / %d", d->page + 1, PAGE_COUNT);
    gfx_text(g, g->font_small, pos, SCREEN_W - 16, 16, C_DIM, 0, ALIGN_RIGHT);
    SDL_Rect rule = {16, CONTENT_Y - 14, SCREEN_W - 32, 1};
    gfx_fill(g, &rule, C_RULE);

    PAGES[d->page].render(d, g);

    glyph_hint(g, g->font_small, d->glyphs, "{L1}{L2} previous  ·  {R1}{R2} next  ·  MENU exit",
               SCREEN_W / 2, FOOTER_Y, C_DIM);
    gfx_present(g);
}

/* Returns false when the demo should exit. */
static bool handle(Demo *d, Button btn)
{
    if (btn != BTN_NONE) {
        d->last = btn;
        d->presses[btn]++;
    }
    switch (btn) {
    case BTN_L1:
    case BTN_L2: d->page = (d->page - 1 + PAGE_COUNT) % PAGE_COUNT; break;
    case BTN_R1:
    case BTN_R2: d->page = (d->page + 1) % PAGE_COUNT; break;
    case BTN_MENU: return false;
    default: break;
    }
    return true;
}

int main(void)
{
    LOG_INFO("Miyoo-UI demo starting");

    Gfx gfx;
    if (!gfx_init(&gfx)) {
        gfx_quit(&gfx);
        return 1;
    }

    Demo d;
    memset(&d, 0, sizeof d);
    d.last = BTN_NONE;
    d.lenna_big = image_load_fit(LENNA_PATH, 300, 300);
    d.lenna_small = image_load_fit(LENNA_PATH, 120, 120);
    d.lenna_wide = image_load_fit(LENNA_PATH, 220, 90);
    d.panel = gfx_new_surface(&gfx, 240, 240);
    d.glyphs = glyph_font_open(GLYPH_SIZE_SMALL);
    if (!d.lenna_big)
        LOG_WARN("couldn't load %s", LENNA_PATH);

    bool running = true, dirty = true;
    while (running) {
        SDL_Event ev;
        while (running && SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) {
                running = false;
            } else if (ev.type == SDL_KEYDOWN) {
                running = handle(&d, input_button_from_key(ev.key.keysym.sym));
                dirty = true;
            }
        }
        if (dirty && running) {
            render(&d, &gfx);
            dirty = false;
        }
        SDL_Delay(16);
    }

    LOG_INFO("exiting");
    SDL_Surface *surfs[] = {d.lenna_big, d.lenna_small, d.lenna_wide, d.panel};
    for (size_t i = 0; i < sizeof surfs / sizeof surfs[0]; i++)
        if (surfs[i]) SDL_FreeSurface(surfs[i]);
    if (d.glyphs)
        TTF_CloseFont(d.glyphs);
    gfx_quit(&gfx);
    return 0;
}
