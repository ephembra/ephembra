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
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <time.h>
#include <limits.h>

#define _USE_MATH_DEFINES
#include <math.h>

#ifdef HAVE_GLAD
#include <glad/glad.h>
#else
#define gladLoadGL()
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#endif
#include <GLFW/glfw3.h>

#include "nanovg.h"
#define NANOVG_GL3
#include "nanovg_gl.h"
#include "nanovg_gl_utils.h"

#include "linmath.h"
#include "gl2_nano.h"

#include "lv_color.h"
#include "lv_date.h"
#include "lv_data.h"
#include "lv_ephem.h"
#include "lv_iau2006.h"
#include "lv_osdef.h"
#include "lv_osutil.h"
#include "lv_screen.h"
#include "lv_targa.h"
#include "lv_vecalg.h"
#include "lv_vg.h"

#include "gldemo.h"

const char* ephembra_data_file = "resources/data/DE440Coeff.bin";
const char* ephembra_sans_font = "resources/fonts/DejaVuSans.ttf";
const char* ephembra_mono_font = "resources/fonts/DejaVuSansMono.ttf";
const char* ephembra_awes_font = "resources/fonts/fontawesome-webfont.ttf";
const char* ephembra_image_tmpl = "%s/resources/images/%s.png";
const char* ephembra_video_dir = "tmp/video";
const char* ephembra_video_tmpl = "%s/ephembra-%06d.tga";

static const float min_zoom = 2.0f, max_zoom = 2048.0f;
static const float global_scale = 1e12f;

static int opt_help;
static int opt_width = 1280;
static int opt_height = 720;

static lv_color grey = { 0.4980f, 0.4980f, 0.4980f, 1.0f };
static lv_color white = { 0.8157f, 0.8157f, 0.8157f, 1.0f };

/*
 * gldemo
 */

void lv_app_init(lv_app *app, GLFWwindow *window)
{
    memset(app, 0, sizeof(lv_app));
    app->window = window;
    app->zoom = 19.0f;
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
    app->timedisp = 1;
    app->playback = 0;
    app->record = 0;
    app->precession = 1;
    app->cartoon = 1;
    app->sym_legend = 1;
    app->name_legend = 0;
    app->dist_legend = 0;
    app->font_size = 12;
    app->symbol_size = 16;
    app->play_step = 0;
    app->play_rate = 1.0;
    app->ui_scale = 2.0f;
    app->grid_layer = 0;
    app->grid_steps = 10;
    app->grid_scale = 9.0f;
    app->trail_width = 6.0f;
    app->line_width = 2.0f;
    app->planet_scale = 2.5f;
    app->zodiac_layer = 1;
    app->symbol_offset = -0.05f;
    app->zodiac_offset = 0.556f;
    app->zodiac_scale = 9.0f;
    app->frame_num = 0;
    app->frame_stop = 3600;

    lv_app_imgui_init(app);
    lv_vg_uinit(app);
    lv_ephem_init(app);
}

void lv_app_destroy(lv_app *app)
{
    lv_ephem_destroy(app);
    lv_vg_udestroy(app);
    lv_app_imgui_destroy(app);
}

static void lv_date_to_slider(lv_app *app)
{
    if (app->jd - 0.5 < app->sjd) {
        app->cjd = app->sjd;
        app->cjdf = (int)(app->jd - 0.5 - app->sjd);
    } else if (app->jd - 0.5 > app->ejd) {
        app->cjd = app->ejd;
        app->cjdf = (int)(app->jd - 0.5 - app->ejd);
    } else {
        app->cjd = (int)app->jd;
        app->cjdf = 0;
    }
}

static void lv_current_date(lv_app *app)
{
    lv_date d = lv_date_time_now();
    double njd = lv_date_to_julian(d);
    if (app->jd != njd) {
        app->jd = njd;
        lv_date_to_slider(app);
        app->date = lv_julian_to_date(app->jd);
        lv_ephem_calc(app, app->jd);
    }
}

void lv_ephem_init(lv_app *app)
{
    char ephembra_data_file_rsrc[PATH_MAX];
    char ephembra_sans_font_rsrc[PATH_MAX];
    char ephembra_mono_font_rsrc[PATH_MAX];
    char executable_dir[PATH_MAX];

    NVGcontext *vg = *(NVGcontext**)app->ctx_nanovg->priv;

    if (get_resource_path(ephembra_data_file_rsrc,
        sizeof(ephembra_data_file_rsrc), ephembra_data_file) != 0 ||
        get_resource_path(ephembra_sans_font_rsrc,
        sizeof(ephembra_sans_font_rsrc), ephembra_sans_font) != 0 ||
        get_resource_path(ephembra_mono_font_rsrc,
        sizeof(ephembra_mono_font_rsrc), ephembra_mono_font) != 0 ||
        get_executable_dir(executable_dir, sizeof(executable_dir)) != 0)
    {
        fprintf(stderr, "lv_ephem_init: failed to locate resources\n");
        exit(1);
    }

    nvgCreateFont(vg, "mono", ephembra_mono_font_rsrc);
    nvgCreateFont(vg, "sans", ephembra_sans_font_rsrc);

    de440_create_ephem(&app->ctx, ephembra_data_file_rsrc);
    app->eph = (double*)malloc(ephem_id_Last * app->steps * sizeof(double) * 3);
    lv_current_date(app);

    app->images = (int*)malloc(data_count * sizeof(int));
    for (size_t idx = 0; idx < data_count; idx++)
    {
        char path[PATH_MAX];
        snprintf(path, sizeof(path),
            ephembra_image_tmpl, executable_dir, data[idx].name);
        app->images[idx] = nvgCreateImage(vg, path, NVG_IMAGE_GENERATE_MIPMAPS);
    }
}

void lv_ephem_destroy(lv_app *app)
{
    NVGcontext *vg = *(NVGcontext**)app->ctx_nanovg->priv;

    for (size_t idx = 0; idx < data_count; idx++) {
        nvgDeleteImage(vg, app->images[idx]);
    }

    de440_destroy_ephem(&app->ctx);

    free(app->images);
    free(app->eph);
}

void lv_ephem_calc(lv_app *app, double jd)
{
    for (size_t idx = 0; idx < data_count; idx++)
    {
        size_t oid = data[idx].oid;
        for (size_t i = 0; i < app->steps; i++)
        {
            double interval = data[idx].orbit / app->steps;
            double tjd = jd - (i * interval);
            size_t row = de440_find_row(&app->ctx, tjd);
            double *o = lv_ephem_object(app, oid, i);
            de440_ephem_obj(&app->ctx, tjd, row, oid, o);
        }
    }
}

void lv_iau2006_dynamic_matrix(lv_app *app, mat4x4 m)
{
    if (app->precession) {
        lv_iau2006_combined_matrix(m, app->jd);
    } else {
        lv_iau2006_obliquity_matrix(m, app->jd);
    }
}

void lv_iau2006_dynamic_basis(lv_app *app, vec3 x0, vec3 y0, vec3 z0)
{
    if (app->precession) {
        lv_iau2006_combined_basis(x0, y0, z0, app->jd);
    } else {
        lv_iau2006_obliquity_basis(x0, y0, z0, app->jd);
    }
}

static inline lv_color lv_idx_color(lv_app* app, size_t idx, float alpha)
{
    lv_color color = lv_color_af(data[idx].color, alpha);
    if (data[idx].oid == app->rot_oid) {
        return lv_color_adjust(color, 1.5, 1.5);
    } else {
        return color;
    }
}

static inline float lv_idx_scale(bool cartoon, size_t idx)
{
    float r = (float)(data[idx].dist / data[data_count-1].dist);
    return cartoon ? (idx / 10.0f) / r : 1.0f;
}

static inline size_t lv_idx_for_oid(size_t oid)
{
    for (size_t idx = 0; idx < data_count; idx++) {
        if (data[idx].oid == oid) return idx;
    }
    return -1;
}

static int lv_oid_zsort(const void *p1, const void *p2)
{
    const lv_oid_idx *a = (lv_oid_idx *)p1;
    const lv_oid_idx *b = (lv_oid_idx *)p2;
    if (a->pos[2] < b->pos[2]) return -1;
    else if (a->pos[2] > b->pos[2]) return 1;
    else return 0;
}

static inline void vec3_sum_scale_3(vec3 r,
    vec3 a, float as, vec3 b, float bs, vec3 c, float cs)
{
    r[0] = a[0] * as + b[0] * bs + c[0] * cs;
    r[1] = a[1] * as + b[1] * bs + c[1] * cs;
    r[2] = a[2] * as + b[2] * bs + c[2] * cs;
}

void lv_grid_3d(lv_app *app, lv_context* ctx)
{
    float f = global_scale * app->zodiac_offset;
    float g = global_scale * app->grid_scale;
    vec4 x0, y0, z0;

    lv_iau2006_dynamic_basis(app, x0, y0, z0);

    int n = app->grid_steps;
    float step = g / n;

    lv_vg_stroke_width(ctx, app->line_width);
    lv_vg_stroke_color(ctx, grey);
    for (int i = -n; i <= n; i++)
    {
        vec3 p0, p1, p2, p3;

        vec3_sum_scale_3(p0, z0, -f, y0, -g, x0, i * step);
        vec3_sum_scale_3(p1, z0, -f, y0,  g, x0, i * step);

        lv_vg_begin_path(ctx);
        lv_vg_3d_move_to(ctx, lv_point_3d(p0[0], p0[1], p0[2]));
        lv_vg_3d_line_to(ctx, lv_point_3d(p1[0], p1[1], p1[2]));
        lv_vg_stroke(ctx);

        vec3_sum_scale_3(p2, z0, -f, x0, -g, y0, i * step);
        vec3_sum_scale_3(p3, z0, -f, x0,  g, y0, i * step);

        lv_vg_begin_path(ctx);
        lv_vg_3d_move_to(ctx, lv_point_3d(p2[0], p2[1], p2[2]));
        lv_vg_3d_line_to(ctx, lv_point_3d(p3[0], p3[1], p3[2]));
        lv_vg_stroke(ctx);
    }
}

void lv_zodiac_3d(lv_app *app, lv_context* ctx)
{
    float f = global_scale * app->zodiac_offset;
    float g = global_scale * app->zodiac_scale;
    float s = lv_idx_scale(app->cartoon, lv_idx_for_oid(ephem_id_Earth));
    vec4 x0, y0, z0;
    vec3 p0;

    lv_iau2006_dynamic_basis(app, x0, y0, z0);
    lv_ephem_object_shift_vec3(app, ephem_id_Earth, 0, p0, z0, f, s);

    for (int i = 0; i < 12; i++)
    {
        lv_vg_stroke_color(ctx, white);
        lv_vg_stroke_width(ctx, app->line_width);
        lv_vg_fill_color(ctx, lv_rgbaf(0.1f, 0.1f, 0.1f, 1.0f));
        lv_vg_begin_path(ctx);
        lv_vg_3d_move_to(ctx, lv_point_3d(p0[0], p0[1], p0[2]));
        for (int j = 0; j < 31; j++)
        {
            float theta = 2.0f * (float)M_PI * (i*30.0f+j) / 360.0f;
            vec3 p1;
            vec3_sincos_basis(p1, theta, x0, y0, z0, f, g);
            lv_vg_3d_line_to(ctx, lv_point_3d(p1[0], p1[1], p1[2]));
        }
        lv_vg_close_path(ctx);
        lv_vg_fill(ctx);
        lv_vg_stroke(ctx);
    }

    for (size_t idx = 0; idx < data_count; idx++)
    {
        size_t oid = data[idx].oid;
        float s = lv_idx_scale(app->cartoon, idx);
        vec3 p0, p1, p2, p3;

        if (oid == ephem_id_Moon) continue;

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

void lv_zodiac_2d(lv_app *app, lv_context* ctx, int w, int h)
{
    NVGcontext *vg = *(NVGcontext**)app->ctx_nanovg->priv;

    float f = global_scale * app->zodiac_offset;
    float g = global_scale * app->zodiac_scale * (1.0f - app->symbol_offset);
    vec4 x0, y0, z0;

    lv_iau2006_dynamic_basis(app, x0, y0, z0);

    for (int i = 0; i < 12; i++)
    {
        vec3 p, q;
        float theta = 2.0f * (float)M_PI * (i*30.0f+15.0f) / 360.0f;
        float symbol_size = app->symbol_size * app->ui_scale;

        vec3_sincos_basis(p, theta, x0, y0, z0, f, g);
        object_to_screen(q, p, app->m_mvp, w, h);

        nvgFontSize(vg, symbol_size);
        nvgFontFace(vg, "sans");
        nvgFillColor(vg, nvgRGBA(255, 255, 255, 255));
        nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
        nvgText(vg, q[0], q[1], signs[i].symbol, NULL);
    }

    for (size_t idx = 0; idx < data_count; idx++)
    {
        size_t oid = data[idx].oid;

        if (oid == ephem_id_Moon) continue;

        vec3 p0, p1, p2, q;
        float s = lv_idx_scale(app->cartoon, idx);

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

void lv_planets_3d(lv_app *app, lv_context* ctx)
{
    float f = global_scale * app->zodiac_offset;
    vec4 x0, y0, z0;
    vec3 p0, p1;

    lv_iau2006_dynamic_basis(app, x0, y0, z0);

    for (size_t idx = 0; idx < data_count; idx++)
    {
        size_t oid = data[idx].oid;

        if (oid == ephem_id_Moon) continue;

        float s = lv_idx_scale(app->cartoon, idx);

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
            lv_vg_stroke_color(ctx, lv_idx_color(app, idx, alpha));
            lv_vg_stroke(ctx);
        }
    }
}

void lv_planets_2d(lv_app *app, lv_context* ctx, int w, int h)
{
    NVGcontext *vg = *(NVGcontext**)app->ctx_nanovg->priv;

    float f = global_scale * app->zodiac_offset;
    vec4 x0, y0, z0;

    lv_oid_idx *zidx = alloca(data_count * sizeof(lv_oid_idx));

    lv_iau2006_dynamic_basis(app, x0, y0, z0);

    for (size_t idx = 0; idx < data_count; idx++)
    {
        vec3 p0;
        size_t oid = data[idx].oid;
        zidx[idx].idx = idx;
        float s = lv_idx_scale(app->cartoon, idx);
        lv_ephem_object_shift_vec3(app, oid, 0, p0, z0, -f, s);
        object_to_screen(zidx[idx].pos, p0, app->m_mvp, w, h);
    }

    qsort(zidx, data_count, sizeof(lv_oid_idx), lv_oid_zsort);

    for (size_t i = 0; i < data_count; i++)
    {
        lv_color color;
        NVGcolor vgc;
        size_t oid, idx;
        int img, iw, ih;
        float r, a, b, s, dw, dh, x, y;
        vec2 q;

        idx = zidx[i].idx;
        oid = data[idx].oid;
        q[0] = zidx[i].pos[0];
        q[1] = zidx[i].pos[1];

        if (oid == ephem_id_Moon) continue;

        color = lv_idx_color(app, idx, 1.0f);
        vgc = nvgRGBAf(color.r, color.g, color.b, color.a);

        img = app->images[idx];
        nvgImageSize(vg, img, &iw, &ih);

        r = (float)(data[idx].diameter / 139820.0); /* Jupiter */
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
            snprintf(name, sizeof(name), "%s %s", data[idx].symbol, data[idx].name);
            nvgFontSize(vg, font_size);
            nvgText(vg, q[0], q[1] + v + ih * s * 0.5f, name, NULL);
            v += font_size * 1.5f;
        }
        else if (app->name_legend) {
            nvgFontSize(vg, font_size);
            nvgText(vg, q[0], q[1] + v + ih * s * 0.5f, data[idx].name, NULL);
            v += font_size * 1.5f;
        }
        else if (app->sym_legend) {
            nvgFontSize(vg, symbol_size);
            nvgText(vg, q[0], q[1] + v + ih * s * 0.5f, data[idx].symbol, NULL);
            v += font_size * 1.5f;
        }

        if (app->dist_legend) {
            char dist[64];
            vec3 p0;
            lv_ephem_object_vec3(app, oid, 0, p0, 1.0f);
            snprintf(dist, sizeof(dist), "%12.0f km", vec3_len(p0) / 1e3f);
            nvgFontSize(vg, font_size);
            nvgText(vg, q[0], q[1] + v + ih * s * 0.5f, dist, NULL);
        }
    }
}

static inline void model_matrix_transform(lv_app *app,
    vec3 scale, vec3 trans, vec3 rot, float r)
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

static void lv_save_screenshot(int w, int h, const char* filename)
{
    uchar* image = (uchar*)malloc(w*h*4);
    if (image == NULL) return;
    glReadPixels(0, 0, w, h, GL_BGRA, GL_UNSIGNED_BYTE, image);
    lv_targa_write_bgra(filename, w, h, image, lv_targa_type_rgb_rle, 4, 3);
    free(image);
}

void lv_render(lv_app* app, int w, int h, float r)
{
    vec3 rot = { app->rot[0], app->rot[1], app->rot[2] };
    vec3 scale = { 1/global_scale, 1/global_scale, 1/global_scale };
    vec3 trans = { app->trans[0], app->trans[1], app->trans[2] + app->zoom };
    vec2f origin = { app->origin.x, app->origin.y };
    lv_context* ctx;

    if (app->date_valid) {
        lv_date_to_slider(app);
        lv_ephem_calc(app, app->jd);
    }
    else if (app->slider_valid) {
        app->jd = app->cjd + app->cjdf + 0.5;
        app->date = lv_julian_to_date(app->jd);
        lv_ephem_calc(app, app->jd);
    }
    else if (app->playback) {
        if (app->jd - 0.5 >= app->ejd + app->ejdf) {
            app->playback = 0;
        } else {
            app->jd += app->play_rate;
            app->date = lv_julian_to_date(app->jd);
            lv_date_to_slider(app);
            lv_ephem_calc(app, app->jd);
        }
    }
    else if (app->timedisp) {
        lv_current_date(app);
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
    lv_vg_begin_frame(ctx, (float)w, (float)h, r);
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

void lv_record(lv_app* app, int w, int h, float r)
{
    struct stat st;
    int fb_width, fb_height;
    char filename[PATH_MAX];
    int frame;

    if (!app->record || app->frame_num >= app->frame_stop) return;

    if (stat(ephembra_video_dir, &st) != 0 || !(st.st_mode & S_IFDIR)) {
        lv_error("error: directory does not exist: %s\n", ephembra_video_dir);
        return;
    }

    snprintf(filename, sizeof(filename), ephembra_video_tmpl,
             ephembra_video_dir, app->frame_num++);
    glfwGetFramebufferSize(app->window, &fb_width, &fb_height);
    lv_save_screenshot(fb_width, fb_height, filename);

    if (app->frame_num >= app->frame_stop) app->record = 0;
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
    vec3 vnear, vfar, o1, o2;
    vec4 x0, y0, z0;

    glfwGetWindowSize(app->window, &win_width, &win_height);
    screen_to_object(vnear, snear, app->m_inv, win_width, win_height);
    screen_to_object(vfar, sfar, app->m_inv, win_width, win_height);

    lv_iau2006_dynamic_basis(app, x0, y0, z0);

    for (size_t idx = 0; idx < data_count; idx++)
    {
        size_t oid = data[idx].oid;
        float s = lv_idx_scale(app->cartoon, idx);
        for (size_t i = 0; i < lv_steps(app) - 1; i++)
        {
            double interval = data[idx].orbit / lv_steps(app);
            double tjd = jd - (i * interval);
            lv_ephem_object_vec3(app, oid, i,     o1, s);
            lv_ephem_object_vec3(app, oid, i + 1, o2, s);
            float z = -z0[2] * f + (float)(o1[2] + o2[2]) * 0.5f;
            float t = (z - vnear[2]) / (vfar[2] - vnear[2]);
            float px = vfar[0] * t + vnear[0] * (1.0f - t);
            float py = vfar[1] * t + vnear[1] * (1.0f - t);
            vec2 p = { px, py };
            vec2 a = { -z0[0] * f + o1[0], -z0[1] * f + o1[1] };
            vec2 b = { -z0[0] * f + o2[0], -z0[1] * f + o2[1] };
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
        lv_save_screenshot(fb_width, fb_height, "screenshot.tga");
        break;
    }
    case GLFW_KEY_R: app->playback = app->record = !app->record; break;
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

    while (!glfwWindowShouldClose(window))
    {
        double start_time = glfwGetTime();

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
        lv_record(app, win_width, win_height, win_ratio);
        glfwSwapBuffers(window);
        glfwPollEvents();

        for (;;) {
            double end_time = glfwGetTime();
            double elapsed  = end_time - start_time;
            double wait_time = frame_time - elapsed;
            if (wait_time < 0.0) break;
            glfwWaitEventsTimeout(wait_time);
        }
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

void lv_app_main(int argc, char **argv)
{
    GLFWwindow* window;
    lv_app app;

    if (!glfwInit()) {
        lv_panic("glfwInit failed\n");
    }

    glfwSetErrorCallback(errorcb);

    glfwWindowHint(GLFW_SCALE_TO_MONITOR , GL_TRUE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

    window = glfwCreateWindow(opt_width, opt_height,
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

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glClearColor(0.f, 0.f, 0.f, 1.f);

    lv_app_init(&app, window);
    lv_main_loop(window, &app);
    lv_app_destroy(&app);

    glfwTerminate();
}

/*
 * main program
 */

int main(int argc, char **argv)
{
    lv_ll = lv_ll_info;
    parse_options(argc, argv);
    lv_app_main(argc, argv);
    return 0;
}
