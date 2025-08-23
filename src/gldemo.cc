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

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <time.h>

#define _USE_MATH_DEFINES
#include <math.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_MODULE_H
#include FT_GLYPH_H
#include FT_OUTLINE_H

#include "demolib.h"

#include "nanovg.h"
#define NANOVG_GLES3_IMPLEMENTATION
#include "nanovg_gl.h"
#include "nanovg_gl_utils.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "linmath.h"
#include "gl2_util.h"
#include "ephembra.h"

#include "lv_model.h"
#include "lv_ops_nanovg.h"
#include "lv_ops_buffer.h"
#include "lv_ops_xform.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#define countof(arr) (sizeof(arr)/sizeof(arr[0]))

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

static lv_oid data[10] = {
    { 0, "☉", "Sun",        1000000,    695700,      0.000, { 1.00, 0.84, 0.00, 1.0 } }, // golden-yellow photosphere
    { 1, "☿", "Mercury",   57909227,      4879,     87.969, { 0.60, 0.60, 0.60, 1.0 } }, // mid-grey, rocky
    { 2, "♀", "Venus",    108209475,     12104,    224.701, { 0.96, 0.89, 0.70, 1.0 } }, // pale golden cream
    { 3, "♁", "Earth",    149598023,     12742,    365.256, { 0.27, 0.55, 0.68, 1.0 } }, // blue-green oceans/land
    { 4, "♂", "Mars",     227939200,      6779,    686.980, { 0.70, 0.40, 0.35, 1.0 } }, // reddish-orange dusty soil
    { 5, "♃", "Jupiter",  778340821,    139820,   4332.589, { 0.87, 0.72, 0.53, 1.0 } }, // beige bands with light brown
    { 6, "♄", "Saturn",  1426666422,    116460,  10759.220, { 0.93, 0.85, 0.63, 1.0 } }, // pale yellow-brown
    { 7, "♅", "Uranus",  2870658186,     50724,  30687.000, { 0.56, 0.75, 0.82, 1.0 } }, // pale cyan
    { 8, "♆", "Neptune", 4498396441,     49244,  60190.000, { 0.28, 0.35, 0.68, 1.0 } }, // deep azure blue
    { 9, "♇", "Pluto",   5906376272,      2377,  90560.000, { 0.72, 0.62, 0.57, 1.0 } }  // light brown-grey, icy patches
};

static lv_sign signs[12] = {
    { "♈", "Aries",       { 0.937, 0.325, 0.314, 1.000 } }, // U+2648
    { "♉", "Taurus",      { 1.000, 0.439, 0.263, 1.000 } }, // U+2649
    { "♊", "Gemini",      { 1.000, 0.655, 0.149, 1.000 } }, // U+264A
    { "♋", "Cancer",      { 1.000, 0.800, 0.196, 1.000 } }, // U+264B
    { "♌", "Leo",         { 0.988, 0.894, 0.220, 1.000 } }, // U+264C
    { "♍", "Virgo",       { 0.612, 0.800, 0.396, 1.000 } }, // U+264D
    { "♎", "Libra",       { 0.400, 0.733, 0.416, 1.000 } }, // U+264E
    { "♏", "Scorpio",     { 0.149, 0.651, 0.604, 1.000 } }, // U+264F
    { "♐", "Sagittarius", { 0.980, 0.463, 0.824, 1.000 } }, // U+2650
    { "♑", "Capricorn",   { 0.494, 0.341, 0.761, 1.000 } }, // U+2651
    { "♒", "Aquarius",    { 0.671, 0.278, 0.737, 1.000 } }, // U+2652
    { "♓", "Pisces",      { 0.925, 0.251, 0.478, 1.000 } }  // U+2653
};

static const int font_sizes[] = {
    8, 9, 10, 11, 12, 14, 16, 18, 20, 22, 24, 26, 28, 36, 48, 72
};

static const char* ephembra_data_file = "build/data/DE440Coeff.bin";
static const char* ephembra_sans_font = "resources/fonts/DejaVuSans.ttf";
static const char* ephembra_mono_font = "resources/fonts/DejaVuSansMono.ttf";
static const char* ephembra_awes_font = "resources/fonts/fontawesome-webfont.ttf";
static const char* ephembra_image_tmpl = "resources/images/%s.png";

static const float min_zoom = 2.0f, max_zoom = 2048.0f;
static const float global_scale = 1e12;

static int opt_help;
static int opt_width = 1280;
static int opt_height = 720;

static lv_color grey;
static lv_color white;

/*
 * gldemo
 */

int lv_oid_zsort(const void *p1, const void *p2)
{
    const lv_oid_idx *a = (lv_oid_idx *)p1;
    const lv_oid_idx *b = (lv_oid_idx *)p2;
    if (a->pos[2] < b->pos[2]) return -1;
    else if (a->pos[2] > b->pos[2]) return 1;
    else return 0;
}

static void lv_init_colors()
{
    grey      = { 0.4980, 0.4980, 0.4980, 1.0 };
    white     = { 0.8157, 0.8157, 0.8157, 1.0 };
}

static void lv_app_init(lv_app *app)
{
    memset(app, 0, sizeof(app));
    app->zoom = 16.0f;
    app->rot[0] = 65.0f;
    app->rot_oid = -1;
    app->rot_tjd = NAN;
    app->sel_oid = -1;
    app->sel_tjd = NAN;
    app->steps = 360;
    app->divs = 36;
    app->sjd = 2341972 + 500; /* 1-JAN-1700 */
    app->ejd = 2597640 - 500; /* 31-DEC-2399 */
    app->sjdf = -500;
    app->ejdf = 500;
    app->cjdf = 0;
    app->playback = 0;
    app->precession = 1;
    app->cartoon = 1;
    app->sym_legend = 1;
    app->name_legend = 0;
    app->dist_legend = 0;
    app->font_size = 12;
    app->symbol_size = 16;
    app->ui_scale = 2.0f;
    app->grid_layer = 0;
    app->grid_steps = 10;
    app->grid_scale = 9.0f;
    app->trail_width = 6.0f;
    app->line_width = 2.0f;
    app->planet_scale = 2.5f;
    app->zodiac_layer = 0;
    app->symbol_offset = -0.05f;
    app->zodiac_offset = 0.0f;
    app->zodiac_scale = 9.0f;
    lv_init_colors();
}

static void lv_vg_uinit(lv_app* app)
{
    app->ctx_nanovg = (lv_context*)calloc(1, sizeof(lv_context));
    app->ctx_buffer = (lv_context*)calloc(1, sizeof(lv_context));
    app->ctx_xform = (lv_context*)calloc(1, sizeof(lv_context));
    lv_vg_init(app->ctx_nanovg, &lv_nanovg_vg_ops, NULL);
    lv_vg_init(app->ctx_buffer, &lv_buffer_vg_ops, NULL);
    lv_vg_init(app->ctx_xform, &lv_xform_vg_ops, app->ctx_nanovg);
}

static void lv_vg_udestroy(lv_app* app)
{
    lv_vg_destroy(app->ctx_nanovg);
    lv_vg_destroy(app->ctx_buffer);
    lv_vg_destroy(app->ctx_xform);
    free(app->ctx_nanovg);
    free(app->ctx_buffer);
    free(app->ctx_xform);
}

static void lv_update_date(lv_app *app)
{
    app->jd = app->cjd + app->cjdf + 0.5;
    app->date = lv_julian_to_date(app->jd);
}

static void lv_current_date(lv_app *app)
{
    time_t t = time(NULL);
    struct tm *tm = gmtime(&t);
    lv_date d = { tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday };
    app->cjd = (int)lv_date_to_julian(d);
    lv_update_date(app);
}

static void lv_ephem_init(lv_app *app)
{
    NVGcontext *vg = *(NVGcontext**)app->ctx_nanovg->priv;

    lv_current_date(app);
    de440_create_ephem(&app->ctx, ephembra_data_file);
    app->eph = (double*)malloc(countof(data) * app->steps * sizeof(double) * 3);
    app->images = (int*)malloc(countof(data) * sizeof(int));
    nvgCreateFont(vg, "mono", ephembra_mono_font);
    nvgCreateFont(vg, "sans", ephembra_sans_font);

    for (size_t oid = 0; oid < countof(data); oid++)
    {
        char path[64];
        snprintf(path, sizeof(path), ephembra_image_tmpl, data[oid].name);
        app->images[oid] = nvgCreateImage(vg, path, NVG_IMAGE_GENERATE_MIPMAPS);
    }
}

static void lv_ephem_destroy(lv_app *app)
{
    NVGcontext *vg = *(NVGcontext**)app->ctx_nanovg->priv;

    for (size_t oid = 0; oid < countof(data); oid++) {
        nvgDeleteImage(vg, app->images[oid]);
    }

    de440_destroy_ephem(&app->ctx);

    free(app->images);
    free(app->eph);
}

static size_t lv_steps(lv_app *app) { return app->steps; }

static size_t lv_edges(lv_app *app) { return app->steps / app->divs; }

static double* lv_ephem_object(lv_app *app, size_t oid, size_t idx)
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

static void lv_ephem_calc(lv_app *app, double jd)
{
    for (size_t oid = 1; oid < countof(data); oid++)
    {
        for (size_t i = 0; i < app->steps; i++)
        {
            double interval = data[oid].orbit / app->steps;
            double tjd = jd - (i * interval);
            size_t row = de440_find_row(&app->ctx, tjd);
            double *o = lv_ephem_object(app, oid, i);
            de440_ephem_obj(&app->ctx, tjd, row, oid, o);
        }
    }
}

static inline float lv_oid_scale(bool cartoon, size_t oid)
{
    float r = (float)(data[oid].dist / data[ephem_id_Pluto].dist);
    return cartoon ? ((oid + 1.0f) / countof(data)) / r : 1.0f;
}

static void lv_iau2006_dynamic_matrix(lv_app *app, mat4x4 m)
{
    if (app->precession) {
        lv_iau2006_combined_matrix(m, app->jd);
    } else {
        lv_iau2006_obliquity_matrix(m, app->jd);
    }
}

static void lv_iau2006_dynamic_basis(lv_app *app, vec3 x0, vec3 y0, vec3 z0)
{
    if (app->precession) {
        lv_iau2006_combined_basis(x0, y0, z0, app->jd);
    } else {
        lv_iau2006_obliquity_basis(x0, y0, z0, app->jd);
    }
}

static inline lv_color lv_oid_color(lv_app* app, size_t oid, float alpha)
{
    lv_color color = lv_color_af(data[oid].color, alpha);
    if (oid == app->rot_oid) {
        return lv_color_adjust(color, 1.5, 1.5);
    } else {
        return color;
    }
}

static void lv_grid_3d(lv_app *app, lv_context* ctx)
{
    float f = global_scale * app->zodiac_offset;
    float g = global_scale * app->grid_scale;
    vec4 x0, y0, z0;

    lv_iau2006_dynamic_basis(app, x0, y0, z0);

    int n = app->grid_steps;
    float step = g * (1.0f / app->grid_steps);

    lv_vg_stroke_width(ctx, app->line_width);
    lv_vg_stroke_color(ctx, grey);
    for (int i = -n; i <= n; i++)
    {
        vec3 p0 = {
            -z0[0] * f + y0[0] * -g + x0[0] * (i * step),
            -z0[1] * f + y0[1] * -g + x0[1] * (i * step),
            -z0[2] * f + y0[2] * -g + x0[2] * (i * step)
        };
        vec3 p1 = {
            -z0[0] * f + y0[0] *  g + x0[0] * (i * step),
            -z0[1] * f + y0[1] *  g + x0[1] * (i * step),
            -z0[2] * f + y0[2] *  g + x0[2] * (i * step)
        };

        lv_vg_begin_path(ctx);
        lv_vg_3d_move_to(ctx, lv_point_3d(p0[0], p0[1], p0[2]));
        lv_vg_3d_line_to(ctx, lv_point_3d(p1[0], p1[1], p1[2]));
        lv_vg_stroke(ctx);

        vec3 p2 = {
            -z0[0] * f + x0[0] * -g + y0[0] * (i * step),
            -z0[1] * f + x0[1] * -g + y0[1] * (i * step),
            -z0[2] * f + x0[2] * -g + y0[2] * (i * step)
        };
        vec3 p3 = {
            -z0[0] * f + x0[0] *  g + y0[0] * (i * step),
            -z0[1] * f + x0[1] *  g + y0[1] * (i * step),
            -z0[2] * f + x0[2] *  g + y0[2] * (i * step)
        };

        lv_vg_begin_path(ctx);
        lv_vg_3d_move_to(ctx, lv_point_3d(p2[0], p2[1], p2[2]));
        lv_vg_3d_line_to(ctx, lv_point_3d(p3[0], p3[1], p3[2]));
        lv_vg_stroke(ctx);
    }
}

static void lv_zodiac_3d(lv_app *app, lv_context* ctx)
{
    float f = global_scale * app->zodiac_offset;
    float g = global_scale * app->zodiac_scale;
    float s = lv_oid_scale(app->cartoon, ephem_id_EarthMoon);
    vec3 x0, y0, z0, p0;

    lv_iau2006_dynamic_basis(app, x0, y0, z0);
    lv_ephem_object_shift_vec3(app, ephem_id_EarthMoon, 0, p0, z0, f, s);

    for (int i = 0; i < 12; i++)
    {
        lv_vg_stroke_color(ctx, white);
        lv_vg_stroke_width(ctx, app->line_width);
        lv_vg_fill_color(ctx, lv_rgbaf(0.1f, 0.1f, 0.1f, 1.0f));
        lv_vg_begin_path(ctx);
        lv_vg_3d_move_to(ctx, lv_point_3d(p0[0], p0[1], p0[2]));
        for (int j = 0; j < 31; j++)
        {
            float theta = 2.0f * M_PI * (i*30.0f+j) / 360.0f;
            vec3 p1;
            vec3_sincos_basis(p1, theta, x0, y0, z0, f, g);
            lv_vg_3d_line_to(ctx, lv_point_3d(p1[0], p1[1], p1[2]));
        }
        lv_vg_close_path(ctx);
        lv_vg_fill(ctx);
        lv_vg_stroke(ctx);
    }

    lv_iau2006_dynamic_basis(app, x0, y0, z0);

    for (size_t oid = 0; oid < countof(data); oid++)
    {
        float s = lv_oid_scale(app->cartoon, oid);
        vec3 p0, p1, p2, p3;

        lv_ephem_object_vec3(app, oid, 0, p0, s);
        if (p0[0] != p0[0]) continue;
        vec3_project_to_basis(p1, p0, x0, y0);
        vec3_multiply_add(p2, z0, f, p1);
        lv_ephem_object_shift_vec3(app, oid, 0, p3, z0, -f, s);

        lv_vg_begin_path(ctx);
        lv_vg_3d_move_to(ctx, lv_point_3d(p2[0], p2[1], p2[2]));
        lv_vg_3d_line_to(ctx, lv_point_3d(p3[0], p3[1], p3[2]));
        lv_vg_stroke_width(ctx, app->line_width);
        lv_vg_stroke_color(ctx, grey);
        lv_vg_stroke(ctx);
    }
}

static void lv_zodiac_2d(lv_app *app, lv_context* ctx, float w, float h)
{
    NVGcontext *vg = *(NVGcontext**)app->ctx_nanovg->priv;

    float f = global_scale * app->zodiac_offset;
    float g = global_scale * app->zodiac_scale * (1.0f - app->symbol_offset);
    vec4 x0, y0, z0;

    lv_iau2006_dynamic_basis(app, x0, y0, z0);

    for (int i = 0; i < 12; i++)
    {
        vec3 p, q;
        float theta = 2.0f * M_PI * (i*30.0f+15.0f) / 360.0f;
        float symbol_size = app->symbol_size * app->ui_scale;

        vec3_sincos_basis(p, theta, x0, y0, z0, f, g);
        object_to_screen(q, p, app->m_mvp, w, h);

        nvgFontSize(vg, symbol_size);
        nvgFontFace(vg, "sans");
        nvgFillColor(vg, nvgRGBA(255, 255, 255, 255));
        nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
        nvgText(vg, q[0], q[1], signs[i].symbol, NULL);
    }

    for (size_t oid = 0; oid < countof(data); oid++)
    {
        float s, x, y;
        vec3 p0, p1, p2, q;

        s = lv_oid_scale(app->cartoon, oid);
        lv_ephem_object_vec3(app, oid, 0, p0, s);
        vec3_project_to_basis(p1, p0, x0, y0);
        vec3_multiply_add(p2, z0, f, p1);
        object_to_screen(q, p2, app->m_mvp, w, h);

        nvgBeginPath(vg);
        nvgCircle(vg, q[0], q[1], 3.0f);
        nvgFillColor(vg, nvgRGBA(255, 255, 255, 255));
        nvgFill(vg);
    }
}

static void lv_planets_3d(lv_app *app, lv_context* ctx)
{
    float f = global_scale * app->zodiac_offset;
    vec4 x0, y0, z0;
    vec3 p0, p1;

    lv_iau2006_dynamic_basis(app, x0, y0, z0);

    for (size_t oid = 0; oid < countof(data); oid++)
    {
        float s = lv_oid_scale(app->cartoon, oid);

        lv_vg_stroke_width(ctx, app->trail_width);
        for (size_t i = 0; i < lv_steps(app) + 1; i += lv_edges(app))
        {
            float alpha = (float)(lv_steps(app)-1-i) / lv_steps(app);

            lv_vg_begin_path(ctx);
            lv_ephem_object_shift_vec3(app, oid, i, p0, z0, -f, s);
            if (p0[0] != p0[0]) continue;
            lv_vg_3d_move_to(ctx, lv_point_3d(p0[0], p0[1], p0[2]));
            for (size_t j = i + 1; j <= i + lv_edges(app) && j < lv_steps(app) + 1; j++)
            {
                lv_ephem_object_shift_vec3(app, oid, j, p1, z0, -f, s);
                if (p1[0] != p1[0]) break;
                lv_vg_3d_line_to(ctx, lv_point_3d(p1[0], p1[1], p1[2]));
            }
            lv_vg_stroke_color(ctx, lv_oid_color(app, oid, alpha));
            lv_vg_stroke(ctx);
        }
    }
}

static void lv_planets_2d(lv_app *app, lv_context* ctx, float w, float h)
{
    NVGcontext *vg = *(NVGcontext**)app->ctx_nanovg->priv;

    float f = global_scale * app->zodiac_offset;
    vec4 x0, y0, z0;

    lv_oid_idx zidx[countof(data)];

    lv_iau2006_dynamic_basis(app, x0, y0, z0);

    for (size_t oid = 0; oid < countof(data); oid++)
    {
        vec3 p0;
        float s = lv_oid_scale(app->cartoon, oid);
        lv_ephem_object_shift_vec3(app, oid, 0, p0, z0, -f, s);
        zidx[oid].oid = oid;
        object_to_screen(zidx[oid].pos, p0, app->m_mvp, w, h);
    }

    qsort(zidx, countof(data), sizeof(lv_oid_idx), lv_oid_zsort);

    for (size_t idx = 0; idx < countof(data); idx++)
    {
        lv_color color;
        NVGcolor vgc;
        size_t oid;
        int img, iw, ih;
        float r, a, b, s, dw, dh, x, y;
        vec4 q;

        oid = zidx[idx].oid;;
        memcpy(q, zidx[idx].pos, sizeof(vec4));

        color = lv_oid_color(app, oid, 1.0f);
        memcpy(&vgc, &color, sizeof(vgc));

        img = app->images[oid];
        nvgImageSize(vg, img, &iw, &ih);

        r = data[oid].diameter / data[ephem_id_Jupiter].diameter;
        a = app->planet_scale / 50.0f;
        b = app->planet_scale / 25.0f;
        s = a + b * log10f(1.0f + 9.0f * r);
        dw = iw * s, dh = ih * s;
        x = q[0] - dw/2.0f;
        y = q[1] - dh/2.0f;

        nvgBeginPath(vg);
        nvgRect(vg, x, y, dw, dh);
        nvgFillPaint(vg, nvgImagePattern(vg, x, y, dw, dh, 0.0f, img, 1.0f));
        nvgFill(vg);

        float font_size = app->font_size * app->ui_scale;
        float symbol_size = app->symbol_size * app->ui_scale;
        float v = font_size;

        nvgFontFace(vg, "sans");
        nvgFontSize(vg, font_size);
        nvgFillColor(vg, nvgRGBA(255, 255, 255, 255));
        nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);

        if (app->name_legend && app->sym_legend) {
            char name[64];
            snprintf(name, sizeof(name), "%s %s", data[oid].symbol, data[oid].name);
            nvgFontSize(vg, font_size);
            nvgText(vg, q[0], q[1] + v + ih * s * 0.5f, name, NULL);
            v += font_size * 1.5f;
        }
        else if (app->name_legend) {
            nvgFontSize(vg, font_size);
            nvgText(vg, q[0], q[1] + v + ih * s * 0.5f, data[oid].name, NULL);
            v += font_size * 1.5f;
        }
        else if (app->sym_legend) {
            nvgFontSize(vg, symbol_size);
            nvgText(vg, q[0], q[1] + v + ih * s * 0.5f, data[oid].symbol, NULL);
            v += font_size * 1.5f;
        }

        if (app->dist_legend && oid > 0) {
            char dist[64];
            vec3 p0;
            lv_ephem_object_vec3(app, oid, 0, p0, 1.0f);
            snprintf(dist, sizeof(dist), "%12.0f km", vec3_len(p0) / 1e3f);
            nvgFontSize(vg, font_size);
            nvgText(vg, q[0], q[1] + v + ih * s * 0.5f, dist, NULL);
        }
    }
}

static void model_matrix_transform(lv_app *app, vec3 scale, vec3 trans,
    vec3 rot, float r)
{
    mat4x4 m_model, m_proj;
    mat4x4 r_frame, r_inv;

    lv_iau2006_obliquity_matrix(r_frame, app->jd);
    mat4x4_invert(r_inv, r_frame);

    mat4x4_identity(m_model);
    mat4x4_translate_in_place(m_model, trans[0], trans[1], trans[2]);
    mat4x4_scale_aniso(m_model, m_model, scale[0], scale[1], scale[2]);
    mat4x4_rotate_X(m_model, m_model, deg_rad(rot[0]));
    mat4x4_rotate_Y(m_model, m_model, deg_rad(rot[1]));
    mat4x4_rotate_Z(m_model, m_model, deg_rad(rot[2]));
    mat4x4_mul(m_model, m_model, r_inv);
    mat4x4_perspective(m_proj, deg_rad(30.f), r, 1.f, 1e6f);
    mat4x4_mul(app->m_mvp, m_proj, m_model);
    mat4x4_invert(app->m_inv, app->m_mvp);
}

static void lv_render(lv_app* app, float w, float h, float r)
{
    vec3 rot = { app->rot[0], app->rot[1], app->rot[2] };
    vec3 scale = { 1/global_scale, 1/global_scale, 1/global_scale };
    vec3 trans = { app->trans[0], app->trans[1], app->trans[2] + app->zoom };
    vec2f origin = { app->origin.x, app->origin.y };
    mat4x4 m_model, m_proj;
    lv_context* ctx;

    if (app->playback) {
        if (++app->cjdf > app->ejdf) {
            if (app->cjd + app->cjdf >= app->ejd) {
                app->playback = 0;
            } else {
                app->cjd += (app->ejdf - app->sjdf) + 1;
                app->cjdf = app->sjdf;
            }
        }
    } else {
        double njd = lv_date_to_julian(app->date);
        if (njd != app->cjd + app->cjdf + 0.5) {
            if (njd - 0.5 < app->sjd) {
                app->cjd = app->sjd;
                app->cjdf = njd - 0.5 - app->sjd;
            } else if (njd - 0.5 > app->ejd) {
                app->cjd = app->ejd;
                app->cjdf = njd - 0.5 - app->ejd;
            } else {
                app->cjd = njd;
                app->cjdf = 0;
            }
        }
    }

    if (app->cjd != app->ljd || app->cjdf != app->ljdf) {
        lv_update_date(app);
        lv_ephem_calc(app, app->jd);
        app->ljd = app->cjd;
        app->ljdf = app->cjdf;
    }

    ctx = app->ctx_buffer;
    lv_buffer_vg_clear(ctx);

    if (app->zodiac_layer) {
        lv_zodiac_3d(app, ctx);
    }

    lv_planets_3d(app, ctx);

    if (app->grid_layer) {
        lv_grid_3d(app, ctx);
    }

    model_matrix_transform(app, scale, trans, rot, r);

    ctx = app->ctx_xform;
    lv_xform_proj_matrix(app->ctx_xform, app->m_mvp, 1);
    lv_vg_begin_frame(ctx, w, h, r);
    lv_vg_reset(ctx);
    lv_vg_push(ctx);
    lv_buffer_vg_playback(app->ctx_buffer, ctx);
    lv_vg_pop(ctx);

    if (app->zodiac_layer) {
        lv_zodiac_2d(app, ctx, w, h);
    }

    lv_planets_2d(app, ctx, w, h);

    lv_vg_end_frame(ctx);
}

void lv_date_picker(lv_date *date)
{
    int dim;

    ImGui::Text("Date:");
    ImGui::SameLine();
    ImGui::PushItemWidth(200);
    ImGui::InputInt("YYYY", &date->year);

    if (date->year < 1700) {
        date->year = 1700;
        date->month = 1;
        date->day = 1;
    } else if (date->year > 2399) {
        date->year = 2399;
        date->month = 12;
        date->day = 31;
    }

    ImGui::PopItemWidth();
    ImGui::PushItemWidth(150);
    ImGui::SameLine();
    ImGui::InputInt("MM", &date->month);

    if (date->month < 1) {
        date->year--;
        date->month = 12;
        if (date->year < 1700) {
            date->year = 1700;
            date->month = 1;
            date->day = 1;
        }
    } else if (date->month > 12) {
        date->year++;
        date->month = 1;
        if (date->year > 2399) {
            date->year = 2399;
            date->month = 12;
            date->day = 31;
        }
    }

    ImGui::SameLine();
    ImGui::InputInt("DD", &date->day);
    ImGui::PopItemWidth();

    dim = lv_days_in_month(date->year, date->month-1);
    if (date->day < 1) {
        date->month--;
        if (date->month < 1) {
            date->year--;
            date->month = 12;
            if (date->year < 1700) {
                date->year = 1700;
                date->month = 1;
                date->day = 1;
            } else {
                date->day = lv_days_in_month(date->year, date->month-1);
            }
        } else {
            date->day = lv_days_in_month(date->year, date->month-1);
        }
    } else if (date->day > dim) {
        date->month++;
        if (date->month > 12) {
            date->month = 1;
            date->year++;
            if (date->year > 2399) {
                date->year = 2399;
                date->month = 12;
                date->day = 31;
            } else {
                date->day = 1;
            }
        } else {
            date->day = 1;
        }
    }
}

static void lv_font_size(const char *label, int *font_size)
{
    char font_size_str[16];
    snprintf(font_size_str, sizeof(font_size_str), "%u", *font_size);

    if (ImGui::BeginCombo(label, font_size_str)) {
        for (int i = 0; i < IM_ARRAYSIZE(font_sizes); i++)
        {
            bool sel = (*font_size == font_sizes[i]);
            snprintf(font_size_str, sizeof(font_size_str), "%u", font_sizes[i]);
            if (ImGui::Selectable(font_size_str, sel)) {
                *font_size = font_sizes[i];
            }
            if (sel) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
}

static void lv_imgui(lv_app* app, float w, float h, float r)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    ImGui::NewFrame();
    ImGui::Begin("Controller", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::PushItemWidth(1000.0f);
    if (ImGui::SliderInt("Julian Date", &app->cjd, app->sjd, app->ejd)) {
        lv_update_date(app);
    }
    if (ImGui::SliderInt("Fine Adjust", &app->cjdf, app->sjdf, app->ejdf)) {
        lv_update_date(app);
    }
    ImGui::PopItemWidth();
    if (ImGui::Button(app->playback  ? "\uf04c##playback"
                                     : "\uf04b##playback", ImVec2(36, 36))) {
        app->playback = !app->playback;
    }
    ImGui::SameLine();
    lv_date_picker(&app->date);
    ImGui::End();

    ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("Model");
    ImGui::Separator();
    ImGui::Checkbox("IAU 2006 Precession", &app->precession);
    ImGui::SliderFloat("Rotate X", &app->rot[0], -180.0f, 180.0f);
    ImGui::SliderFloat("Rotate Y", &app->rot[1], -180.0f, 180.0f);
    ImGui::SliderFloat("Rotate Z", &app->rot[2], -180.0f, 180.0f);
    ImGui::SliderFloat("Translate X", &app->trans[0], -10.0f, 10.0f);
    ImGui::SliderFloat("Translate Y", &app->trans[1], -10.0f, 10.0f);
    ImGui::SliderFloat("Translate Z", &app->trans[2], -10.0f, 10.0f);

    ImGui::Text("Style");
    ImGui::Separator();
    ImGui::Checkbox("Cartoon Scaling", &app->cartoon);
    ImGui::SliderFloat("Trail Width", &app->trail_width, 0.0f, 12.0f);
    ImGui::SliderFloat("Line Width", &app->line_width, 0.0f, 6.0f);
    ImGui::SliderFloat("Planet Scale", &app->planet_scale, 0.0f, 5.0f);
    lv_font_size("Font Size", &app->font_size);
    lv_font_size("Symbol Size", &app->symbol_size);

    ImGui::Text("Grid");
    ImGui::Separator();
    ImGui::Checkbox("Grid Layer", &app->grid_layer);
    ImGui::SliderInt("Grid Divisions", &app->grid_steps, 1, 20);
    ImGui::SliderFloat("Grid Scale", &app->grid_scale, 0.0f, 20.0f);

    ImGui::Text("Zodiac");
    ImGui::Separator();
    ImGui::Checkbox("Zodiac Layer", &app->zodiac_layer);
    ImGui::SliderFloat("Symbol Offset", &app->symbol_offset, -0.1f, 0.1f);
    ImGui::SliderFloat("Zodiac Offset", &app->zodiac_offset, 0.0f, 20.0f);
    ImGui::SliderFloat("Zodiac Scale", &app->zodiac_scale, 0.0f, 20.0f);

    ImGui::Text("Legends");
    ImGui::Separator();
    ImGui::Checkbox("Symbols", &app->sym_legend);
    ImGui::Checkbox("Names", &app->name_legend);
    ImGui::Checkbox("Distances", &app->dist_legend);

    ImGui::End();

    ImGui::Begin("Chart", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    if (ImGui::BeginTable("Chart", 3))
    {
        mat4x4 m, im;
        vec3 e, o, d;
        vec4 q, p;
        float a;
        int deg, min, sid;

        ImGui::TableSetupColumn("Planet");
        ImGui::TableSetupColumn("Pos");
        ImGui::TableSetupColumn("Sign");
        ImGui::TableHeadersRow();

        lv_ephem_object_vec3(app, ephem_id_EarthMoon, 0, e, 1.0f);
        lv_iau2006_dynamic_matrix(app, m);
        mat4x4_invert(im, m);

        for (size_t oid = 0; oid < countof(data); oid++)
        {
            if (oid == ephem_id_EarthMoon) continue;

            lv_ephem_object_vec3(app, oid, 0, o, 1.0);
            vec3_sub(d, o, e);
            vec4_vec3_w1(q, d);
            mat4x4_mul_vec4(p, im, q);
            a = vector_angle_deg(p[0], p[1]);

            deg = (int)floorf(a);
            min = (int)floorf((a - deg) * 60.0);
            sid = deg / 30;

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%s %s", data[oid].symbol, data[oid].name);
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%3d°%02d′", deg, min);
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%s %s", signs[sid].symbol, signs[sid].name);
        }
        ImGui::EndTable();
    }
    ImGui::End();

    ImGui::Render();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

static int mouse_find_oid(lv_app *app, vec2f pos,
    size_t *sel_oid, double *sel_tjd)
{
    int win_width, win_height;
    double jd = app->cjd + app->cjdf;
    float epsilon = global_scale / 12;
    float f = global_scale * app->zodiac_offset;
    vec3 snear = { pos.x, pos.y, 0.0f };
    vec3 sfar = { pos.x, pos.y, 1.0f };
    vec3 near, far;
    vec4 x0, y0, z0;

    glfwGetWindowSize(app->window, &win_width, &win_height);
    screen_to_object(near, snear, app->m_inv, win_width, win_height);
    screen_to_object(far, sfar, app->m_inv, win_width, win_height);

    lv_iau2006_dynamic_basis(app, x0, y0, z0);

    for (size_t oid = 1; oid < countof(data); oid++)
    {
        float s = lv_oid_scale(app->cartoon, oid);
        for (size_t i = 0; i < lv_steps(app) - 1; i++)
        {
            double interval = data[oid].orbit / lv_steps(app);
            double tjd = jd - (i * interval);
            double *o1 = lv_ephem_object(app, oid, i);
            double *o2 = lv_ephem_object(app, oid, i + 1);
            float z = -z0[2] * f + (float)(o1[2] + o2[2]) * 0.5f * s;
            float t = (z - near[2]) / (far[2] - near[2]);
            float px = far[0] * t + near[0] * (1.0f - t);
            float py = far[1] * t + near[1] * (1.0f - t);
            vec2 p = { px, py };
            vec2 a = {
                -z0[0] * f + (float)o1[0] * s,
                -z0[1] * f + (float)o1[1] * s
            };
            vec2 b = {
                -z0[0] * f + (float)o2[0] * s,
                -z0[1] * f + (float)o2[1] * s
            };
            float d = vec2_dist_point_line(p, a, b);
            if (d < epsilon) {
                *sel_oid = oid;
                *sel_tjd = tjd;
                return 1;
            }
        }
    }

    *sel_oid = -1;
    *sel_tjd = NAN;
    return 0;
}

static void image_set_alpha(uchar* image, int w, int h, int stride, uchar a)
{
    int x, y;
    for (y = 0; y < h; y++) {
        uchar* row = &image[y*stride];
        for (x = 0; x < w; x++)
            row[x*4+3] = a;
    }
}

static void image_flip_horiz(uchar* image, int w, int h, int stride)
{
    int i = 0, j = h-1, k;
    while (i < j) {
        uchar* ri = &image[i * stride];
        uchar* rj = &image[j * stride];
        for (k = 0; k < w*4; k++) {
            uchar t = ri[k];
            ri[k] = rj[k];
            rj[k] = t;
        }
        i++;
        j--;
    }
}

static void lv_save_screenshot(int w, int h, const char* filename)
{
    uchar* image = (uchar*)malloc(w*h*4);
    if (image == NULL) return;
    glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, image);
    image_set_alpha(image, w, h, w*4, 255);
    image_flip_horiz(image, w, h, w*4);
#ifdef STB_IMAGE_WRITE_IMPLEMENTATION
    stbi_write_png(filename, w, h, 4, image, w*4);
#endif
    free(image);
}

static void key(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    NVG_NOTUSED(scancode);
    NVG_NOTUSED(mods);

    lv_app *app = (lv_app*)glfwGetWindowUserPointer(window);

    if(action == GLFW_RELEASE) return;

    switch (key) {
    case GLFW_KEY_ESCAPE:
        if (action == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
        break;
    case GLFW_KEY_Z: app->rot[2] += 5.f; break;
    case GLFW_KEY_C: app->rot[2] -= 5.f; break;
    case GLFW_KEY_W: app->rot[0] += 5.f; break;
    case GLFW_KEY_S: app->rot[0] -= 5.f; break;
    case GLFW_KEY_A: app->rot[1] += 5.f; break;
    case GLFW_KEY_D: app->rot[1] -= 5.f; break;
    case GLFW_KEY_P: {
        int fb_width, fb_height;
        glfwGetFramebufferSize(window, &fb_width, &fb_height);
        lv_save_screenshot(fb_width, fb_height, "screenshot.png");
        break;
    }
    default: return;
    }
}

static void scroll(GLFWwindow* window, double xoffset, double yoffset)
{
    lv_app *app = (lv_app*)glfwGetWindowUserPointer(window);

    lv_trace("scroll\n");

    float quantum = app->zoom / 16.f;
    float ratio = 1.f + (float)quantum / (float)app->zoom;
    if (yoffset < 0. && app->zoom < max_zoom) {
        app->origin.x *= ratio;
        app->origin.y *= ratio;
        app->zoom += quantum;
    } else if (yoffset > 0. && app->zoom > min_zoom) {
        app->origin.x /= ratio;
        app->origin.y /= ratio;
        app->zoom -= quantum;
    }
}

static int mouse_left_drag;
static int mouse_right_drag;

static void mouse_button(GLFWwindow* window, int button, int action, int mods)
{
    lv_app *app = (lv_app*)glfwGetWindowUserPointer(window);
    size_t oid;
    double tjd;

    switch (button) {
    case GLFW_MOUSE_BUTTON_LEFT:
        mouse_left_drag = (action == GLFW_PRESS);
        app->last_mouse = app->mouse;
        app->last_zoom = app->zoom;
        if (mouse_find_oid(app, app->mouse, &oid, &tjd)) {
            app->sel_oid = oid;
            app->sel_tjd = tjd;
            app->rot_oid = (action == GLFW_PRESS) ? oid : -1;
            app->rot_tjd = (action == GLFW_PRESS) ? tjd : NAN;
        }
        break;
    case GLFW_MOUSE_BUTTON_RIGHT:
        mouse_right_drag = (action == GLFW_PRESS);
        app->last_mouse = app->mouse;
        app->last_zoom = app->zoom;
        break;
    }
}

static void cursor_position(GLFWwindow* window, double xpos, double ypos)
{
    lv_app *app = (lv_app*)glfwGetWindowUserPointer(window);

    app->mouse = (vec2f) { (float)xpos, (float)ypos };

    if (mouse_left_drag) {
        app->origin.x += app->mouse.x - app->last_mouse.x;
        app->origin.y += app->mouse.y - app->last_mouse.y;
        app->last_mouse = app->mouse;
    }
    if (mouse_right_drag) {
        float delta0 = app->mouse.x - app->last_mouse.x;
        float delta1 = app->mouse.y - app->last_mouse.y;
        float zoom = app->last_zoom * powf(65.0f/64.0f,(float)-delta1);
        if (zoom != app->zoom && zoom > min_zoom && zoom < max_zoom) {
            app->zoom = zoom;
            app->origin.x = (app->origin.x * (zoom / app->zoom));
            app->origin.y = (app->origin.y * (zoom / app->zoom));
        }
    }
}

static void lv_main_loop(GLFWwindow* window, lv_app *app)
{
    double target_fps = 60.0;
    double frame_time = 1.0 / target_fps;
    double last_time;

    while (!glfwWindowShouldClose(window))
    {
        double start_time = glfwGetTime();
        double delta      = start_time - last_time;

        int win_width, win_height;
        int fb_width, fb_height;
        float win_ratio;

        glfwGetWindowSize(window, &win_width, &win_height);
        glfwGetFramebufferSize(window, &fb_width, &fb_height);
        win_ratio = (float)win_width / (float)win_height;

        glViewport(0, 0, fb_width, fb_height);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
        lv_render(app, win_width, win_height, win_ratio);
        lv_imgui(app, win_width, win_height, win_ratio);
        glfwSwapBuffers(window);
        glfwPollEvents();

        for (;;) {
            double end_time = glfwGetTime();
            double elapsed  = end_time - start_time;
            double wait_time = frame_time - elapsed;
            if (wait_time < 0.0) break;
            glfwWaitEventsTimeout(wait_time);
        }

        last_time = glfwGetTime();
      }
}

/*
 * option processing
 */

static void print_help(int argc, char **argv)
{
    lv_info(
        "usage: %s [options]\n"
        "\n"
        "Options:\n"
        "  -l, (info|debug|trace)             debug level\n"
        "  -w, --width <int>                  buffer width\n"
        "  -h, --height <int>                 buffer height\n"
        "  -h, --help                         command line help\n",
        argv[0]
    );
}

static int match_opt(const char *arg, const char *opt, const char *longopt)
{
    return strcmp(arg, opt) == 0 || strcmp(arg, longopt) == 0;
}

static void parse_options(int argc, char **argv)
{
    int i = 1;
    while (i < argc) {
        if (match_opt(argv[i], "-h", "--help")) {
            opt_help++;
            i++;
        } else if (match_opt(argv[i], "-l", "--level")) {
            char* level = argv[++i];
            if (strcmp(level, "none") == 0) {
                lv_ll = lv_ll_none;
            } else if (strcmp(level, "info") == 0) {
                lv_ll = lv_ll_info;
            } else if (strcmp(level, "debug") == 0) {
                lv_ll = lv_ll_debug;
            } else if (strcmp(level, "trace") == 0) {
                lv_ll = lv_ll_trace;
            }
            i++;
        } else if (match_opt(argv[i], "-w", "--width")) {
            opt_width = atoi(argv[++i]);
            i++;
        } else if (match_opt(argv[i], "-h", "--height")) {
            opt_height = atoi(argv[++i]);
            i++;
        } else {
            lv_error("error: unknown option: %s\n", argv[i]);
            opt_help++;
            break;
        }
    }

    if (opt_help) {
        print_help(argc, argv);
        exit(1);
    }
}

static void errorcb(int error, const char* desc)
{
    lv_error("GLFW error %d: %s\n", error, desc);
}

void gllv_app(int argc, char **argv)
{
    GLFWwindow* window;
    lv_app app;

    if (!glfwInit()) {
        lv_panic("glfwInit failed\n");
    }

    glfwSetErrorCallback(errorcb);

    glfwWindowHint(GLFW_SCALE_TO_MONITOR , GL_TRUE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    lv_app_init(&app);

    app.window = window = glfwCreateWindow(opt_width, opt_height,
        "ephembra", NULL, NULL);
    if (!window) {
        lv_panic("glfwCreateWindow failed\n");
    }

    glfwSetWindowUserPointer(window, &app);
    glfwSetKeyCallback(window, key);
    glfwSetScrollCallback(window, scroll);
    glfwSetMouseButtonCallback(window, mouse_button);
    glfwSetCursorPosCallback(window, cursor_position);
    glfwMakeContextCurrent(window);
    gladLoadGL();
    glfwSwapInterval(0);
    glfwSetTime(0);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    ImFontConfig icons_config;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;
    ImWchar icons_ranges[] = { 0xf000, 0xf3ff, 0 };

    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear();
    io.Fonts->AddFontFromFileTTF(ephembra_mono_font, 14.0f);
    io.Fonts->AddFontFromFileTTF(ephembra_awes_font, 14.0f,
        &icons_config, icons_ranges);
    io.Fonts->Build();
    io.FontGlobalScale = app.ui_scale;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glClearColor(0.f, 0.f, 0.f, 1.f);

    lv_vg_uinit(&app);
    lv_ephem_init(&app);
    lv_main_loop(window, &app);
    lv_ephem_destroy(&app);
    lv_vg_udestroy(&app);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
}

/*
 * main program
 */

int main(int argc, char **argv)
{
    lv_ll = lv_ll_info;
    parse_options(argc, argv);
    gllv_app(argc, argv);
    return 0;
}
