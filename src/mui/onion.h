#ifndef MUI_ONION_H
#define MUI_ONION_H

#include <stdbool.h>
#include <stddef.h>

/* The current OnionOS theme, read the same way Onion's own apps do
 * (Onion src/common/theme/load.h and config.h):
 *
 *   theme folder  "theme" in <sdcard>/system.json; "./" or a missing folder
 *                 means <sdcard>/miyoo/app/
 *   theme config  <theme>/config.json, then the per-profile overrides in
 *                 <sdcard>/Saves/CurrentProfile/theme/config.json
 *   text font     "list" -> "font", else "title" -> "font"; relative paths are
 *                 inside the theme folder
 *   fallback      /customer/app/Exo-2-Bold-Italic.ttf (the stock Miyoo font)
 *
 * sdcard may be NULL for /mnt/SDCARD. Paths in system.json that start with
 * /mnt/SDCARD are moved under sdcard, so a copied card tree works too. */

/* Writes the current theme folder (ending in '/') to out. */
void onion_theme_dir(const char *sdcard, char *out, size_t n);

/* Writes the path of the theme's list font to out. Returns false (out = "")
 * if neither the theme's font nor Onion's fallback font exists; the caller
 * then keeps its own font (gfx_init_font falls back to FONT_PATH). */
bool onion_theme_font(const char *sdcard, char *out, size_t n);

#endif
