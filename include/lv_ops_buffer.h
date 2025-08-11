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

typedef struct lv_buffer_vg_context lv_buffer_vg_context;
struct lv_buffer_vg_context
{
    array_buffer buf;
};

static void array_buffer_append_impl(array_buffer * buf, void *a, size_t n)
{
    size_t o = array_buffer_count(buf);
    array_buffer_resize(buf, o + n);
    uint *p = (uint*)array_buffer_data(buf) + o;
    memcpy(p, a, n * sizeof(uint));
}

#define array_buffer_append(buf, arr) \
array_buffer_append_impl(buf, (void*)arr, sizeof(arr)>>2)

static void lv_buffer_vg_init(lv_context* ctx, void *arg)
{
    lv_debug("trace: lv_buffer_vg_init\n");
    lv_buffer_vg_context *priv = lv_type_new(lv_buffer_vg_context);
    array_buffer_init(&priv->buf, sizeof(uint), 16);
    ctx->priv = priv;
}

static void lv_buffer_vg_destroy(lv_context* ctx)
{
    lv_debug("trace: lv_buffer_vg_destroy\n");
    lv_buffer_vg_context *priv = (lv_buffer_vg_context*)ctx->priv;
    array_buffer_destroy(&priv->buf);
    free(ctx->priv);
}

static void lv_buffer_vg_begin_frame(lv_context* ctx, float width, float height, float ratio)
{
    lv_debug("trace: lv_buffer_vg_begin_frame: %f %f %f\n", width, height, ratio);
    lv_buffer_vg_context *priv = (lv_buffer_vg_context*)ctx->priv;
    uint cmd[] = { lv_cmd_begin_frame };
    float vals[] = { width, height, ratio };
    array_buffer_append(&priv->buf, cmd);
    array_buffer_append(&priv->buf, vals);
}

static void lv_buffer_vg_end_frame(lv_context* ctx)
{
    lv_debug("trace: lv_buffer_vg_end_frame\n");
    lv_buffer_vg_context *priv = (lv_buffer_vg_context*)ctx->priv;
    uint cmd[] = { lv_cmd_end_frame };
    array_buffer_append(&priv->buf, cmd);
}

static void lv_buffer_vg_push(lv_context * ctx)
{
    lv_debug("trace: lv_buffer_vg_push\n");
    lv_buffer_vg_context *priv = (lv_buffer_vg_context*)ctx->priv;
    uint cmd[] = { lv_cmd_push };
    array_buffer_append(&priv->buf, cmd);
}

static void lv_buffer_vg_pop(lv_context * ctx)
{
    lv_debug("trace: lv_buffer_vg_pop\n");
    lv_buffer_vg_context *priv = (lv_buffer_vg_context*)ctx->priv;
    uint cmd[] = { lv_cmd_pop };
    array_buffer_append(&priv->buf, cmd);
}

static void lv_buffer_vg_reset(lv_context * ctx)
{
    lv_debug("trace: lv_buffer_vg_reset\n");
    lv_buffer_vg_context *priv = (lv_buffer_vg_context*)ctx->priv;
    uint cmd[] = { lv_cmd_reset };
    array_buffer_append(&priv->buf, cmd);
}

static void lv_buffer_vg_begin_path(lv_context * ctx)
{
    lv_debug("trace: lv_buffer_begin_path\n");
    lv_buffer_vg_context *priv = (lv_buffer_vg_context*)ctx->priv;
    uint cmd[] = { lv_cmd_begin_path };
    array_buffer_append(&priv->buf, cmd);
}

static void lv_buffer_vg_close_path(lv_context * ctx)
{
    lv_debug("trace: lv_buffer_close_path\n");
    lv_buffer_vg_context *priv = (lv_buffer_vg_context*)ctx->priv;
    uint cmd[] = { lv_cmd_close_path };
    array_buffer_append(&priv->buf, cmd);
}

static void lv_buffer_vg_path_winding(lv_context * ctx, int dir)
{
    lv_debug("trace: lv_buffer_vg_path_winding: %d\n", dir);
    lv_buffer_vg_context *priv = (lv_buffer_vg_context*)ctx->priv;
    uint cmd[] = { lv_cmd_path_winding, (uint)dir };
    array_buffer_append(&priv->buf, cmd);
}

static void lv_buffer_vg_miter_limit(lv_context * ctx, float limit)
{
    lv_debug("trace: lv_buffer_vg_miter_limit: %f\n", limit);
    lv_buffer_vg_context *priv = (lv_buffer_vg_context*)ctx->priv;
    uint cmd[] = { lv_cmd_miter_limit };
    float param[] = { limit };
    array_buffer_append(&priv->buf, cmd);
    array_buffer_append(&priv->buf, param);
}

static void lv_buffer_vg_line_cap(lv_context * ctx, int cap)
{
    lv_debug("trace: lv_buffer_vg_line_cap: %d\n", cap);
    lv_buffer_vg_context *priv = (lv_buffer_vg_context*)ctx->priv;
    uint cmd[] = { lv_cmd_line_cap, (uint)cap };
    array_buffer_append(&priv->buf, cmd);
}

static void lv_buffer_vg_line_join(lv_context * ctx, int join)
{
    lv_debug("trace: lv_buffer_vg_line_join: %d\n", join);
    lv_buffer_vg_context *priv = (lv_buffer_vg_context*)ctx->priv;
    uint cmd[] = { lv_cmd_line_join, (uint)join };
    array_buffer_append(&priv->buf, cmd);
}

static void lv_buffer_vg_fill_color(lv_context * ctx, lv_color c)
{
    lv_debug("trace: lv_buffer_vg_fill_color\n");
    lv_buffer_vg_context *priv = (lv_buffer_vg_context*)ctx->priv;
    uint cmd[] = { lv_cmd_fill_color };
    array_buffer_append(&priv->buf, cmd);
    array_buffer_append_impl(&priv->buf, &c, sizeof(c)>>2);
}

static void lv_buffer_vg_fill_paint(lv_context * ctx, lv_paint p)
{
    lv_debug("trace: lv_buffer_vg_fill_paint\n");
    lv_buffer_vg_context *priv = (lv_buffer_vg_context*)ctx->priv;
    uint cmd[] = { lv_cmd_fill_paint };
    array_buffer_append(&priv->buf, cmd);
    array_buffer_append_impl(&priv->buf, &p, sizeof(p)>>2);
}

static void lv_buffer_vg_fill(lv_context * ctx)
{
    lv_debug("trace: lv_buffer_vg_fill\n");
    lv_buffer_vg_context *priv = (lv_buffer_vg_context*)ctx->priv;
    uint cmd[] = { lv_cmd_fill };
    array_buffer_append(&priv->buf, cmd);
}

static void lv_buffer_vg_stroke_color(lv_context * ctx, lv_color c)
{
    lv_debug("trace: lv_buffer_vg_stroke_color\n");
    lv_buffer_vg_context *priv = (lv_buffer_vg_context*)ctx->priv;
    uint cmd[] = { lv_cmd_stroke_color };
    array_buffer_append(&priv->buf, cmd);
    array_buffer_append_impl(&priv->buf, &c, sizeof(c)>>2);
}

static void lv_buffer_vg_stroke_paint(lv_context * ctx, lv_paint p)
{
    lv_debug("trace: lv_buffer_vg_stroke_paint\n");
    lv_buffer_vg_context *priv = (lv_buffer_vg_context*)ctx->priv;
    uint cmd[] = { lv_cmd_stroke_paint };
    array_buffer_append(&priv->buf, cmd);
    array_buffer_append_impl(&priv->buf, &p, sizeof(p)>>2);
}


static void lv_buffer_vg_stroke_width(lv_context * ctx, float w)
{
    lv_debug("trace: lv_buffer_vg_stroke_width\n");
    lv_buffer_vg_context *priv = (lv_buffer_vg_context*)ctx->priv;
    uint cmd[] = { lv_cmd_stroke_width };
    float param[] = { w };
    array_buffer_append(&priv->buf, cmd);
    array_buffer_append(&priv->buf, param);
}

static void lv_buffer_vg_stroke(lv_context * ctx)
{
    lv_debug("trace: lv_buffer_vg_stroke\n");
    lv_buffer_vg_context *priv = (lv_buffer_vg_context*)ctx->priv;
    uint cmd[] = { lv_cmd_stroke };
    array_buffer_append(&priv->buf, cmd);
}

static void lv_buffer_vg_2d_translate(lv_context * ctx, vec2f v0)
{
    lv_debug("trace: lv_buffer_vg_2d_translate: %f %f\n", v0.x, v0.y);
    lv_buffer_vg_context *priv = (lv_buffer_vg_context*)ctx->priv;
    uint cmd[] = { lv_cmd_2d_translate };
    float param[] = { v0.x, v0.y };
    array_buffer_append(&priv->buf, cmd);
    array_buffer_append(&priv->buf, param);
}

static void lv_buffer_vg_2d_rotate(lv_context * ctx, float a0)
{
    lv_debug("trace: lv_buffer_vg_2d_rotate: %f\n", a0);
    lv_buffer_vg_context *priv = (lv_buffer_vg_context*)ctx->priv;
    uint cmd[] = { lv_cmd_2d_rotate };
    float param[] = { a0 };
    array_buffer_append(&priv->buf, cmd);
    array_buffer_append(&priv->buf, param);
}

static void lv_buffer_vg_2d_skew_x(lv_context * ctx, float a0)
{
    lv_debug("trace: lv_buffer_vg_2d_skew_x: %f\n", a0);
    lv_buffer_vg_context *priv = (lv_buffer_vg_context*)ctx->priv;
    uint cmd[] = { lv_cmd_2d_skew_x };
    float param[] = { a0 };
    array_buffer_append(&priv->buf, cmd);
    array_buffer_append(&priv->buf, param);
}

static void lv_buffer_vg_2d_skew_y(lv_context * ctx, float a0)
{
    lv_debug("trace: lv_buffer_vg_2d_skew_x: %f\n", a0);
    lv_buffer_vg_context *priv = (lv_buffer_vg_context*)ctx->priv;
    uint cmd[] = { lv_cmd_2d_skew_y };
    float param[] = { a0 };
    array_buffer_append(&priv->buf, cmd);
    array_buffer_append(&priv->buf, param);
}

static void lv_buffer_vg_2d_scale(lv_context * ctx, vec2f v0)
{
    lv_debug("trace: lv_buffer_vg_2d_scale: %f %f\n", v0.x, v0.y);
    lv_buffer_vg_context *priv = (lv_buffer_vg_context*)ctx->priv;
    uint cmd[] = { lv_cmd_2d_scale };
    float param[] = { v0.x, v0.y };
    array_buffer_append(&priv->buf, cmd);
    array_buffer_append(&priv->buf, param);
}

static void lv_buffer_vg_2d_move_to(lv_context * ctx, vec2f p0)
{
    lv_debug("trace: lv_buffer_vg_move_to: %f %f\n", p0.x, p0.y);
    lv_buffer_vg_context *priv = (lv_buffer_vg_context*)ctx->priv;
    uint cmd[] = { lv_cmd_2d_move_to };
    float param[] = { p0.x, p0.y };
    array_buffer_append(&priv->buf, cmd);
    array_buffer_append(&priv->buf, param);
}

static void lv_buffer_vg_2d_line_to(lv_context * ctx, vec2f p0)
{
    lv_debug("trace: lv_buffer_vg_line_to: %f, %f\n", p0.x, p0.y);
    lv_buffer_vg_context *priv = (lv_buffer_vg_context*)ctx->priv;
    uint cmd[] = { lv_cmd_2d_line_to };
    float param[] = { p0.x, p0.y };
    array_buffer_append(&priv->buf, cmd);
    array_buffer_append(&priv->buf, param);
}

static void lv_buffer_vg_2d_quadratic_to(lv_context * ctx, vec2f c0, vec2f p0)
{
    lv_debug("trace: lv_buffer_vg_quadratic_to: %f, %f, %f, %f\n",
        c0.x, c0.y, p0.x, p0.y);
    lv_buffer_vg_context *priv = (lv_buffer_vg_context*)ctx->priv;
    uint cmd[] = { lv_cmd_2d_quadratic_to };
    float param[] = { c0.x, c0.y, p0.x, p0.y };
    array_buffer_append(&priv->buf, cmd);
    array_buffer_append(&priv->buf, param);
}

static void lv_buffer_vg_2d_bezier_to(lv_context * ctx, vec2f c0, vec2f c1, vec2f p0)
{
    lv_debug("trace: lv_buffer_vg_bezier_to: %f, %f, %f, %f, %f, %f\n",
        c0.x, c0.y, c1.x, c1.y, p0.x, p0.y);
    lv_buffer_vg_context *priv = (lv_buffer_vg_context*)ctx->priv;
    uint cmd[] = { lv_cmd_2d_bezier_to };
    float param[] = { c0.x, c0.y, c1.x, c1.y, p0.x, p0.y };
    array_buffer_append(&priv->buf, cmd);
    array_buffer_append(&priv->buf, param);
}

static void lv_buffer_vg_2d_arc_to(lv_context* ctx, vec2f p0, vec2f p1, float a0)
{
    lv_debug("trace: lv_buffer_vg_2d_arc_to: %f, %f, %f, %f, %f\n",
        p0.x, p0.y, p1.x, p1.y, a0);
    lv_buffer_vg_context *priv = (lv_buffer_vg_context*)ctx->priv;
    uint cmd[] = { lv_cmd_2d_arc_to };
    float param[] = { p0.x, p0.y, p1.x, p1.y, a0 };
    array_buffer_append(&priv->buf, cmd);
    array_buffer_append(&priv->buf, param);
}

static void lv_buffer_vg_2d_arc(lv_context * ctx, vec2f c0, float r, vec2f a0, int dir)
{
    lv_debug("trace: lv_buffer_vg_2d_arc: %f, %f, %f, %f, %f, %d\n",
        c0.x, c0.y, r, a0.x, a0.y, dir);
    lv_buffer_vg_context *priv = (lv_buffer_vg_context*)ctx->priv;
    uint cmd[] = { lv_cmd_2d_arc };
    float param[] = { c0.x, c0.y, r, a0.x, a0.y };
    uint parami[] = { (uint)dir };
    array_buffer_append(&priv->buf, cmd);
    array_buffer_append(&priv->buf, param);
    array_buffer_append(&priv->buf, parami);
}

static void lv_buffer_vg_2d_rect(lv_context* ctx, vec2f p0, vec2f s0)
{
    lv_debug("trace: lv_buffer_vg_2d_rect: %f, %f, %f, %f\n",
        p0.x, p0.y, s0.x, s0.y);
    lv_buffer_vg_context *priv = (lv_buffer_vg_context*)ctx->priv;
    uint cmd[] = { lv_cmd_2d_rect };
    float param[] = { p0.x, p0.y, s0.x, s0.y };
    array_buffer_append(&priv->buf, cmd);
    array_buffer_append(&priv->buf, param);
}

static void lv_buffer_vg_2d_rounded_rect(lv_context* ctx, vec2f p0, vec2f s0, float r)
{
    lv_debug("trace: lv_buffer_vg_2d_rounded_rect: %f, %f, %f, %f, %f\n",
        p0.x, p0.y, s0.x, s0.y, r);
    lv_buffer_vg_context *priv = (lv_buffer_vg_context*)ctx->priv;
    uint cmd[] = { lv_cmd_2d_rounded_rect };
    float param[] = { p0.x, p0.y, s0.x, s0.y, r };
    array_buffer_append(&priv->buf, cmd);
    array_buffer_append(&priv->buf, param);
}

static void lv_buffer_vg_2d_ellipse(lv_context * ctx, vec2f c0, vec2f r0)
{
    lv_debug("trace: lv_buffer_vg_2d_ellipse: %f, %f, %f, %f\n",
        c0.x, c0.y, r0.x, r0.y);
    lv_buffer_vg_context *priv = (lv_buffer_vg_context*)ctx->priv;
    uint cmd[] = { lv_cmd_2d_ellipse };
    float param[] = { c0.x, c0.y, r0.x, r0.y };
    array_buffer_append(&priv->buf, cmd);
    array_buffer_append(&priv->buf, param);
}

static void lv_buffer_vg_2d_circle(lv_context * ctx, vec2f c0, float r)
{
    lv_debug("trace: lv_buffer_vg_2d_circle: %f, %f, %f\n",
        c0.x, c0.y, r);
    lv_buffer_vg_context *priv = (lv_buffer_vg_context*)ctx->priv;
    uint cmd[] = { lv_cmd_2d_circle };
    float param[] = { c0.x, c0.y, r };
    array_buffer_append(&priv->buf, cmd);
    array_buffer_append(&priv->buf, param);
}

static void lv_buffer_vg_2d_text_font(lv_context * ctx, const char *font)
{
    lv_debug("trace: lv_buffer_vg_2d_text_font: %s\n", font);
    lv_buffer_vg_context *priv = (lv_buffer_vg_context*)ctx->priv;
    uint len = strlen(font);
    uint cmd[] = { lv_cmd_2d_text_font, len };
    array_buffer_append(&priv->buf, cmd);
    array_buffer_append_impl(&priv->buf, (void*)font, len);
}

static void lv_buffer_vg_2d_text_size(lv_context * ctx, float size)
{
    lv_debug("trace: lv_buffer_vg_2d_text_size: %f\n", size);
    lv_buffer_vg_context *priv = (lv_buffer_vg_context*)ctx->priv;
    uint cmd[] = { lv_cmd_2d_text_size };
    float param[] = { size };
    array_buffer_append(&priv->buf, cmd);
    array_buffer_append(&priv->buf, param);
}

static void lv_buffer_vg_2d_text_leading(lv_context * ctx, float leading)
{
    lv_debug("trace: lv_buffer_vg_2d_text_leading: %f\n", leading);
    lv_buffer_vg_context *priv = (lv_buffer_vg_context*)ctx->priv;
    uint cmd[] = { lv_cmd_2d_text_leading };
    float param[] = { leading };
    array_buffer_append(&priv->buf, cmd);
    array_buffer_append(&priv->buf, param);
}

static void lv_buffer_vg_2d_text_tracking(lv_context * ctx, float tracking)
{
    lv_debug("trace: lv_buffer_vg_2d_text_tracking: %f\n", tracking);
    lv_buffer_vg_context *priv = (lv_buffer_vg_context*)ctx->priv;
    uint cmd[] = { lv_cmd_2d_text_tracking };
    float param[] = { tracking };
    array_buffer_append(&priv->buf, cmd);
    array_buffer_append(&priv->buf, param);
}

static void lv_buffer_vg_2d_text_blur(lv_context * ctx, float blur)
{
    lv_debug("trace: lv_buffer_vg_2d_text_blur: %f\n", blur);
    lv_buffer_vg_context *priv = (lv_buffer_vg_context*)ctx->priv;
    uint cmd[] = { lv_cmd_2d_text_blur };
    float param[] = { blur };
    array_buffer_append(&priv->buf, cmd);
    array_buffer_append(&priv->buf, param);
}

static void lv_buffer_vg_2d_text_align(lv_context * ctx, int align)
{
    lv_debug("trace: lv_buffer_vg_2d_text_align: %d\n", align);
    lv_buffer_vg_context *priv = (lv_buffer_vg_context*)ctx->priv;
    uint cmd[] = { lv_cmd_2d_text_align, (uint)align };
    array_buffer_append(&priv->buf, cmd);
}

static vec2f lv_buffer_vg_2d_text_bounds(lv_context * ctx, const char *text)
{
    lv_debug("trace: lv_buffer_vg_2d_text_bounds: %s\n", text);
    // todo
    vec2f r = { 0, 0 };
    return r;
}

static void lv_buffer_vg_2d_text_draw(lv_context * ctx, vec2f v0, const char *text)
{
    lv_debug("trace: lv_buffer_vg_2d_text_draw: %f %f %s\n", v0.x, v0.y, text);
    lv_buffer_vg_context *priv = (lv_buffer_vg_context*)ctx->priv;
    uint len = strlen(text);
    uint cmd[] = { lv_cmd_2d_text_draw };
    float param[] = { v0.x, v0.y };
    uint slen[] = { len };
    array_buffer_append(&priv->buf, cmd);
    array_buffer_append(&priv->buf, param);
    array_buffer_append(&priv->buf, slen);
    array_buffer_append_impl(&priv->buf, (void*)text, len);
}

static void lv_buffer_vg_3d_transform(lv_context * ctx, mat4x4 m0)
{
    lv_debug("trace: lv_buffer_vg_3d_transform: ( %f, %f, %f, %f ), "
        "( %f, %f, %f, %f ), ( %f, %f, %f, %f ), ( %f, %f, %f, %f )\n",
        m0[0][0], m0[0][1], m0[0][2], m0[0][3],
        m0[1][0], m0[1][1], m0[1][2], m0[1][3],
        m0[2][0], m0[2][1], m0[2][2], m0[2][3],
        m0[3][0], m0[3][1], m0[3][2], m0[3][3]);
    lv_buffer_vg_context *priv = (lv_buffer_vg_context*)ctx->priv;
    uint cmd[] = { lv_cmd_3d_transform };
    array_buffer_append(&priv->buf, cmd);
    array_buffer_append_impl(&priv->buf, &m0, 16);
}

static void lv_buffer_vg_3d_translate(lv_context * ctx, vec3f v0)
{
    lv_debug("trace: lv_buffer_vg_3d_translate: ( %f, %f, %f )\n",
        v0.x, v0.y, v0.z);
    lv_buffer_vg_context *priv = (lv_buffer_vg_context*)ctx->priv;
    uint cmd[] = { lv_cmd_3d_translate };
    float param[] = { v0.x, v0.y, v0.z };
    array_buffer_append(&priv->buf, cmd);
    array_buffer_append(&priv->buf, param);
}

static void lv_buffer_vg_3d_rotate(lv_context * ctx, vec3f v0, float a0)
{
    lv_debug("trace: lv_buffer_vg_3d_rotate: ( %f, %f, %f ), %f\n",
        v0.x, v0.y, v0.z, a0);
    lv_buffer_vg_context *priv = (lv_buffer_vg_context*)ctx->priv;
    uint cmd[] = { lv_cmd_3d_rotate };
    float param[] = { v0.x, v0.y, v0.z, a0 };
    array_buffer_append(&priv->buf, cmd);
    array_buffer_append(&priv->buf, param);
}

static void lv_buffer_vg_3d_scale(lv_context * ctx, vec3f v0)
{
    lv_debug("trace: lv_buffer_vg_3d_scale: ( %f, %f, %f )\n",
        v0.x, v0.y, v0.z);
    lv_buffer_vg_context *priv = (lv_buffer_vg_context*)ctx->priv;
    uint cmd[] = { lv_cmd_3d_scale };
    float param[] = { v0.x, v0.y, v0.z };
    array_buffer_append(&priv->buf, cmd);
    array_buffer_append(&priv->buf, param);
}

static void lv_buffer_vg_3d_move_to(lv_context * ctx, vec3f p0)
{
    lv_debug("trace: lv_buffer_vg_3d_move_to: ( %f, %f, %f )\n",
        p0.x, p0.y, p0.z);
    lv_buffer_vg_context *priv = (lv_buffer_vg_context*)ctx->priv;
    uint cmd[] = { lv_cmd_3d_move_to };
    float param[] = { p0.x, p0.y, p0.z };
    array_buffer_append(&priv->buf, cmd);
    array_buffer_append(&priv->buf, param);
}

static void lv_buffer_vg_3d_line_to(lv_context * ctx, vec3f p0)
{
    lv_debug("trace: lv_buffer_vg_3d_line_to: ( %f, %f, %f )\n",
        p0.x, p0.y, p0.z);
    lv_buffer_vg_context *priv = (lv_buffer_vg_context*)ctx->priv;
    uint cmd[] = { lv_cmd_3d_line_to };
    float param[] = { p0.x, p0.y, p0.z };
    array_buffer_append(&priv->buf, cmd);
    array_buffer_append(&priv->buf, param);
}

static void lv_buffer_vg_3d_quadratic_to(lv_context * ctx, vec3f c0, vec3f p0)
{
    lv_debug("trace: lv_buffer_vg_3d_quadratic_to: ( %f, %f, %f ), ( %f, %f, %f )\n",
        c0.x, c0.y, c0.z, p0.x, p0.y, p0.z);
    lv_buffer_vg_context *priv = (lv_buffer_vg_context*)ctx->priv;
    uint cmd[] = { lv_cmd_3d_quadratic_to };
    float param[] = { c0.x, c0.y, c0.z, p0.x, p0.y, p0.z };
    array_buffer_append(&priv->buf, cmd);
    array_buffer_append(&priv->buf, param);
}

static void lv_buffer_vg_3d_bezier_to(lv_context * ctx, vec3f c0, vec3f c1, vec3f p0)
{
    lv_debug("trace: lv_buffer_vg_3d_bezier_to: ( %f, %f, %f ), ( %f, %f, %f ), "
        "( %f, %f, %f )\n",
        c0.x, c0.y, c0.z, c1.x, c1.y, c1.z, p0.x, p0.y, p0.z);
    lv_buffer_vg_context *priv = (lv_buffer_vg_context*)ctx->priv;
    uint cmd[] = { lv_cmd_3d_bezier_to };
    float param[] = { c0.x, c0.y, c0.z, c1.x, c1.y, c1.z, p0.x, p0.y, p0.z };
    array_buffer_append(&priv->buf, cmd);
    array_buffer_append(&priv->buf, param);
}

static const lv_vg_ops lv_buffer_vg_ops =
{
    lv_buffer_vg_init,
    lv_buffer_vg_destroy,

    lv_buffer_vg_begin_frame,
    lv_buffer_vg_end_frame,
    lv_buffer_vg_push,
    lv_buffer_vg_pop,
    lv_buffer_vg_reset,
    lv_buffer_vg_begin_path,
    lv_buffer_vg_close_path,
    lv_buffer_vg_path_winding,
    lv_buffer_vg_miter_limit,
    lv_buffer_vg_line_cap,
    lv_buffer_vg_line_join,
    lv_buffer_vg_fill_color,
    lv_buffer_vg_fill_paint,
    lv_buffer_vg_fill,
    lv_buffer_vg_stroke_color,
    lv_buffer_vg_stroke_paint,
    lv_buffer_vg_stroke_width,
    lv_buffer_vg_stroke,

    lv_buffer_vg_2d_translate,
    lv_buffer_vg_2d_rotate,
    lv_buffer_vg_2d_skew_x,
    lv_buffer_vg_2d_skew_y,
    lv_buffer_vg_2d_scale,
    lv_buffer_vg_2d_move_to,
    lv_buffer_vg_2d_line_to,
    lv_buffer_vg_2d_quadratic_to,
    lv_buffer_vg_2d_bezier_to,
    lv_buffer_vg_2d_arc_to,
    lv_buffer_vg_2d_arc,
    lv_buffer_vg_2d_rect,
    lv_buffer_vg_2d_rounded_rect,
    lv_buffer_vg_2d_ellipse,
    lv_buffer_vg_2d_circle,

    lv_buffer_vg_2d_text_font,
    lv_buffer_vg_2d_text_size,
    lv_buffer_vg_2d_text_leading,
    lv_buffer_vg_2d_text_tracking,
    lv_buffer_vg_2d_text_blur,
    lv_buffer_vg_2d_text_align,
    lv_buffer_vg_2d_text_bounds,
    lv_buffer_vg_2d_text_draw,

    lv_buffer_vg_3d_transform,
    lv_buffer_vg_3d_translate,
    lv_buffer_vg_3d_rotate,
    lv_buffer_vg_3d_scale,
    lv_buffer_vg_3d_move_to,
    lv_buffer_vg_3d_line_to,
    lv_buffer_vg_3d_quadratic_to,
    lv_buffer_vg_3d_bezier_to,
};

static void lv_buffer_vg_clear(lv_context* ctx)
{
    lv_buffer_vg_context *priv = (lv_buffer_vg_context*)ctx->priv;
    array_buffer_resize(&priv->buf, 0);
}

static void lv_buffer_vg_playback(lv_context * src, lv_context * dst)
{
    lv_buffer_vg_context *priv = (lv_buffer_vg_context*)src->priv;
    size_t count = array_buffer_count(&priv->buf);
    uint *p = (uint*)array_buffer_data(&priv->buf);
    float *f = (float*)array_buffer_data(&priv->buf);
    size_t i = 0;

    lv_color lvc;
    lv_paint lvp;
    uint p0;
    float f0, f1, f2;
    vec2f v0, v1, v2;
    vec3f w0, w1, w2;
    mat4x4 m0;
    char *str;

    while (i < count) {
        uint cmd = p[i++];
        switch (cmd) {
        case lv_cmd_begin_frame:
            f0 = f[i+0];
            f1 = f[i+1];
            f2 = f[i+2];
            i += 3;
            lv_vg_begin_frame(dst, f0, f1, f2);
            break;
        case lv_cmd_end_frame:
            lv_vg_end_frame(dst);
            break;
        case lv_cmd_push:
            lv_vg_push(dst);
            break;
        case lv_cmd_pop:
            lv_vg_pop(dst);
            break;
        case lv_cmd_reset:
            lv_vg_reset(dst);
            break;
        case lv_cmd_begin_path:
            lv_vg_begin_path(dst);
            break;
        case lv_cmd_close_path:
            lv_vg_close_path(dst);
            break;
        case lv_cmd_path_winding:
            p0 = p[i+0];
            i += 1;
            lv_vg_path_winding(dst, p0);
            break;
        case lv_cmd_miter_limit:
            f0 = f[i+0];
            i += 1;
            lv_vg_miter_limit(dst, f0);
            break;
        case lv_cmd_line_cap:
            p0 = p[i+0];
            i += 1;
            lv_vg_line_cap(dst, p0);
            break;
        case lv_cmd_line_join:
            p0 = p[i+0];
            i += 1;
            lv_vg_line_join(dst, p0);
            break;
        case lv_cmd_fill_color:
            memcpy(&lvc, (void*)(p + i), sizeof(lvc));
            i += sizeof(lvc)>>2;
            lv_vg_fill_color(dst, lvc);
            break;
        case lv_cmd_fill_paint:
            memcpy(&lvp, (void*)(p + i), sizeof(lvp));
            i += sizeof(lvp)>>2;
            lv_vg_fill_paint(dst, lvp);
            break;
        case lv_cmd_fill:
            lv_vg_fill(dst);
            break;
        case lv_cmd_stroke_color:
            memcpy(&lvc, (void*)(p + i), sizeof(lvc));
            i += sizeof(lvc)>>2;
            lv_vg_stroke_color(dst, lvc);
            break;
        case lv_cmd_stroke_paint:
            memcpy(&lvp, (void*)(p + i), sizeof(lvp));
            i += sizeof(lvp)>>2;
            lv_vg_stroke_paint(dst, lvp);
            break;
        case lv_cmd_stroke_width:
            f0 = f[i+0];
            i += 1;
            lv_vg_stroke_width(dst, f0);
            break;
        case lv_cmd_stroke:
            lv_vg_stroke(dst);
            break;
        case lv_cmd_2d_translate:
            v0 = (vec2f){ f[i+0], f[i+1] };
            i += 2;
            lv_vg_2d_translate(dst, v0);
            break;
        case lv_cmd_2d_rotate:
            f0 = f[i+0];
            i += 1;
            lv_vg_2d_rotate(dst, f0);
            break;
        case lv_cmd_2d_skew_x:
            f0 = f[i+0];
            i += 1;
            lv_vg_2d_skew_x(dst, f0);
            break;
        case lv_cmd_2d_skew_y:
            f0 = f[i+0];
            i += 1;
            lv_vg_2d_skew_y(dst, f0);
            break;
        case lv_cmd_2d_scale:
            v0 = (vec2f){ f[i+0], f[i+1] };
            i += 2;
            lv_vg_2d_scale(dst, v0);
            break;
        case lv_cmd_2d_move_to:
            v0 = (vec2f){ f[i+0], f[i+1] };
            i += 2;
            lv_vg_2d_move_to(dst, v0);
            break;
        case lv_cmd_2d_line_to:
            v0 = (vec2f){ f[i+0], f[i+1] };
            i += 2;
            lv_vg_2d_line_to(dst, v0);
            break;
        case lv_cmd_2d_quadratic_to:
            v0 = (vec2f){ f[i+0], f[i+1] };
            v1 = (vec2f){ f[i+2], f[i+3] };
            i += 4;
            lv_vg_2d_quadratic_to(dst, v0, v1);
            break;
        case lv_cmd_2d_bezier_to:
            v0 = (vec2f){ f[i+0], f[i+1] };
            v1 = (vec2f){ f[i+2], f[i+3] };
            v2 = (vec2f){ f[i+4], f[i+5] };
            i += 6;
            lv_vg_2d_bezier_to(dst, v0, v1, v2);
            break;
        case lv_cmd_2d_arc_to:
            v0 = (vec2f){ f[i+0], f[i+1] };
            v1 = (vec2f){ f[i+2], f[i+3] };
            f0 = f[i+4];
            i += 5;
            lv_vg_2d_arc_to(dst, v0, v1, f0);
            break;
        case lv_cmd_2d_arc:
            v0 = (vec2f){ f[i+0], f[i+1] };
            f0 = f[i+2];
            v1 = (vec2f){ f[i+3], f[i+4] };
            p0 = p[i+5];
            i += 6;
            lv_vg_2d_arc(dst, v0, f0, v1, p0);
            break;
        case lv_cmd_2d_rect:
            v0 = (vec2f){ f[i+0], f[i+1] };
            v1 = (vec2f){ f[i+2], f[i+3] };
            i += 4;
            lv_vg_2d_rect(dst, v0, v1);
            break;
        case lv_cmd_2d_rounded_rect:
            v0 = (vec2f){ f[i+0], f[i+1] };
            v1 = (vec2f){ f[i+2], f[i+3] };
            f0 = f[i+4];
            i += 5;
            lv_vg_2d_rounded_rect(dst, v0, v1, f0);
            break;
        case lv_cmd_2d_ellipse:
            v0 = (vec2f){ f[i+0], f[i+1] };
            v1 = (vec2f){ f[i+2], f[i+3] };
            i += 4;
            lv_vg_2d_ellipse(dst, v0, v1);
            break;
        case lv_cmd_2d_circle:
            v0 = (vec2f){ f[i+0], f[i+1] };
            f0 = f[i+2];
            i += 3;
            lv_vg_2d_circle(dst, v0, f0);
            break;
        case lv_cmd_2d_text_font:
            p0 = p[i+0];
            str = (char*)alloca(p0+1);
            memcpy(str, (const char*)&p[i+1], p0);
            str[p0] = '\0';
            lv_vg_2d_text_font(dst, str);
            i += (1 + ((p0+3)>>2));
            break;
        case lv_cmd_2d_text_size:
            f0 = f[i+0];
            i += 1;
            lv_vg_2d_text_size(dst, f0);
            break;
        case lv_cmd_2d_text_leading:
            f0 = f[i+0];
            i += 1;
            lv_vg_2d_text_leading(dst, f0);
            break;
        case lv_cmd_2d_text_tracking:
            f0 = f[i+0];
            i += 1;
            lv_vg_2d_text_tracking(dst, f0);
            break;
        case lv_cmd_2d_text_blur:
            f0 = f[i+0];
            i += 1;
            lv_vg_2d_text_blur(dst, f0);
            break;
        case lv_cmd_2d_text_align:
            p0 = p[i+0];
            i += 1;
            lv_vg_2d_text_align(dst, p0);
            break;
        case lv_cmd_2d_text_bounds:
            /* nop */
            break;
        case lv_cmd_2d_text_draw:
            v0 = (vec2f){ f[i+0], f[i+1] };
            p0 = p[i+2];
            str = (char*)alloca(p0+1);
            memcpy(str, (const char*)&p[i+3], p0);
            str[p0] = '\0';
            lv_vg_2d_text_draw(dst, v0, str);
            i += (3 + ((p0+3)>>2));
            break;
        case lv_cmd_3d_transform:
            memcpy(m0, f + i, 16 * sizeof(float));
            i+= 16;
            lv_vg_3d_transform(dst, m0);
            break;
        case lv_cmd_3d_translate:
            w0 = (vec3f){ f[i+0], f[i+1], f[i+2] };
            i += 3;
            lv_vg_3d_translate(dst, w0);
            break;
        case lv_cmd_3d_rotate:
            w0 = (vec3f){ f[i+0], f[i+1], f[i+2] };
            f0 = f[i+3];
            i += 4;
            lv_vg_3d_rotate(dst, w0, f0);
            break;
        case lv_cmd_3d_scale:
            w0 = (vec3f){ f[i+0], f[i+1], f[i+2] };
            i += 3;
            lv_vg_3d_scale(dst, w0);
            break;
        case lv_cmd_3d_move_to:
            w0 = (vec3f){ f[i+0], f[i+1], f[i+2] };
            i += 3;
            lv_vg_3d_move_to(dst, w0);
            break;
        case lv_cmd_3d_line_to:
            w0 = (vec3f){ f[i+0], f[i+1], f[i+2] };
            i += 3;
            lv_vg_3d_line_to(dst, w0);
            break;
        case lv_cmd_3d_quadratic_to:
            w0 = (vec3f){ f[i+0], f[i+1], f[i+2] };
            w1 = (vec3f){ f[i+3], f[i+4], f[i+5] };
            i += 6;
            lv_vg_3d_quadratic_to(dst, w0, w1);
            break;
        case lv_cmd_3d_bezier_to:
            w0 = (vec3f){ f[i+0], f[i+1], f[i+2] };
            w1 = (vec3f){ f[i+3], f[i+4], f[i+5] };
            w2 = (vec3f){ f[i+6], f[i+7], f[i+8] };
            i += 9;
            lv_vg_3d_bezier_to(dst, w0, w1, w2);
            break;
        }
    }
}

#ifdef __cplusplus
}
#endif
