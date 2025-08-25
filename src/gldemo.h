/*
 * gldemo - experiment to render the solar system
 *
 * ephembra is a tiny ephemeris library for the JPL DE440 Ephemeris
 *
 * Copyright (c) 2025 Michael Clark <michaeljclark@mac.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct lv_app lv_app;
typedef struct lv_oid lv_oid;
typedef struct lv_oid_idx lv_oid_idx;
typedef struct lv_sign lv_sign;

struct lv_app
{
    GLFWwindow* window;
    lv_context* ctx_nanovg;
    lv_context* ctx_buffer;
    lv_context* ctx_xform;
    mat4x4 m_mvp;
    mat4x4 m_inv;
    vec3 rot;
    vec3 trans;
    vec2f mouse;
    vec2f origin;
    float zoom;
    vec2f last_mouse;
    float last_zoom;
    double rot_tjd;
    size_t rot_oid;
    double sel_tjd;
    size_t sel_oid;
    size_t steps;
    size_t divs;
    int sjd, sjdf;
    int ejd, ejdf;
    int cjd, cjdf;
    int ljd, ljdf;
    double jd;
    lv_date date;
    bool playback;
    bool precession;
    bool cartoon;
    bool sym_legend;
    bool name_legend;
    bool dist_legend;
    bool grid_layer;
    bool zodiac_layer;
    int grid_steps;
    int font_size;
    int symbol_size;
    float ui_scale;
    float grid_scale;
    float trail_width;
    float line_width;
    float planet_scale;
    float symbol_offset;
    float zodiac_offset;
    float zodiac_scale;
    double *eph;
    int *images;
    int font;
    ephem_ctx ctx;
};

struct lv_oid_idx
{
    size_t oid;
    vec3 pos;
};

struct lv_oid
{
    size_t oid;         /* object ID */
    const char *symbol; /* astronomical symbol */
    const char *name;   /* object name */
    double dist;        /* average semi-major axis (km) */
    double diameter;    /* mean diameter (km) */
    double orbit;       /* sidereal orbit period (days) */
    lv_color color;     /* orbital trail color (RGBA, 0-1) */
};

struct lv_sign
{
    const char *symbol;
    const char *name;
    lv_color color;
};

extern const char* ephembra_data_file;
extern const char* ephembra_sans_font;
extern const char* ephembra_mono_font;
extern const char* ephembra_awes_font;
extern const char* ephembra_image_tmpl;

extern lv_oid data[];
extern size_t oid_count;
extern lv_sign signs[];
extern size_t sign_count;

void lv_app_init(lv_app *app);
void lv_vg_uinit(lv_app* app);
void lv_vg_udestroy(lv_app* app);
void lv_app_imgui_init(lv_app *app);
void lv_app_imgui_destroy();
void lv_update_date(lv_app *app);
void lv_current_date(lv_app *app);
void lv_ephem_init(lv_app *app);
void lv_ephem_destroy(lv_app *app);
void lv_ephem_calc(lv_app *app, double jd);
void lv_iau2006_dynamic_matrix(lv_app *app, mat4x4 m);
void lv_iau2006_dynamic_basis(lv_app *app, vec3 x0, vec3 y0, vec3 z0);

void lv_grid_3d(lv_app *app, lv_context* ctx);
void lv_zodiac_3d(lv_app *app, lv_context* ctx);
void lv_zodiac_2d(lv_app *app, lv_context* ctx, float w, float h);
void lv_planets_3d(lv_app *app, lv_context* ctx);
void lv_planets_2d(lv_app *app, lv_context* ctx, float w, float h);
void lv_render(lv_app* app, float w, float h, float r);

void lv_date_picker(lv_date *date);
void lv_font_size(const char *label, int *font_size);
void lv_imgui(lv_app* app, float w, float h, float r);
void lv_app_main(int argc, char **argv);


static inline size_t lv_steps(lv_app *app) { return app->steps; }

static inline size_t lv_edges(lv_app *app) { return app->steps / app->divs; }

static inline double* lv_ephem_object(lv_app *app, size_t oid, size_t idx)
{
    return app->eph + oid * app->steps * 3 + (idx % app->steps) * 3;
}

static inline void lv_ephem_object_vec3(lv_app *app, size_t oid, size_t idx,
    vec3 r, float s)
{
    double *o = lv_ephem_object(app, oid, idx);
    r[0] = (float)o[0] * s;
    r[1] = (float)o[1] * s;
    r[2] = (float)o[2] * s;
}

static inline void lv_ephem_object_shift_vec3(lv_app *app, size_t oid, size_t idx,
    vec3 r, vec3 b, float f, float s)
{
    lv_ephem_object_vec3(app, oid, idx, r, s);
    vec3_multiply_add(r, b, f, r);
}

#ifdef __cplusplus
}
#endif
