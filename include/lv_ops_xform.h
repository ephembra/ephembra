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

enum { lv_xform_max_depth = 32 };

typedef struct lv_xform_vg_context lv_xform_vg_context;
struct lv_xform_vg_context
{
    lv_context * dst;
    size_t depth;
    mat4x4 m_proj, m_final;
    mat4x4 m_xform[lv_xform_max_depth];
    float w, h, r;
    int ndc;
};

#define array_buffer_append(buf, arr) \
array_buffer_append_impl(buf, (void*)arr, sizeof(arr)>>2)

static vec2f lv_xform_vec2f(lv_context *ctx, vec2f p0)
{
    lv_xform_vg_context *priv = (lv_xform_vg_context*)ctx->priv;
    vec2f r = vec2f_2d_project(p0, priv->m_final);
    if (priv->ndc) {
        r.x = (r.x * 0.5f + 0.5f) * priv->w;
        r.y = (r.y * 0.5f + 0.5f) * priv->h;
    }
    return r;
}

static vec2f lv_xform_vec3f(lv_context *ctx, vec3f p0)
{
    lv_xform_vg_context *priv = (lv_xform_vg_context*)ctx->priv;
    vec2f r = vec2f_3d_project(p0, priv->m_final);
    if (priv->ndc) {
        r.x = (r.x * 0.5f + 0.5f) * priv->w;
        r.y = (r.y * 0.5f + 0.5f) * priv->h;
    }
    return r;
}

static void lv_xform_proj_matrix(lv_context *ctx, mat4x4 m, int ndc)
{
    lv_xform_vg_context *priv = (lv_xform_vg_context*)ctx->priv;
    mat4x4_dup(priv->m_proj, m);
    mat4x4_mul(priv->m_final, priv->m_xform[priv->depth], priv->m_proj);
    priv->ndc = ndc;
}

static void lv_xform_mul_matrix(lv_context *ctx, mat4x4 m)
{
    lv_xform_vg_context *priv = (lv_xform_vg_context*)ctx->priv;
    if (priv->depth >= lv_xform_max_depth) abort();
    mat4x4 r;
    mat4x4_mul(r, priv->m_xform[priv->depth], m);
    mat4x4_dup(priv->m_xform[priv->depth], r);
    mat4x4_mul(priv->m_final, priv->m_proj, priv->m_xform[priv->depth]);
}

static void lv_xform_vg_init(lv_context* ctx, void *arg)
{
    lv_debug("trace: lv_xform_vg_init\n");
    lv_xform_vg_context *priv = lv_type_new(lv_xform_vg_context);
    ctx->priv = priv;
    priv->depth = 0;
    mat4x4_identity(priv->m_xform[0]);
    mat4x4_identity(priv->m_proj);
    mat4x4_identity(priv->m_final);
    priv->dst = (lv_context*)arg;
    priv->dst->ops->_reset(priv->dst);
}

static void lv_xform_vg_destroy(lv_context* ctx)
{
    lv_debug("trace: lv_xform_vg_destroy\n");
    free(ctx->priv);
}

static void lv_xform_vg_begin_frame(lv_context* ctx, float w, float h, float r)
{
    lv_debug("trace: lv_xform_vg_begin_frame: %f %f %f\n", w, h, r);
    lv_xform_vg_context *priv = (lv_xform_vg_context*)ctx->priv;
    priv->dst->ops->_begin_frame(priv->dst, w, h, r);
    priv->w = w;
    priv->h = h;
    priv->r = r;
}

static void lv_xform_vg_end_frame(lv_context* ctx)
{
    lv_debug("trace: lv_xform_vg_end_frame\n");
    lv_xform_vg_context *priv = (lv_xform_vg_context*)ctx->priv;
    priv->dst->ops->_end_frame(priv->dst);
}

static void lv_xform_vg_push(lv_context * ctx)
{
    lv_debug("trace: lv_xform_vg_push\n");
    lv_xform_vg_context *priv = (lv_xform_vg_context*)ctx->priv;
    if (priv->depth + 1 >= lv_xform_max_depth) return;
    mat4x4_dup(priv->m_xform[priv->depth + 1], priv->m_xform[priv->depth]);
    priv->depth++;
    priv->dst->ops->_push(priv->dst);
}

static void lv_xform_vg_pop(lv_context * ctx)
{
    lv_debug("trace: lv_xform_vg_pop\n");
    lv_xform_vg_context *priv = (lv_xform_vg_context*)ctx->priv;
    if (priv->depth - 1 >= lv_xform_max_depth) return;
    priv->depth--;
    mat4x4_mul(priv->m_final, priv->m_xform[priv->depth], priv->m_proj);
    priv->dst->ops->_pop(priv->dst);
}

static void lv_xform_vg_reset(lv_context * ctx)
{
    lv_debug("trace: lv_xform_vg_reset\n");
    lv_xform_vg_context *priv = (lv_xform_vg_context*)ctx->priv;
    mat4x4_identity(priv->m_xform[priv->depth]);
    mat4x4_mul(priv->m_final, priv->m_xform[priv->depth], priv->m_proj);
    priv->dst->ops->_reset(priv->dst);
}

static void lv_xform_vg_begin_path(lv_context * ctx)
{
    lv_debug("trace: lv_xform_begin_path\n");
    lv_xform_vg_context *priv = (lv_xform_vg_context*)ctx->priv;
    priv->dst->ops->_begin_path(priv->dst);
}

static void lv_xform_vg_close_path(lv_context * ctx)
{
    lv_debug("trace: lv_xform_close_path\n");
    lv_xform_vg_context *priv = (lv_xform_vg_context*)ctx->priv;
    priv->dst->ops->_close_path(priv->dst);
}

static void lv_xform_vg_path_winding(lv_context * ctx, int dir)
{
    lv_debug("trace: lv_xform_vg_path_winding: %d\n", dir);
    lv_xform_vg_context *priv = (lv_xform_vg_context*)ctx->priv;
    priv->dst->ops->_path_winding(priv->dst, dir);
}

static void lv_xform_vg_miter_limit(lv_context * ctx, float limit)
{
    lv_debug("trace: lv_xform_vg_miter_limit: %f\n", limit);
    lv_xform_vg_context *priv = (lv_xform_vg_context*)ctx->priv;
    priv->dst->ops->_miter_limit(priv->dst, limit);
}

static void lv_xform_vg_line_cap(lv_context * ctx, int cap)
{
    lv_debug("trace: lv_xform_vg_line_cap: %d\n", cap);
    lv_xform_vg_context *priv = (lv_xform_vg_context*)ctx->priv;
    priv->dst->ops->_line_cap(priv->dst, cap);
}

static void lv_xform_vg_line_join(lv_context * ctx, int join)
{
    lv_debug("trace: lv_xform_vg_line_join: %d\n", join);
    lv_xform_vg_context *priv = (lv_xform_vg_context*)ctx->priv;
    priv->dst->ops->_line_join(priv->dst, join);
}

static void lv_xform_vg_fill_color(lv_context * ctx, lv_color c)
{
    lv_debug("trace: lv_xform_vg_fill_color\n");
    lv_xform_vg_context *priv = (lv_xform_vg_context*)ctx->priv;
    priv->dst->ops->_fill_color(priv->dst, c);
}

static void lv_xform_vg_fill_paint(lv_context * ctx, lv_paint p)
{
    lv_debug("trace: lv_xform_vg_fill_paint\n");
    lv_xform_vg_context *priv = (lv_xform_vg_context*)ctx->priv;
    priv->dst->ops->_fill_paint(priv->dst, p);
}

static void lv_xform_vg_fill(lv_context * ctx)
{
    lv_debug("trace: lv_xform_vg_fill\n");
    lv_xform_vg_context *priv = (lv_xform_vg_context*)ctx->priv;
    priv->dst->ops->_fill(priv->dst);
}

static void lv_xform_vg_stroke_color(lv_context * ctx, lv_color c)
{
    lv_debug("trace: lv_xform_vg_stroke_color\n");
    lv_xform_vg_context *priv = (lv_xform_vg_context*)ctx->priv;
    priv->dst->ops->_stroke_color(priv->dst, c);
}

static void lv_xform_vg_stroke_paint(lv_context * ctx, lv_paint p)
{
    lv_debug("trace: lv_xform_vg_stroke_paint\n");
    lv_xform_vg_context *priv = (lv_xform_vg_context*)ctx->priv;
    priv->dst->ops->_stroke_paint(priv->dst, p);
}

static void lv_xform_vg_stroke_width(lv_context * ctx, float w)
{
    lv_debug("trace: lv_xform_vg_stroke_width\n");
    lv_xform_vg_context *priv = (lv_xform_vg_context*)ctx->priv;
    priv->dst->ops->_stroke_width(priv->dst, w);
}

static void lv_xform_vg_stroke(lv_context * ctx)
{
    lv_debug("trace: lv_xform_vg_stroke\n");
    lv_xform_vg_context *priv = (lv_xform_vg_context*)ctx->priv;
    priv->dst->ops->_stroke(priv->dst);
}

static void lv_xform_vg_2d_translate(lv_context * ctx, vec2f v0)
{
    lv_debug("trace: lv_xform_vg_2d_translate: %f %f\n", v0.x, v0.y);
    lv_xform_vg_context *priv = (lv_xform_vg_context*)ctx->priv;
    mat4x4 m = {
        {  1.f,  0.f,  0.f,  0.f },
        {  0.f,  1.f,  0.f,  0.f },
        {  0.f,  0.f,  1.f,  0.f },
        { v0.x, v0.y,  0.f,  1.f }
    };
    lv_xform_mul_matrix(ctx, m);
}

static void lv_xform_vg_2d_rotate(lv_context * ctx, float a0)
{
    lv_debug("trace: lv_xform_vg_2d_rotate: %f\n", a0);
    lv_xform_vg_context *priv = (lv_xform_vg_context*)ctx->priv;
    float cs = cosf(a0), sn = sinf(a0);
    mat4x4 m = {
        {   cs,   sn,  0.f,  0.f },
        {  -sn,   cs,  0.f,  0.f },
        {  0.f,  0.f,  1.f,  0.f },
        {  0.f,  0.f,  0.f,  1.f }
    };
    lv_xform_mul_matrix(ctx, m);
}

static void lv_xform_vg_2d_skew_x(lv_context * ctx, float a0)
{
    lv_debug("trace: lv_xform_vg_2d_skew_x: %f\n", a0);
    lv_xform_vg_context *priv = (lv_xform_vg_context*)ctx->priv;
    float t = tanf(a0);
    mat4x4 m = {
        {  1.f,  0.f,  0.f,  0.f },
        {    t,  1.f,  0.f,  0.f },
        {  0.f,  0.f,  1.f,  0.f },
        {  0.f,  0.f,  0.f,  1.f }
    };
    lv_xform_mul_matrix(ctx, m);
}

static void lv_xform_vg_2d_skew_y(lv_context * ctx, float a0)
{
    lv_debug("trace: lv_xform_vg_2d_skew_x: %f\n", a0);
    lv_xform_vg_context *priv = (lv_xform_vg_context*)ctx->priv;
    float t = tanf(a0);
    mat4x4 m = {
        {  1.f,    t,  0.f,  0.f },
        {  0.f,  1.f,  0.f,  0.f },
        {  0.f,  0.f,  1.f,  0.f },
        {  0.f,  0.f,  0.f,  1.f }
    };
    lv_xform_mul_matrix(ctx, m);
}

static void lv_xform_vg_2d_scale(lv_context * ctx, vec2f v0)
{
    lv_debug("trace: lv_xform_vg_2d_scale: %f %f\n", v0.x, v0.y);
    lv_xform_vg_context *priv = (lv_xform_vg_context*)ctx->priv;
    mat4x4 m = {
        { v0.x,  0.f,  0.f,  0.f },
        {  0.f, v0.y,  0.f,  0.f },
        {  0.f,  0.f,  1.f,  0.f },
        {  0.f,  0.f,  0.f,  1.f }
    };
    lv_xform_mul_matrix(ctx, m);
}

static void lv_xform_vg_2d_move_to(lv_context * ctx, vec2f p0)
{
    lv_debug("trace: lv_xform_vg_move_to: %f %f\n", p0.x, p0.y);
    lv_xform_vg_context *priv = (lv_xform_vg_context*)ctx->priv;
    priv->dst->ops->_2d_move_to(priv->dst, lv_xform_vec2f(ctx, p0));
}

static void lv_xform_vg_2d_line_to(lv_context * ctx, vec2f p0)
{
    lv_debug("trace: lv_xform_vg_line_to: %f, %f\n", p0.x, p0.y);
    lv_xform_vg_context *priv = (lv_xform_vg_context*)ctx->priv;
    priv->dst->ops->_2d_line_to(priv->dst, lv_xform_vec2f(ctx, p0));
}

static void lv_xform_vg_2d_quadratic_to(lv_context * ctx, vec2f c0, vec2f p0)
{
    lv_debug("trace: lv_xform_vg_quadratic_to: %f, %f, %f, %f\n",
        c0.x, c0.y, p0.x, p0.y);
    lv_xform_vg_context *priv = (lv_xform_vg_context*)ctx->priv;
    priv->dst->ops->_2d_quadratic_to(priv->dst, lv_xform_vec2f(ctx, c0),
        lv_xform_vec2f(ctx, p0));
}

static void lv_xform_vg_2d_bezier_to(lv_context * ctx, vec2f c0, vec2f c1, vec2f p0)
{
    lv_debug("trace: lv_xform_vg_bezier_to: %f, %f, %f, %f, %f, %f\n",
        c0.x, c0.y, c1.x, c1.y, p0.x, p0.y);
    lv_xform_vg_context *priv = (lv_xform_vg_context*)ctx->priv;
    priv->dst->ops->_2d_bezier_to(priv->dst, lv_xform_vec2f(ctx, c0),
        lv_xform_vec2f(ctx, c1), lv_xform_vec2f(ctx, p0));
}

static void lv_xform_vg_2d_arc_to(lv_context* ctx, vec2f p0, vec2f p1, float a0)
{
    lv_debug("trace: lv_xform_vg_2d_arc_to: %f, %f, %f, %f, %f\n",
        p0.x, p0.y, p1.x, p1.y, a0);
    lv_xform_vg_context *priv = (lv_xform_vg_context*)ctx->priv;
    // todo
}

static void lv_xform_vg_2d_arc(lv_context * ctx, vec2f c0, float r, vec2f a0, int dir)
{
    lv_debug("trace: lv_xform_vg_2d_arc: %f, %f, %f, %f, %f, %d\n",
        c0.x, c0.y, r, a0.x, a0.y, dir);
    lv_xform_vg_context *priv = (lv_xform_vg_context*)ctx->priv;
    // todo
}

static void lv_xform_vg_2d_rect(lv_context* ctx, vec2f p0, vec2f s0)
{
    lv_debug("trace: lv_xform_vg_2d_rect: %f, %f, %f, %f\n",
        p0.x, p0.y, s0.x, s0.y);
    lv_xform_vg_context *priv = (lv_xform_vg_context*)ctx->priv;
    lv_xform_vg_2d_move_to(ctx, (vec2f) { p0.x, p0.y } );
    lv_xform_vg_2d_line_to(ctx, (vec2f) { p0.x, p0.y + s0.y } );
    lv_xform_vg_2d_line_to(ctx, (vec2f) { p0.x + s0.x, p0.y + s0.y } );
    lv_xform_vg_2d_line_to(ctx, (vec2f) { p0.x + s0.x, p0.y } );
    lv_xform_vg_close_path(ctx);
}

static void lv_xform_vg_2d_rounded_rect(lv_context* ctx, vec2f p0, vec2f s0, float r0)
{
    lv_debug("trace: lv_xform_vg_2d_rounded_rect: %f, %f, %f, %f, %f\n",
        p0.x, p0.y, s0.x, s0.y, r0);
    lv_xform_vg_context *priv = (lv_xform_vg_context*)ctx->priv;
    /* length proportional to radius of a cubic bezier handle for 90deg arcs */
    const float k = 4.f/3.f*(sqrtf(2.f)-1.f), j = 1.f - k;
    vec2f r = { lv_min(r0, fabsf(s0.x) * 0.5f) * lv_sign(s0.x),
                 lv_min(r0, fabsf(s0.y) * 0.5f) * lv_sign(s0.y) };
    lv_xform_vg_2d_move_to  (ctx, (vec2f) { p0.x, p0.y + r.y });
    lv_xform_vg_2d_line_to  (ctx, (vec2f) { p0.x, p0.y + s0.y - r.y });
    lv_xform_vg_2d_bezier_to(ctx, (vec2f) { p0.x, p0.y + s0.y - r.y * j },
                                  (vec2f) { p0.x + r.x * j, p0.y + s0.y },
                                  (vec2f) { p0.x + r.x, p0.y + s0.y });
    lv_xform_vg_2d_line_to  (ctx, (vec2f) { p0.x + s0.x - r.x, p0.y + s0.y });
    lv_xform_vg_2d_bezier_to(ctx, (vec2f) { p0.x + s0.x - r.x * j, p0.y + s0.y },
                                  (vec2f) { p0.x + s0.x, p0.y + s0.y - r.y * j },
                                  (vec2f) { p0.x + s0.x, p0.y + s0.y - r.y });
    lv_xform_vg_2d_line_to  (ctx, (vec2f) { p0.x + s0.x, p0.y + r.y });
    lv_xform_vg_2d_bezier_to(ctx, (vec2f) { p0.x + s0.x, p0.y + r.y * j },
                                  (vec2f) { p0.x + s0.x - r.x * j, p0.y },
                                  (vec2f) { p0.x + s0.x - r.x, p0.y });
    lv_xform_vg_2d_line_to  (ctx, (vec2f) { p0.x + r.x, p0.y });
    lv_xform_vg_2d_bezier_to(ctx, (vec2f) { p0.x + r.x * j, p0.y, },
                                  (vec2f) { p0.x, p0.y + r.y * j },
                                  (vec2f) { p0.x, p0.y + r.y });
    lv_xform_vg_close_path(ctx);
}

static void lv_xform_vg_2d_ellipse(lv_context * ctx, vec2f c0, vec2f r0)
{
    lv_debug("trace: lv_xform_vg_2d_ellipse: %f, %f, %f, %f\n",
        c0.x, c0.y, r0.x, r0.y);
    lv_xform_vg_context *priv = (lv_xform_vg_context*)ctx->priv;
    /* length proportional to radius of a cubic bezier handle for 90deg arcs */
    const float k = 4.f/3.f*(sqrtf(2.f)-1.f), j = 1.f - k;
    lv_xform_vg_2d_move_to  (ctx, (vec2f) { c0.x-r0.x, c0.y });
    lv_xform_vg_2d_bezier_to(ctx, (vec2f) { c0.x-r0.x, c0.y+r0.y * k },
                                  (vec2f) { c0.x-r0.x * k, c0.y+r0.y },
                                  (vec2f) { c0.x, c0.y+r0.y });
    lv_xform_vg_2d_bezier_to(ctx, (vec2f) { c0.x+r0.x * k, c0.y+r0.y },
                                  (vec2f) { c0.x+r0.x, c0.y+r0.y * k },
                                  (vec2f) { c0.x+r0.x, c0.y });
    lv_xform_vg_2d_bezier_to(ctx, (vec2f) { c0.x+r0.x, c0.y-r0.y * k },
                                  (vec2f) { c0.x+r0.x * k, c0.y-r0.y },
                                  (vec2f) { c0.x, c0.y-r0.y });
    lv_xform_vg_2d_bezier_to(ctx, (vec2f) { c0.x-r0.x * k, c0.y-r0.y },
                                  (vec2f) { c0.x-r0.x, c0.y-r0.y * k },
                                  (vec2f) { c0.x-r0.x, c0.y });
    lv_xform_vg_close_path(ctx);
}

static void lv_xform_vg_2d_circle(lv_context * ctx, vec2f c0, float r)
{
    lv_debug("trace: lv_xform_vg_2d_circle: %f, %f, %f\n",
        c0.x, c0.y, r);
    vec2f r0 = { r, r };
    lv_xform_vg_2d_ellipse(ctx, c0, r0);
}

static void lv_xform_vg_2d_text_font(lv_context * ctx, const char *font)
{
    lv_debug("trace: lv_xform_vg_2d_text_font: %s\n", font);
    // todo
}

static void lv_xform_vg_2d_text_size(lv_context * ctx, float size)
{
    lv_debug("trace: lv_xform_vg_2d_text_size: %f\n", size);
    // todo
}

static void lv_xform_vg_2d_text_leading(lv_context * ctx, float leading)
{
    lv_debug("trace: lv_xform_vg_2d_text_leading: %f\n", leading);
    // todo
}

static void lv_xform_vg_2d_text_tracking(lv_context * ctx, float tracking)
{
    lv_debug("trace: lv_xform_vg_2d_text_tracking: %f\n", tracking);
    // todo
}

static void lv_xform_vg_2d_text_blur(lv_context * ctx, float blur)
{
    lv_debug("trace: lv_xform_vg_2d_text_blur: %f\n", blur);
    // todo
}

static void lv_xform_vg_2d_text_align(lv_context * ctx, int align)
{
    lv_debug("trace: lv_xform_vg_2d_text_align: %d\n", align);
    // todo
}

static vec2f lv_xform_vg_2d_text_bounds(lv_context * ctx, const char *text)
{
    lv_debug("trace: lv_xform_vg_2d_text_bounds: %s\n", text);
    // todo
    vec2f r = { 0, 0 };
    return r;
}

static void lv_xform_vg_2d_text_draw(lv_context * ctx, vec2f v0, const char *text)
{
    lv_debug("trace: lv_xform_vg_2d_text_draw: %f %f %s\n", v0.x, v0.y, text);
    // todo
}

static void lv_xform_vg_3d_transform(lv_context * ctx, mat4x4 m0)
{
    lv_debug("trace: lv_xform_vg_3d_transform: ( %f, %f, %f, %f ), "
        "( %f, %f, %f, %f ), ( %f, %f, %f, %f ), ( %f, %f, %f, %f )\n",
        m0[0][0], m0[0][1], m0[0][2], m0[0][3],
        m0[1][0], m0[1][1], m0[1][2], m0[1][3],
        m0[2][0], m0[2][1], m0[2][2], m0[2][3],
        m0[3][0], m0[3][1], m0[3][2], m0[3][3]);
    lv_xform_vg_context *priv = (lv_xform_vg_context*)ctx->priv;
    lv_xform_mul_matrix(ctx, m0);
}

static void lv_xform_vg_3d_translate(lv_context * ctx, vec3f v0)
{
    lv_debug("trace: lv_xform_vg_3d_translate: ( %f, %f, %f )\n",
        v0.x, v0.y, v0.z);
    lv_xform_vg_context *priv = (lv_xform_vg_context*)ctx->priv;
    mat4x4 m = {
        {  1.f,  0.f,  0.f,  0.f },
        {  0.f,  1.f,  0.f,  0.f },
        {  0.f,  0.f,  1.f,  0.f },
        { v0.x, v0.y, v0.z,  1.f }
    };
    lv_xform_mul_matrix(ctx, m);
}

static void lv_xform_vg_3d_rotate(lv_context * ctx, vec3f v0, float a0)
{
    lv_debug("trace: lv_xform_vg_3d_rotate: ( %f, %f, %f ), %f\n",
        v0.x, v0.y, v0.z, a0);
    lv_xform_vg_context *priv = (lv_xform_vg_context*)ctx->priv;
    mat4x4 m;
    mat4x4_rotate_make(m, v0, a0);
    lv_xform_mul_matrix(ctx, m);
}

static void lv_xform_vg_3d_scale(lv_context * ctx, vec3f v0)
{
    lv_debug("trace: lv_xform_vg_3d_scale: ( %f, %f, %f )\n",
        v0.x, v0.y, v0.z);
    lv_xform_vg_context *priv = (lv_xform_vg_context*)ctx->priv;
    mat4x4 m = {
        { v0.x,  0.f,  0.f,  0.f },
        {  0.f, v0.y,  0.f,  0.f },
        {  0.f,  0.f, v0.z,  0.f },
        {  0.f,  0.f,  0.f,  1.f }
    };
    lv_xform_mul_matrix(ctx, m);
}

static void lv_xform_vg_3d_move_to(lv_context * ctx, vec3f p0)
{
    lv_debug("trace: lv_xform_vg_3d_move_to: ( %f, %f, %f )\n",
        p0.x, p0.y, p0.z);
    lv_xform_vg_context *priv = (lv_xform_vg_context*)ctx->priv;
    priv->dst->ops->_2d_move_to(priv->dst, lv_xform_vec3f(ctx, p0));
}

static void lv_xform_vg_3d_line_to(lv_context * ctx, vec3f p0)
{
    lv_debug("trace: lv_xform_vg_3d_line_to: ( %f, %f, %f )\n",
        p0.x, p0.y, p0.z);
    lv_xform_vg_context *priv = (lv_xform_vg_context*)ctx->priv;
    priv->dst->ops->_2d_line_to(priv->dst, lv_xform_vec3f(ctx, p0));
}

static void lv_xform_vg_3d_quadratic_to(lv_context * ctx, vec3f c0, vec3f p0)
{
    lv_debug("trace: lv_xform_vg_3d_quadratic_to: ( %f, %f, %f ), ( %f, %f, %f )\n",
        c0.x, c0.y, c0.z, p0.x, p0.y, p0.z);
    lv_xform_vg_context *priv = (lv_xform_vg_context*)ctx->priv;
    priv->dst->ops->_2d_quadratic_to(priv->dst, lv_xform_vec3f(ctx, c0),
        lv_xform_vec3f(ctx, p0));
}

static void lv_xform_vg_3d_bezier_to(lv_context * ctx, vec3f c0, vec3f c1, vec3f p0)
{
    lv_debug("trace: lv_xform_vg_3d_bezier_to: ( %f, %f, %f ), ( %f, %f, %f ), "
        "( %f, %f, %f )\n",
        c0.x, c0.y, c0.z, c1.x, c1.y, c1.z, p0.x, p0.y, p0.z);
    lv_xform_vg_context *priv = (lv_xform_vg_context*)ctx->priv;
    priv->dst->ops->_2d_bezier_to(priv->dst, lv_xform_vec3f(ctx, c0),
        lv_xform_vec3f(ctx, c1), lv_xform_vec3f(ctx, p0));
}

static const lv_vg_ops lv_xform_vg_ops =
{
    lv_xform_vg_init,
    lv_xform_vg_destroy,

    lv_xform_vg_begin_frame,
    lv_xform_vg_end_frame,
    lv_xform_vg_push,
    lv_xform_vg_pop,
    lv_xform_vg_reset,
    lv_xform_vg_begin_path,
    lv_xform_vg_close_path,
    lv_xform_vg_path_winding,
    lv_xform_vg_miter_limit,
    lv_xform_vg_line_cap,
    lv_xform_vg_line_join,
    lv_xform_vg_fill_color,
    lv_xform_vg_fill_paint,
    lv_xform_vg_fill,
    lv_xform_vg_stroke_color,
    lv_xform_vg_stroke_paint,
    lv_xform_vg_stroke_width,
    lv_xform_vg_stroke,

    lv_xform_vg_2d_translate,
    lv_xform_vg_2d_rotate,
    lv_xform_vg_2d_skew_x,
    lv_xform_vg_2d_skew_y,
    lv_xform_vg_2d_scale,
    lv_xform_vg_2d_move_to,
    lv_xform_vg_2d_line_to,
    lv_xform_vg_2d_quadratic_to,
    lv_xform_vg_2d_bezier_to,
    lv_xform_vg_2d_arc_to,
    lv_xform_vg_2d_arc,
    lv_xform_vg_2d_rect,
    lv_xform_vg_2d_rounded_rect,
    lv_xform_vg_2d_ellipse,
    lv_xform_vg_2d_circle,

    lv_xform_vg_2d_text_font,
    lv_xform_vg_2d_text_size,
    lv_xform_vg_2d_text_leading,
    lv_xform_vg_2d_text_tracking,
    lv_xform_vg_2d_text_blur,
    lv_xform_vg_2d_text_align,
    lv_xform_vg_2d_text_bounds,
    lv_xform_vg_2d_text_draw,

    lv_xform_vg_3d_transform,
    lv_xform_vg_3d_translate,
    lv_xform_vg_3d_rotate,
    lv_xform_vg_3d_scale,
    lv_xform_vg_3d_move_to,
    lv_xform_vg_3d_line_to,
    lv_xform_vg_3d_quadratic_to,
    lv_xform_vg_3d_bezier_to,
};

#ifdef __cplusplus
}
#endif
