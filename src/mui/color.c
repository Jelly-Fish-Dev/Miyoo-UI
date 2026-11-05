#include "mui/color.h"

#include "mui/theme.h"

SDL_Color color_text_on(SDL_Color bg)
{
    /* Rec. 601 luma, 0..255. */
    int luma = (299 * bg.r + 587 * bg.g + 114 * bg.b) / 1000;
    return luma > 150 ? C_DARK : C_WHITE;
}

static Uint8 mix(Uint8 a, Uint8 b, int t256)
{
    return (Uint8)(a + ((b - a) * t256) / 256);
}

SDL_Color color_mix(SDL_Color from, SDL_Color to, int t256)
{
    if (t256 < 0) t256 = 0;
    if (t256 > 256) t256 = 256;
    SDL_Color c = {mix(from.r, to.r, t256), mix(from.g, to.g, t256), mix(from.b, to.b, t256), 0};
    return c;
}
