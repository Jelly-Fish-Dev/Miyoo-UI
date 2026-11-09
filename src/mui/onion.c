#include "mui/onion.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "cJSON.h"
#include "mui/log.h"

#define DEVICE_SDCARD   "/mnt/SDCARD"
#define FALLBACK_DIR    "miyoo/app/"
#define OVERRIDES       "Saves/CurrentProfile/theme/config.json"
#define FALLBACK_FONT   "/customer/app/Exo-2-Bold-Italic.ttf"
#define MAX_JSON_BYTES  (256 * 1024)

static bool is_file(const char *path)
{
    struct stat st;
    return stat(path, &st) == 0 && S_ISREG(st.st_mode);
}

static bool is_dir(const char *path)
{
    struct stat st;
    return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}

/* Parses a JSON file, or NULL if it's missing, too big or invalid. */
static cJSON *read_json(const char *path)
{
    FILE *f = fopen(path, "rb");
    if (!f)
        return NULL;
    char *buf = malloc(MAX_JSON_BYTES + 1);
    size_t len = buf ? fread(buf, 1, MAX_JSON_BYTES, f) : 0;
    fclose(f);
    if (!buf)
        return NULL;
    buf[len] = '\0';
    cJSON *root = cJSON_Parse(buf);
    free(buf);
    if (!root)
        LOG_WARN("onion: %s is not valid JSON", path);
    return root;
}

/* root[obj][key] as a non-empty string, or NULL. */
static const char *json_str(const cJSON *root, const char *obj, const char *key)
{
    const cJSON *o = obj ? cJSON_GetObjectItemCaseSensitive(root, obj) : root;
    const cJSON *v = cJSON_GetObjectItemCaseSensitive(o, key);
    return (cJSON_IsString(v) && v->valuestring[0]) ? v->valuestring : NULL;
}

/* Moves a device path (/mnt/SDCARD/...) under sdcard. */
static void on_card(const char *sdcard, const char *path, char *out, size_t n)
{
    size_t dl = strlen(DEVICE_SDCARD);
    if (strcmp(sdcard, DEVICE_SDCARD) != 0 && strncmp(path, DEVICE_SDCARD, dl) == 0 &&
        (path[dl] == '/' || path[dl] == '\0'))
        snprintf(out, n, "%s%s", sdcard, path + dl);
    else
        snprintf(out, n, "%s", path);
}

void onion_theme_dir(const char *sdcard, char *out, size_t n)
{
    if (!sdcard)
        sdcard = DEVICE_SDCARD;
    char path[600];
    snprintf(path, sizeof path, "%s/system.json", sdcard);

    out[0] = '\0';
    cJSON *sys = read_json(path);
    const char *theme = sys ? json_str(sys, NULL, "theme") : NULL;
    if (theme && strcmp(theme, "./") != 0)
        on_card(sdcard, theme, out, n);
    cJSON_Delete(sys);

    if (!out[0] || !is_dir(out))
        snprintf(out, n, "%s/" FALLBACK_DIR, sdcard);
    size_t len = strlen(out);
    if (len + 1 < n && out[len - 1] != '/')
        snprintf(out + len, n - len, "/");
}

/* The font named in one config file (list, else title), or NULL. */
static const char *config_font(const cJSON *cfg)
{
    const char *f = json_str(cfg, "list", "font");
    return f ? f : json_str(cfg, "title", "font");
}

bool onion_theme_font(const char *sdcard, char *out, size_t n)
{
    if (!sdcard)
        sdcard = DEVICE_SDCARD;
    char dir[512], path[600];
    onion_theme_dir(sdcard, dir, sizeof dir);

    /* The theme's own config.json, then the profile's overrides on top. */
    snprintf(path, sizeof path, "%sconfig.json", dir);
    cJSON *cfg = read_json(path);
    snprintf(path, sizeof path, "%s/" OVERRIDES, sdcard);
    cJSON *over = read_json(path);
    /* Onion applies overrides without fallbacks: only their own list font counts. */
    const char *font = over ? json_str(over, "list", "font") : NULL;
    if (!font && cfg)
        font = config_font(cfg);

    char cand[600] = "";
    if (font) {
        if (font[0] == '/')
            on_card(sdcard, font, cand, sizeof cand);
        else
            snprintf(cand, sizeof cand, "%s%s", dir, font);
    }
    cJSON_Delete(cfg);
    cJSON_Delete(over);

    if (cand[0] && is_file(cand)) {
        snprintf(out, n, "%s", cand);
    } else if (is_file(FALLBACK_FONT)) {
        if (cand[0])
            LOG_WARN("onion: theme font %s missing, using %s", cand, FALLBACK_FONT);
        snprintf(out, n, "%s", FALLBACK_FONT);
    } else {
        out[0] = '\0';
        return false;
    }
    LOG_INFO("onion: theme %s, font %s", dir, out);
    return true;
}
