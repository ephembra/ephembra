/*
 * Copyright (c) 2023-2025 Michael Clark <michaeljclark@mac.com>
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

/* common */

#define lv_type_new(T) (T*)calloc(1, sizeof(T));

typedef unsigned long ulong;
typedef long long llong;
typedef unsigned long long ullong;
typedef unsigned int uint;
typedef unsigned char uchar;

/* logging */

typedef enum {
    lv_ll_trace, lv_ll_debug, lv_ll_info, lv_ll_error, lv_ll_none
} lv_log_level;

static lv_log_level lv_ll = lv_ll_info;

#define lv_panic(...) do { fprintf(stderr, __VA_ARGS__); exit(1); } while(0);
#define lv_trace(...) if (lv_ll <= lv_ll_trace) printf(__VA_ARGS__);
#define lv_debug(...) if (lv_ll <= lv_ll_debug) printf(__VA_ARGS__);
#define lv_info(...)  if (lv_ll <= lv_ll_info) printf(__VA_ARGS__);
#define lv_error(...)  if (lv_ll <= lv_ll_error) printf(__VA_ARGS__);

/* command */

#define lv_cmd_arg0(n)       (0)
#define lv_cmd_arg1(n,a)     (a)
#define lv_cmd_arg2(n,a,b)   (a | (b << 16))
#define lv_cmd_arg3(n,a,b,c) (a | (b << 16) | (c << 24))

#define lv_min(a,b) (((a)<(b))?(a):(b))
#define lv_max(a,b) (((a)>(b))?(a):(b))
#define lv_sign(val) ((0 < val) - (val < 0))

#define LV_ARGS_NARGS_X(a,b,c,d,e,f,g,h,n,...) n
#define LV_ARGS_NARGS(...) LV_ARGS_NARGS_X(__VA_ARGS__,7,6,5,4,3,2,1,0,)
#define LV_ARGS_CONCAT_X(a,b) a##b
#define LV_ARGS_CONCAT(a,b) LV_ARGS_CONCAT_X(a,b)
#define LV_ARGS_DISP(b,...) LV_ARGS_CONCAT(b,LV_ARGS_NARGS(__VA_ARGS__))(__VA_ARGS__)

#define lv_args(...) LV_ARGS_DISP(lv_cmd_arg,__VA_ARGS__)

struct lv_color {
        union {
                float rgba[4];
                float hsva[4];
                struct {
                        float r,g,b,a;
                        float h,s,v;
                };
        };
};
typedef struct lv_color lv_color;

struct lv_paint {
        float xform[6];
        float extent[2];
        float radius;
        float feather;
        lv_color innerColor;
        lv_color outerColor;
        int image;
};
typedef struct lv_paint lv_paint;

typedef struct lv_context lv_context;
typedef struct lv_vg_ops lv_vg_ops;

enum {
    lv_cmd_begin_frame,     /* w:float h:float r:float */
    lv_cmd_end_frame,       /* */
    lv_cmd_push,            /* */
    lv_cmd_pop,             /* */
    lv_cmd_reset,           /* */
    lv_cmd_fill_color,      /* c:color */
    lv_cmd_fill_paint,      /* p:paint */
    lv_cmd_fill,            /* */
    lv_cmd_stroke_color,    /* c:color */
    lv_cmd_stroke_paint,    /* p:paint */
    lv_cmd_stroke_width,    /* w:float */
    lv_cmd_stroke,          /* */
    lv_cmd_begin_path,      /* */
    lv_cmd_close_path,      /* */
    lv_cmd_path_winding,    /* dir:int */
    lv_cmd_miter_limit,     /* limit:float */
    lv_cmd_line_cap,        /* cap:int */
    lv_cmd_line_join,       /* join:int */

    lv_cmd_2d_translate,    /* v0:vec2 */
    lv_cmd_2d_rotate,       /* a0:float */
    lv_cmd_2d_skew_x,       /* a0:float */
    lv_cmd_2d_skew_y,       /* a0:float */
    lv_cmd_2d_scale,        /* v0:vec2 */
    lv_cmd_2d_move_to,      /* p0:vec2 */
    lv_cmd_2d_line_to,      /* p0:vec2 */
    lv_cmd_2d_quadratic_to, /* p0:vec2, p1:vec2 */
    lv_cmd_2d_bezier_to,    /* p0:vec2, p1:vec2, p2:vec2 */
    lv_cmd_2d_arc_to,       /* p0:vec2, p1:vec2, a0:float */
    lv_cmd_2d_arc,          /* c0:vec2f, r:float, a0:vec2f, dir:int */
    lv_cmd_2d_rect,         /* p0:vec2f, s0:vec2f */
    lv_cmd_2d_rounded_rect, /* p0:vec2f, s0:vec2f, r:float */
    lv_cmd_2d_ellipse,      /* c0:vec2f, r0:vec2f */
    lv_cmd_2d_circle,       /* c0:vec2f, r:float */

    lv_cmd_2d_text_font,    /* font:string */
    lv_cmd_2d_text_size,    /* size:float */
    lv_cmd_2d_text_leading, /* leading:float */
    lv_cmd_2d_text_tracking,/* tracking:float */
    lv_cmd_2d_text_blur,    /* blur:float */
    lv_cmd_2d_text_align,   /* align:int */
    lv_cmd_2d_text_bounds,  /* text:string */
    lv_cmd_2d_text_draw,    /* v0:vec2, text:string */

    lv_cmd_3d_transform,    /* v0:mat4x4 */
    lv_cmd_3d_translate,    /* v0:vec3 */
    lv_cmd_3d_rotate,       /* v0:vec3, a0:float */
    lv_cmd_3d_scale,        /* v0:vec3 */
    lv_cmd_3d_move_to,      /* p0:vec3 */
    lv_cmd_3d_line_to,      /* p0:vec3 */
    lv_cmd_3d_quadratic_to, /* p0:vec3, p1:vec3 */
    lv_cmd_3d_bezier_to,    /* p0:vec3, p1:vec3, p2:vec3 */
};

enum {
    lv_arg_end,
    lv_arg_int,
    lv_arg_float,
    lv_arg_vec2,
    lv_arg_vec3,
    lv_arg_vec4,
    lv_arg_mat3x3,
    lv_arg_mat4x4,
    lv_arg_color,
    lv_arg_paint,
    lv_arg_string,
};

static const int lv_cmd_args[] = {
    /* common */
    /* [lv_cmd_begin_frame]     = */ lv_args(lv_arg_float, lv_arg_float, lv_arg_float),
    /* [lv_cmd_end_frame]       = */ lv_args(),
    /* [lv_cmd_push]            = */ lv_args(),
    /* [lv_cmd_pop]             = */ lv_args(),
    /* [lv_cmd_reset]           = */ lv_args(),
    /* [lv_cmd_fill_color]      = */ lv_args(lv_arg_color),
    /* [lv_cmd_fill_paint]      = */ lv_args(lv_arg_paint),
    /* [lv_cmd_fill]            = */ lv_args(),
    /* [lv_cmd_stroke_color]    = */ lv_args(lv_arg_color),
    /* [lv_cmd_stroke_paint]    = */ lv_args(lv_arg_paint),
    /* [lv_cmd_stroke_width]    = */ lv_args(lv_arg_float),
    /* [lv_cmd_stroke]          = */ lv_args(),
    /* [lv_cmd_begin_path]      = */ lv_args(),
    /* [lv_cmd_close_path]      = */ lv_args(),
    /* [lv_cmd_path_winding]    = */ lv_args(lv_arg_int),
    /* [lv_cmd_miter_limit]     = */ lv_args(lv_arg_float),
    /* [lv_cmd_line_cap]        = */ lv_args(lv_arg_int),
    /* [lv_cmd_line_join]       = */ lv_args(lv_arg_int),

    /* 2d commands */
    /* [lv_cmd_2d_translate]    = */ lv_args(lv_arg_vec2),
    /* [lv_cmd_2d_rotate]       = */ lv_args(lv_arg_float),
    /* [lv_cmd_2d_skew_x]       = */ lv_args(lv_arg_float),
    /* [lv_cmd_2d_skew_y]       = */ lv_args(lv_arg_float),
    /* [lv_cmd_2d_scale]        = */ lv_args(lv_arg_vec2),
    /* [lv_cmd_2d_move_to]      = */ lv_args(lv_arg_vec2),
    /* [lv_cmd_2d_line_to]      = */ lv_args(lv_arg_vec2),
    /* [lv_cmd_2d_quadratic_to] = */ lv_args(lv_arg_vec2, lv_arg_vec2),
    /* [lv_cmd_2d_bezier_to]    = */ lv_args(lv_arg_vec2, lv_arg_vec2, lv_arg_vec2),
    /* [lv_cmd_2d_arc_to]       = */ lv_args(lv_arg_vec2, lv_arg_vec2, lv_arg_float),
    /* [lv_cmd_2d_arc]          = */ lv_args(lv_arg_vec2, lv_arg_float, lv_arg_vec2, lv_arg_int),
    /* [lv_cmd_2d_rect]         = */ lv_args(lv_arg_vec2, lv_arg_vec2),
    /* [lv_cmd_2d_rounded_rect] = */ lv_args(lv_arg_vec2, lv_arg_vec2, lv_arg_float),
    /* [lv_cmd_2d_ellipse]      = */ lv_args(lv_arg_vec2, lv_arg_vec2),
    /* [lv_cmd_2d_circle]       = */ lv_args(lv_arg_vec2, lv_arg_float),

    /* text commands */
    /* [lv_cmd_2d_text_font]    = */ lv_args(lv_arg_string),
    /* [lv_cmd_2d_text_size]    = */ lv_args(lv_arg_float),
    /* [lv_cmd_2d_text_leading] = */ lv_args(lv_arg_float),
    /* [lv_cmd_2d_text_tracking]= */ lv_args(lv_arg_float),
    /* [lv_cmd_2d_text_blur]    = */ lv_args(lv_arg_float),
    /* [lv_cmd_2d_text_align]   = */ lv_args(lv_arg_int),
    /* [lv_cmd_2d_text_bounds]  = */ lv_args(lv_arg_string),
    /* [lv_cmd_2d_text_draw]    = */ lv_args(lv_arg_vec2, lv_arg_string),

    /* 3d commands */
    /* [lv_cmd_3d_transform]    = */ lv_args(lv_arg_mat4x4),
    /* [lv_cmd_3d_translate]    = */ lv_args(lv_arg_vec3),
    /* [lv_cmd_3d_rotate]       = */ lv_args(lv_arg_vec3, lv_arg_float),
    /* [lv_cmd_3d_scale]        = */ lv_args(lv_arg_vec3),
    /* [lv_cmd_3d_move_to]      = */ lv_args(lv_arg_vec3),
    /* [lv_cmd_3d_line_to]      = */ lv_args(lv_arg_vec3),
    /* [lv_cmd_3d_quadratic_to] = */ lv_args(lv_arg_vec3, lv_arg_vec3),
    /* [lv_cmd_3d_bezier_to]    = */ lv_args(lv_arg_vec3, lv_arg_vec3, lv_arg_vec3),
};

static const char* lv_cmd_names[] = {
    /* common */
    /* [lv_cmd_begin_frame]     = */ "begin_frame",
    /* [lv_cmd_end_frame]       = */ "end_frame",
    /* [lv_cmd_push]            = */ "push",
    /* [lv_cmd_pop]             = */ "pop",
    /* [lv_cmd_reset]           = */ "reset",
    /* [lv_cmd_fill_color]      = */ "fill_color",
    /* [lv_cmd_fill_paint]      = */ "fill_paint",
    /* [lv_cmd_fill]            = */ "fill",
    /* [lv_cmd_stroke_color]    = */ "stroke_color",
    /* [lv_cmd_stroke_paint]    = */ "stroke_paint",
    /* [lv_cmd_stroke_width]    = */ "stroke_width",
    /* [lv_cmd_stroke]          = */ "stroke",
    /* [lv_cmd_begin_path]      = */ "begin_path",
    /* [lv_cmd_close_path]      = */ "close_path",
    /* [lv_cmd_path_winding]    = */ "path_winding",
    /* [lv_cmd_miter_limit]     = */ "miter_limit",
    /* [lv_cmd_line_cap]        = */ "line_cap",
    /* [lv_cmd_line_join]       = */ "line_join",

    /* 2d commands */
    /* [lv_cmd_2d_translate]    = */ "2d_translate",
    /* [lv_cmd_2d_rotate]       = */ "2d_rotate",
    /* [lv_cmd_2d_skew_x]       = */ "2d_skew_x",
    /* [lv_cmd_2d_skew_y]       = */ "2d_skew_y",
    /* [lv_cmd_2d_scale]        = */ "2d_scale",
    /* [lv_cmd_2d_move_to]      = */ "2d_move_to",
    /* [lv_cmd_2d_line_to]      = */ "2d_line_to",
    /* [lv_cmd_2d_quadratic_to] = */ "2d_quadratic_to",
    /* [lv_cmd_2d_bezier_to]    = */ "2d_bezier_to",
    /* [lv_cmd_2d_arc_to]       = */ "2d_arc_to",
    /* [lv_cmd_2d_arc]          = */ "2d_arc",
    /* [lv_cmd_2d_rect]         = */ "2d_rect",
    /* [lv_cmd_2d_rounded_rect] = */ "2d_rounded_rect",
    /* [lv_cmd_2d_ellipse]      = */ "2d_ellipse",
    /* [lv_cmd_2d_circle]       = */ "2d_circle",

    /* text commands */
    /* [lv_cmd_2d_text_font]    = */ "2d_text_font",
    /* [lv_cmd_2d_text_size]    = */ "2d_text_size",
    /* [lv_cmd_2d_text_leading] = */ "2d_text_leading",
    /* [lv_cmd_2d_text_tracking]= */ "2d_text_tracking",
    /* [lv_cmd_2d_text_blur]    = */ "2d_text_blur",
    /* [lv_cmd_2d_text_align]   = */ "2d_text_align",
    /* [lv_cmd_2d_text_bounds]  = */ "2d_text_bounds",
    /* [lv_cmd_2d_text_draw]    = */ "2d_text_draw",

    /* 3d commands */
    /* [lv_cmd_3d_transform]    = */ "3d_transform",
    /* [lv_cmd_3d_translate]    = */ "3d_translate",
    /* [lv_cmd_3d_rotate]       = */ "3d_rotate",
    /* [lv_cmd_3d_scale]        = */ "3d_scale",
    /* [lv_cmd_3d_move_to]      = */ "3d_move_to",
    /* [lv_cmd_3d_line_to]      = */ "3d_line_to",
    /* [lv_cmd_3d_quadratic_to] = */ "3d_quadratic_to",
    /* [lv_cmd_3d_bezier_to]    = */ "3d_bezier_to",
};

enum {
    lv_align_hleft      = 1<<0,
    lv_align_hcenter    = 1<<1,
    lv_align_hright     = 1<<2,
    lv_align_vtop       = 1<<3,
    lv_align_vmiddle    = 1<<4,
    lv_align_vbottom    = 1<<5,
    lv_align_vbaseline  = 1<<6,
};

enum {
    lv_butt,
    lv_round,
    lv_square,
    lv_bevel,
    lv_miter,
};

static vec2f lv_point_2d(float x, float y) { vec2f p = { x, y }; return p; }
static vec3f lv_point_3d(float x, float y, float z) { vec3f p = { x, y, z }; return p; }
static vec4f lv_point_4d(float x, float y, float z, float w) { vec4f p = { x, y, z, w }; return p; }

static vec3f lv_point_3d_2f(vec2f o, float z) { vec3f p = { o.x, o.y, z }; return p; }

static lv_color lv_rgbf(float r, float g, float b)
{
    lv_color c = { r, g, b, 1.0f }; return c;
}

static lv_color lv_rgbaf(float r, float g, float b, float a)
{
    lv_color c = { r, g, b, a }; return c;
}

static lv_color lv_rgb(unsigned char r, unsigned char g, unsigned char b)
{
    return lv_rgbf(r / 255.0f, g / 255.0f, b / 255.0f);
}

static lv_color lv_rgba(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    return lv_rgbaf(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
}

static lv_color lv_color_af(lv_color c, float a)
{
    return lv_rgbaf(c.r, c.g, c.b, a);
}

static lv_color lv_color_a(lv_color c, unsigned char a)
{
    return lv_rgbaf(c.r, c.g, c.b, a / 255.0f);
}

static void lv_vg_init(lv_context * ctx, const lv_vg_ops *ops, void *arg);
static void lv_vg_destory(lv_context * ctx);

static void lv_vg_begin_frame(lv_context * ctx, float width, float height, float ratio);
static void lv_vg_end_frame(lv_context * ctx);
static void lv_vg_push(lv_context * ctx);
static void lv_vg_pop(lv_context * ctx);
static void lv_vg_reset(lv_context * ctx);
static void lv_vg_begin_path(lv_context * ctx);
static void lv_vg_close_path(lv_context * ctx);
static void lv_vg_path_winding(lv_context * ctx, int dir);
static void lv_vg_miter_limit(lv_context * ctx, float limit);
static void lv_vg_line_cap(lv_context * ctx, int cap);
static void lv_vg_line_join(lv_context * ctx, int join);
static void lv_vg_fill_color(lv_context * ctx, lv_color c);
static void lv_vg_fill_paint(lv_context * ctx, lv_paint p);
static void lv_vg_fill(lv_context * ctx);
static void lv_vg_stroke_color(lv_context * ctx, lv_color c);
static void lv_vg_stroke_paint(lv_context * ctx, lv_paint p);
static void lv_vg_stroke_width(lv_context * ctx, float w);
static void lv_vg_stroke(lv_context * ctx);

static void lv_vg_2d_translate(lv_context * ctx, vec2f v0);
static void lv_vg_2d_rotate(lv_context * ctx, float a0);
static void lv_vg_2d_skew_x(lv_context * ctx, float a0);
static void lv_vg_2d_skew_y(lv_context * ctx, float a0);
static void lv_vg_2d_scale(lv_context * ctx, vec2f v0);
static void lv_vg_2d_move_to(lv_context * ctx, vec2f p0);
static void lv_vg_2d_line_to(lv_context * ctx, vec2f p0);
static void lv_vg_2d_quadratic_to(lv_context * ctx, vec2f c0, vec2f p0);
static void lv_vg_2d_bezier_to(lv_context * ctx, vec2f c0, vec2f c1, vec2f p0);
static void lv_vg_2d_arc_to(lv_context* ctx, vec2f p0, vec2f p1, float a0);
static void lv_vg_2d_arc(lv_context * ctx, vec2f c0, float r, vec2f a0, int dir);
static void lv_vg_2d_rect(lv_context* ctx, vec2f p0, vec2f s0);
static void lv_vg_2d_rounded_rect(lv_context* ctx, vec2f p0, vec2f s0, float r);
static void lv_vg_2d_ellipse(lv_context * ctx, vec2f c0, vec2f r0);
static void lv_vg_2d_circle(lv_context * ctx, vec2f c0, float r);

static void lv_vg_2d_text_font(lv_context * ctx, const char *font);
static void lv_vg_2d_text_size(lv_context * ctx, float size);
static void lv_vg_2d_text_leading(lv_context * ctx, float leading);
static void lv_vg_2d_text_tracking(lv_context * ctx, float tracking);
static void lv_vg_2d_text_blur(lv_context * ctx, float blur);
static void lv_vg_2d_text_align(lv_context * ctx, int align);
static vec2f lv_vg_2d_text_bounds(lv_context * ctx, const char *text);
static void lv_vg_2d_text_draw(lv_context * ctx, vec2f v0, const char *text);

static void lv_vg_3d_transform(lv_context * ctx, mat4x4 m0);
static void lv_vg_3d_translate(lv_context * ctx, vec3f v0);
static void lv_vg_3d_rotate(lv_context * ctx, vec3f v0, float a0);
static void lv_vg_3d_scale(lv_context * ctx, vec3f v0);
static void lv_vg_3d_move_to(lv_context * ctx, vec3f p0);
static void lv_vg_3d_line_to(lv_context * ctx, vec3f p0);
static void lv_vg_3d_quadratic_to(lv_context * ctx, vec3f c0, vec3f p0);
static void lv_vg_3d_bezier_to(lv_context * ctx, vec3f c0, vec3f c1, vec3f p0);


static float vec2f_dot(vec2f a, vec2f b)
{
    return a.x * b.x + a.y * b.y;
}

static float vec3f_dot(vec3f a, vec3f b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static float vec2f_cross(vec2f v1, vec2f v2)
{
    return v1.x * v2.y - v2.x * v1.y;
}

static vec3f vec3f_cross(vec3f v1, vec3f v2)
{
    return (vec3f) {
        v1.y * v2.z - v1.z * v2.y,
        v1.z * v2.x - v1.x * v2.z,
        v1.x * v2.y - v1.y * v2.x
    };
}

static vec3f vec3f_cross3(vec3f a, vec3f b, vec3f c)
{
    vec3f v1 = { b.x - a.x, b.y - a.y, b.z - a.z };
    vec3f v2 = { c.x - a.x, c.y - a.y, c.z - a.z };

    return vec3f_cross(v1, v2);
}

static vec2f vec2f_normalize(vec2f v)
{
    float r = sqrtf(vec2f_dot(v, v));
    return (r > 0) ? (vec2f) { v.x / r, v.y / r } : v;
}

static vec3f vec3f_normalize(vec3f v)
{
    float r = sqrtf(vec3f_dot(v, v));
    return (r > 0) ? (vec3f) { v.x / r, v.y / r, v.z / r } : v;
}

static vec2f vec2f_2d_project(vec2f v, mat4x4 m)
{
    vec4 a = { v.x, v.y, 0.f, 1.f };
    vec4 b;
    mat4x4_mul_vec4(b, m, a);
    float nr = 1.0f / b[3];
    vec2f r = { b[0] * nr, b[1] * nr };
    return r;
}

static vec2f vec2f_3d_project(vec3f v, mat4x4 m)
{
    vec4 a = { v.x, v.y, v.z, 1.f };
    vec4 b;
    mat4x4_mul_vec4(b, m, a);
    float nr = 1.0f / b[3];
    vec2f r = { b[0] * nr, b[1] * nr };
    return r;
}

static void mat4x4_rotate_make(mat4x4 m, vec3f v0, float a0)
{
    v0 = vec3f_normalize(v0);

    float ar = a0 * M_PI / 180.f;
    float s = sin(ar);
    float c = cos(ar);
    float a2 = v0.x * v0.x;
    float b2 = v0.y * v0.y;
    float c2 = v0.z * v0.z;
    float as = v0.x * s;
    float bs = v0.y * s;
    float cs = v0.z * s;
    float ab = v0.x * v0.y * (1 - c);
    float bc = v0.y * v0.z * (1 - c);
    float ca = v0.z * v0.x * (1 - c);

    m[0][0] = a2 + c * (1 - a2);
    m[0][1] = ab + cs;
    m[0][2] = ca - bs;
    m[0][3] = 0;
    m[1][0] = ab - cs;
    m[1][1] = b2 + c * (1 - b2);
    m[1][2] = bc + as;
    m[1][3] = 0;
    m[2][0] = ca + bs;
    m[2][1] = bc - as;
    m[2][2] = c2 + c * (1 - c2);
    m[2][3] = 0;
    m[3][0] = 0;
    m[3][1] = 0;
    m[3][2] = 0;
    m[3][3] = 1;
}


struct lv_context
{
    const lv_vg_ops * ops;
    void *priv;
    void *user;
};

struct lv_vg_ops
{
    void (*_init)(lv_context * ctx, void *arg);
    void (*_destroy)(lv_context * ctx);

    void (*_begin_frame)(lv_context * ctx, float width, float height, float ratio);
    void (*_end_frame)(lv_context * ctx);
    void (*_push)(lv_context * ctx);
    void (*_pop)(lv_context * ctx);
    void (*_reset)(lv_context * ctx);
    void (*_begin_path)(lv_context * ctx);
    void (*_close_path)(lv_context * ctx);
    void (*_path_winding)(lv_context * ctx, int dir);
    void (*_miter_limit)(lv_context * ctx, float limit);
    void (*_line_cap)(lv_context * ctx, int cap);
    void (*_line_join)(lv_context * ctx, int join);
    void (*_fill_color)(lv_context * ctx, lv_color c);
    void (*_fill_paint)(lv_context * ctx, lv_paint p);
    void (*_fill)(lv_context * ctx);
    void (*_stroke_color)(lv_context * ctx, lv_color c);
    void (*_stroke_paint)(lv_context * ctx, lv_paint p);
    void (*_stroke_width)(lv_context * ctx, float w);
    void (*_stroke)(lv_context * ctx);

    void (*_2d_translate)(lv_context * ctx, vec2f v0);
    void (*_2d_rotate)(lv_context * ctx, float a0);
    void (*_2d_skew_x)(lv_context * ctx, float a0);
    void (*_2d_skew_y)(lv_context * ctx, float a0);
    void (*_2d_scale)(lv_context * ctx, vec2f v0);
    void (*_2d_move_to)(lv_context * ctx, vec2f p0);
    void (*_2d_line_to)(lv_context * ctx, vec2f p0);
    void (*_2d_quadratic_to)(lv_context * ctx, vec2f c0, vec2f p0);
    void (*_2d_bezier_to)(lv_context * ctx, vec2f c0, vec2f c1, vec2f p0);
    void (*_2d_arc_to)(lv_context* ctx, vec2f p0, vec2f p1, float a0);
    void (*_2d_arc)(lv_context * ctx, vec2f c0, float r, vec2f a0, int dir);
    void (*_2d_rect)(lv_context* ctx, vec2f p0, vec2f s0);
    void (*_2d_rounded_rect)(lv_context* ctx, vec2f p0, vec2f s0, float r);
    void (*_2d_ellipse)(lv_context * ctx, vec2f c0, vec2f r0);
    void (*_2d_circle)(lv_context * ctx, vec2f c0, float r);

    void (*_2d_text_font)(lv_context * ctx, const char *font);
    void (*_2d_text_size)(lv_context * ctx, float size);
    void (*_2d_text_leading)(lv_context * ctx, float leading);
    void (*_2d_text_tracking)(lv_context * ctx, float tracking);
    void (*_2d_text_blur)(lv_context * ctx, float blur);
    void (*_2d_text_align)(lv_context * ctx, int align);
    vec2f (*_2d_text_bounds)(lv_context * ctx, const char *text);
    void (*_2d_text_draw)(lv_context * ctx, vec2f v0, const char *text);

    void (*_3d_transform)(lv_context * ctx, mat4x4 m0);
    void (*_3d_translate)(lv_context * ctx, vec3f v0);
    void (*_3d_rotate)(lv_context * ctx, vec3f v0, float a0);
    void (*_3d_scale)(lv_context * ctx, vec3f v0);
    void (*_3d_move_to)(lv_context * ctx, vec3f p0);
    void (*_3d_line_to)(lv_context * ctx, vec3f p0);
    void (*_3d_quadratic_to)(lv_context * ctx, vec3f c0, vec3f p0);
    void (*_3d_bezier_to)(lv_context * ctx, vec3f c0, vec3f c1, vec3f p0);
};

static void lv_vg_init(lv_context* ctx, const lv_vg_ops *ops, void *arg)
{
    memset(ctx, 0, sizeof(*ctx));
    ctx->ops = ops;
    ctx->ops->_init(ctx, arg);
}

static void lv_vg_destroy(lv_context* ctx)
{
    ctx->ops->_destroy(ctx);
}

static void lv_vg_begin_frame(lv_context* ctx, float width, float height, float ratio)
{
    ctx->ops->_begin_frame(ctx, width, height, ratio);
}

static void lv_vg_end_frame(lv_context* ctx)
{
    ctx->ops->_end_frame(ctx);
}

static void lv_vg_push(lv_context * ctx)
{
    ctx->ops->_push(ctx);
}

static void lv_vg_pop(lv_context * ctx)
{
    ctx->ops->_pop(ctx);
}

static void lv_vg_reset(lv_context * ctx)
{
    ctx->ops->_reset(ctx);
}

static void lv_vg_begin_path(lv_context * ctx)
{
    ctx->ops->_begin_path(ctx);
}

static void lv_vg_close_path(lv_context * ctx)
{
    ctx->ops->_close_path(ctx);
}

static void lv_vg_path_winding(lv_context * ctx, int dir)
{
    ctx->ops->_path_winding(ctx, dir);
}

static void lv_vg_miter_limit(lv_context * ctx, float limit)
{
    ctx->ops->_miter_limit(ctx, limit);
}

static void lv_vg_line_cap(lv_context * ctx, int cap)
{
    ctx->ops->_line_cap(ctx, cap);
}

static void lv_vg_line_join(lv_context * ctx, int join)
{
    ctx->ops->_line_join(ctx, join);
}

static void lv_vg_fill_color(lv_context * ctx, lv_color c)
{
    ctx->ops->_fill_color(ctx, c);
}

static void lv_vg_fill_paint(lv_context * ctx, lv_paint p)
{
    ctx->ops->_fill_paint(ctx, p);
}

static void lv_vg_fill(lv_context * ctx)
{
    ctx->ops->_fill(ctx);
}

static void lv_vg_stroke_color(lv_context * ctx, lv_color c)
{
    ctx->ops->_stroke_color(ctx, c);
}

static void lv_vg_stroke_paint(lv_context * ctx, lv_paint p)
{
    ctx->ops->_stroke_paint(ctx, p);
}

static void lv_vg_stroke_width(lv_context * ctx, float w)
{
    ctx->ops->_stroke_width(ctx, w);
}

static void lv_vg_stroke(lv_context * ctx)
{
    ctx->ops->_stroke(ctx);
}

static void lv_vg_2d_translate(lv_context * ctx, vec2f v0)
{
    ctx->ops->_2d_translate(ctx, v0);
}

static void lv_vg_2d_rotate(lv_context * ctx, float a0)
{
    ctx->ops->_2d_rotate(ctx, a0);
}

static void lv_vg_2d_skew_x(lv_context * ctx, float a0)
{
    ctx->ops->_2d_skew_x(ctx, a0);
}

static void lv_vg_2d_skew_y(lv_context * ctx, float a0)
{
    ctx->ops->_2d_skew_y(ctx, a0);
}

static void lv_vg_2d_scale(lv_context * ctx, vec2f v0)
{
    ctx->ops->_2d_scale(ctx, v0);
}

static void lv_vg_2d_move_to(lv_context * ctx, vec2f p0)
{
    ctx->ops->_2d_move_to(ctx, p0);
}

static void lv_vg_2d_line_to(lv_context * ctx, vec2f p0)
{
    ctx->ops->_2d_line_to(ctx, p0);
}

static void lv_vg_2d_quadratic_to(lv_context * ctx, vec2f c0, vec2f p0)
{
    ctx->ops->_2d_quadratic_to(ctx, c0, p0);
}

static void lv_vg_2d_bezier_to(lv_context * ctx, vec2f c0, vec2f c1, vec2f p0)
{
    ctx->ops->_2d_bezier_to(ctx, c0, c1, p0);
}

static void lv_vg_2d_arc_to(lv_context* ctx, vec2f p0, vec2f p1, float a0)
{
    ctx->ops->_2d_arc_to(ctx, p0, p1, a0);
}

static void lv_vg_2d_arc(lv_context * ctx, vec2f c0, float r, vec2f a0, int dir)
{
    ctx->ops->_2d_arc(ctx, c0, r, a0, dir);
}

static void lv_vg_2d_rect(lv_context* ctx, vec2f p0, vec2f s0)
{
    ctx->ops->_2d_rect(ctx, p0, s0);
}

static void lv_vg_2d_rounded_rect(lv_context* ctx, vec2f p0, vec2f s0, float r)
{
    ctx->ops->_2d_rounded_rect(ctx, p0, s0, r);
}

static void lv_vg_2d_ellipse(lv_context * ctx, vec2f c0, vec2f r0)
{
    ctx->ops->_2d_ellipse(ctx, c0, r0);
}

static void lv_vg_2d_circle(lv_context * ctx, vec2f c0, float r)
{
    ctx->ops->_2d_circle(ctx, c0, r);
}

static void lv_vg_2d_text_font(lv_context * ctx, const char *font)
{
    ctx->ops->_2d_text_font(ctx, font);
}

static void lv_vg_2d_text_size(lv_context * ctx, float size)
{
    ctx->ops->_2d_text_size(ctx, size);
}

static void lv_vg_2d_text_leading(lv_context * ctx, float leading)
{
    ctx->ops->_2d_text_leading(ctx, leading);
}

static void lv_vg_2d_text_tracking(lv_context * ctx, float tracking)
{
    ctx->ops->_2d_text_tracking(ctx, tracking);
}

static void lv_vg_2d_text_blur(lv_context * ctx, float blur)
{
    ctx->ops->_2d_text_blur(ctx, blur);
}

static void lv_vg_2d_text_align(lv_context * ctx, int align)
{
    ctx->ops->_2d_text_align(ctx, align);
}

static vec2f lv_vg_2d_text_bounds(lv_context * ctx, const char *text)
{
    return ctx->ops->_2d_text_bounds(ctx, text);
}

static void lv_vg_2d_text_draw(lv_context * ctx, vec2f v0, const char *text)
{
    ctx->ops->_2d_text_draw(ctx, v0, text);
}

static void lv_vg_3d_transform(lv_context * ctx, mat4x4 m0)
{
    ctx->ops->_3d_transform(ctx, m0);
}

static void lv_vg_3d_translate(lv_context * ctx, vec3f v0)
{
    ctx->ops->_3d_translate(ctx, v0);
}

static void lv_vg_3d_rotate(lv_context * ctx, vec3f v0, float a0)
{
    ctx->ops->_3d_rotate(ctx, v0, a0);
}

static void lv_vg_3d_scale(lv_context * ctx, vec3f v0)
{
    ctx->ops->_3d_scale(ctx, v0);
}

static void lv_vg_3d_move_to(lv_context * ctx, vec3f p0)
{
    ctx->ops->_3d_move_to(ctx, p0);
}

static void lv_vg_3d_line_to(lv_context * ctx, vec3f p0)
{
    ctx->ops->_3d_line_to(ctx, p0);
}

static void lv_vg_3d_quadratic_to(lv_context * ctx, vec3f c0, vec3f p0)
{
    ctx->ops->_3d_quadratic_to(ctx, c0, p0);
}

static void lv_vg_3d_bezier_to(lv_context * ctx, vec3f c0, vec3f c1, vec3f p0)
{
    ctx->ops->_3d_bezier_to(ctx, c0, c1, p0);
}

#ifdef __cplusplus
}
#endif
