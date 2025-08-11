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

typedef struct lv_nanovg_vg_context lv_nanovg_vg_context;
struct lv_nanovg_vg_context
{
    NVGcontext* vg;
    int fontNormal;
    int fontBold;
};

static void lv_nanovg_vg_init(lv_context* ctx, void *arg)
{
    static const char* dejavu_regular_fontpath = "fonts/DejaVuSans-Bold.ttf";
    static const char* dejavu_bold_fontpath = "fonts/DejaVuSans-Bold.ttf";

    lv_debug("trace: lv_nanovg_vg_init\n");
    lv_nanovg_vg_context *priv = lv_type_new(lv_nanovg_vg_context);
    priv->vg = nvgCreateGLES3(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);
    if (!priv->vg) {
        lv_panic("lv_context_init: error initializing nanovg\n");
    }
    priv->fontNormal = nvgCreateFont(priv->vg, "sans", dejavu_regular_fontpath);
    priv->fontBold = nvgCreateFont(priv->vg, "sans-bold", dejavu_bold_fontpath);
    ctx->priv = priv;
}

static void lv_nanovg_vg_destroy(lv_context* ctx)
{
    lv_debug("trace: lv_nanovg_vg_destroy\n");
    lv_nanovg_vg_context *priv = (lv_nanovg_vg_context*)ctx->priv;
    nvgDeleteGLES3(priv->vg);
    free(ctx->priv);
}

static void lv_nanovg_vg_begin_frame(lv_context* ctx, float width, float height, float ratio)
{
    lv_debug("trace: lv_nanovg_vg_begin_frame: %f %f %f\n", width, height, ratio);
    lv_nanovg_vg_context *priv = (lv_nanovg_vg_context*)ctx->priv;
    nvgBeginFrame(priv->vg, width, height, ratio);
}

static void lv_nanovg_vg_end_frame(lv_context* ctx)
{
    lv_debug("trace: lv_nanovg_vg_end_frame\n");
    lv_nanovg_vg_context *priv = (lv_nanovg_vg_context*)ctx->priv;
    nvgEndFrame(priv->vg);
}

static void lv_nanovg_vg_push(lv_context * ctx)
{
    lv_debug("trace: lv_nanovg_vg_push\n");
    lv_nanovg_vg_context *priv = (lv_nanovg_vg_context*)ctx->priv;
    nvgSave(priv->vg);
}

static void lv_nanovg_vg_pop(lv_context * ctx)
{
    lv_debug("trace: lv_nanovg_vg_pop\n");
    lv_nanovg_vg_context *priv = (lv_nanovg_vg_context*)ctx->priv;
    nvgRestore(priv->vg);
}

static void lv_nanovg_vg_reset(lv_context * ctx)
{
    lv_debug("trace: lv_nanovg_vg_reset\n");
    lv_nanovg_vg_context *priv = (lv_nanovg_vg_context*)ctx->priv;
    nvgReset(priv->vg);
}

static void lv_nanovg_vg_begin_path(lv_context * ctx)
{
    lv_debug("trace: lv_nanovg_vg_begin_path\n");
    lv_nanovg_vg_context *priv = (lv_nanovg_vg_context*)ctx->priv;
    nvgBeginPath(priv->vg);
}

static void lv_nanovg_vg_close_path(lv_context * ctx)
{
    lv_debug("trace: lv_nanovg_vg_close_path\n");
    lv_nanovg_vg_context *priv = (lv_nanovg_vg_context*)ctx->priv;
    nvgClosePath(priv->vg);
}

static void lv_nanovg_vg_path_winding(lv_context * ctx, int dir)
{
    lv_debug("trace: lv_nanovg_vg_path_winding: %d\n", dir);
    lv_nanovg_vg_context *priv = (lv_nanovg_vg_context*)ctx->priv;
    nvgPathWinding(priv->vg, dir);
}

static void lv_nanovg_vg_miter_limit(lv_context * ctx, float limit)
{
    lv_debug("trace: lv_nanovg_vg_miter_limit: %f\n", limit);
    lv_nanovg_vg_context *priv = (lv_nanovg_vg_context*)ctx->priv;
    nvgMiterLimit(priv->vg, limit);
}

static void lv_nanovg_vg_line_cap(lv_context * ctx, int cap)
{
    lv_debug("trace: lv_nanovg_vg_line_cap: %d\n", cap);
    lv_nanovg_vg_context *priv = (lv_nanovg_vg_context*)ctx->priv;
    nvgLineCap(priv->vg, cap);
}

static void lv_nanovg_vg_line_join(lv_context * ctx, int join)
{
    lv_debug("trace: lv_nanovg_vg_line_join: %d\n", join);
    lv_nanovg_vg_context *priv = (lv_nanovg_vg_context*)ctx->priv;
    nvgLineJoin(priv->vg, join);
}

static void lv_nanovg_vg_fill_color(lv_context * ctx, lv_color c)
{
    lv_debug("trace: lv_nanovg_vg_fill_color\n");
    lv_nanovg_vg_context *priv = (lv_nanovg_vg_context*)ctx->priv;
    NVGcolor vgc;
    memcpy(&vgc, &c, sizeof(vgc));
    nvgFillColor(priv->vg, vgc);
}

static void lv_nanovg_vg_fill_paint(lv_context * ctx, lv_paint p)
{
    lv_debug("trace: lv_nanovg_vg_fill_paint\n");
    lv_nanovg_vg_context *priv = (lv_nanovg_vg_context*)ctx->priv;
    NVGpaint vgp;
    memcpy(&vgp, &p, sizeof(vgp));
    nvgFillPaint(priv->vg, vgp);
}

static void lv_nanovg_vg_fill(lv_context * ctx)
{
    lv_debug("trace: lv_nanovg_vg_fill\n");
    lv_nanovg_vg_context *priv = (lv_nanovg_vg_context*)ctx->priv;
    nvgFill(priv->vg);
}

static void lv_nanovg_vg_stroke_color(lv_context * ctx, lv_color c)
{
    lv_debug("trace: lv_nanovg_vg_stroke_color\n");
    lv_nanovg_vg_context *priv = (lv_nanovg_vg_context*)ctx->priv;
    NVGcolor vgc;
    memcpy(&vgc, &c, sizeof(vgc));
    nvgStrokeColor(priv->vg, vgc);
}

static void lv_nanovg_vg_stroke_paint(lv_context * ctx, lv_paint p)
{
    lv_debug("trace: lv_nanovg_vg_stroke_paint\n");
    lv_nanovg_vg_context *priv = (lv_nanovg_vg_context*)ctx->priv;
    NVGpaint vgp;
    memcpy(&vgp, &p, sizeof(vgp));
    nvgStrokePaint(priv->vg, vgp);
}


static void lv_nanovg_vg_stroke_width(lv_context * ctx, float w)
{
    lv_debug("trace: lv_nanovg_vg_stroke_width\n");
    lv_nanovg_vg_context *priv = (lv_nanovg_vg_context*)ctx->priv;
    nvgStrokeWidth(priv->vg, w);
}

static void lv_nanovg_vg_stroke(lv_context * ctx)
{
    lv_debug("trace: lv_nanovg_vg_stroke\n");
    lv_nanovg_vg_context *priv = (lv_nanovg_vg_context*)ctx->priv;
    nvgStroke(priv->vg);
}

static void lv_nanovg_vg_2d_translate(lv_context * ctx, vec2f v0)
{
    lv_debug("trace: lv_nanovg_vg_2d_translate: %f %f\n", v0.x, v0.y);
    lv_nanovg_vg_context *priv = (lv_nanovg_vg_context*)ctx->priv;
    nvgTranslate(priv->vg, v0.x, v0.y);
}

static void lv_nanovg_vg_2d_rotate(lv_context * ctx, float a0)
{
    lv_debug("trace: lv_nanovg_vg_2d_rotate: %f\n", a0);
    lv_nanovg_vg_context *priv = (lv_nanovg_vg_context*)ctx->priv;
    nvgRotate(priv->vg, a0);
}

static void lv_nanovg_vg_2d_skew_x(lv_context * ctx, float a0)
{
    lv_debug("trace: lv_nanovg_vg_2d_skew_x: %f\n", a0);
    lv_nanovg_vg_context *priv = (lv_nanovg_vg_context*)ctx->priv;
    nvgSkewX(priv->vg, a0);
}

static void lv_nanovg_vg_2d_skew_y(lv_context * ctx, float a0)
{
    lv_debug("trace: lv_nanovg_vg_2d_skew_x: %f\n", a0);
    lv_nanovg_vg_context *priv = (lv_nanovg_vg_context*)ctx->priv;
    nvgSkewY(priv->vg, a0);
}

static void lv_nanovg_vg_2d_scale(lv_context * ctx, vec2f v0)
{
    lv_debug("trace: lv_nanovg_vg_2d_scale: %f %f\n", v0.x, v0.y);
    lv_nanovg_vg_context *priv = (lv_nanovg_vg_context*)ctx->priv;
    nvgScale(priv->vg, v0.x, v0.y);
}

static void lv_nanovg_vg_2d_move_to(lv_context * ctx, vec2f p0)
{
    lv_debug("trace: lv_nanovg_vg_move_to: %f %f\n", p0.x, p0.y);
    lv_nanovg_vg_context *priv = (lv_nanovg_vg_context*)ctx->priv;
    nvgMoveTo(priv->vg, p0.x, p0.y);
}

static void lv_nanovg_vg_2d_line_to(lv_context * ctx, vec2f p0)
{
    lv_debug("trace: lv_nanovg_vg_line_to: %f, %f\n", p0.x, p0.y);
    lv_nanovg_vg_context *priv = (lv_nanovg_vg_context*)ctx->priv;
    nvgLineTo(priv->vg, p0.x, p0.y);
}

static void lv_nanovg_vg_2d_quadratic_to(lv_context * ctx, vec2f c0, vec2f p0)
{
    lv_debug("trace: lv_nanovg_vg_quadratic_to: %f, %f, %f, %f\n",
        c0.x, c0.y, p0.x, p0.y);
    lv_nanovg_vg_context *priv = (lv_nanovg_vg_context*)ctx->priv;
    nvgQuadTo(priv->vg, c0.x, c0.y, p0.x, p0.y);
}

static void lv_nanovg_vg_2d_bezier_to(lv_context * ctx, vec2f c0, vec2f c1, vec2f p0)
{
    lv_debug("trace: lv_nanovg_vg_bezier_to: %f, %f, %f, %f, %f, %f\n",
        c0.x, c0.y, c1.x, c1.y, p0.x, p0.y);
    lv_nanovg_vg_context *priv = (lv_nanovg_vg_context*)ctx->priv;
    nvgBezierTo(priv->vg, c0.x, c0.y, c1.x, c1.y, p0.x, p0.y);
}

static void lv_nanovg_vg_2d_arc_to(lv_context* ctx, vec2f p0, vec2f p1, float a0)
{
    lv_debug("trace: lv_nanovg_vg_2d_arc_to: %f, %f, %f, %f, %f\n",
        p0.x, p0.y, p1.x, p1.y, a0);
    lv_nanovg_vg_context *priv = (lv_nanovg_vg_context*)ctx->priv;
    nvgArcTo(priv->vg, p0.x, p0.y, p1.x, p1.y, a0);
}

static void lv_nanovg_vg_2d_arc(lv_context * ctx, vec2f c0, float r, vec2f a0, int dir)
{
    lv_debug("trace: lv_nanovg_vg_2d_arc: %f, %f, %f, %f, %f, %d\n",
        c0.x, c0.y, r, a0.x, a0.y, dir);
    lv_nanovg_vg_context *priv = (lv_nanovg_vg_context*)ctx->priv;
    nvgArc(priv->vg, c0.x, c0.y, r, a0.x, a0.y, dir);
}

static void lv_nanovg_vg_2d_rect(lv_context* ctx, vec2f p0, vec2f s0)
{
    lv_debug("trace: lv_nanovg_vg_2d_rect: %f, %f, %f, %f\n",
        p0.x, p0.y, s0.x, s0.y);
    lv_nanovg_vg_context *priv = (lv_nanovg_vg_context*)ctx->priv;
    nvgRect(priv->vg, p0.x, p0.y, s0.x, s0.y);
}

static void lv_nanovg_vg_2d_rounded_rect(lv_context* ctx, vec2f p0, vec2f s0, float r)
{
    lv_debug("trace: lv_nanovg_vg_2d_rounded_rect: %f, %f, %f, %f, %f\n",
        p0.x, p0.y, s0.x, s0.y, r);
    lv_nanovg_vg_context *priv = (lv_nanovg_vg_context*)ctx->priv;
    nvgRoundedRect(priv->vg, p0.x, p0.y, s0.x, s0.y, r);
}

static void lv_nanovg_vg_2d_ellipse(lv_context * ctx, vec2f c0, vec2f r0)
{
    lv_debug("trace: lv_nanovg_vg_2d_ellipse: %f, %f, %f, %f\n",
        c0.x, c0.y, r0.x, r0.y);
    lv_nanovg_vg_context *priv = (lv_nanovg_vg_context*)ctx->priv;
    nvgEllipse(priv->vg, c0.x, c0.y, r0.x, r0.y);
}

static void lv_nanovg_vg_2d_circle(lv_context * ctx, vec2f c0, float r)
{
    lv_debug("trace: lv_nanovg_vg_2d_circle: %f, %f, %f\n",
        c0.x, c0.y, r);
    lv_nanovg_vg_context *priv = (lv_nanovg_vg_context*)ctx->priv;
    nvgCircle(priv->vg, c0.x, c0.y, r);
}

static void lv_nanovg_vg_2d_text_font(lv_context * ctx, const char *font)
{
    lv_debug("trace: lv_nanovg_vg_2d_text_font: %s\n", font);
    // todo
}

static void lv_nanovg_vg_2d_text_size(lv_context * ctx, float size)
{
    lv_debug("trace: lv_nanovg_vg_2d_text_size: %f\n", size);
    // todo
}

static void lv_nanovg_vg_2d_text_leading(lv_context * ctx, float leading)
{
    lv_debug("trace: lv_nanovg_vg_2d_text_leading: %f\n", leading);
    // todo
}

static void lv_nanovg_vg_2d_text_tracking(lv_context * ctx, float tracking)
{
    lv_debug("trace: lv_nanovg_vg_2d_text_tracking: %f\n", tracking);
    // todo
}

static void lv_nanovg_vg_2d_text_blur(lv_context * ctx, float blur)
{
    lv_debug("trace: lv_nanovg_vg_2d_text_blur: %f\n", blur);
    // todo
}

static void lv_nanovg_vg_2d_text_align(lv_context * ctx, int align)
{
    lv_debug("trace: lv_nanovg_vg_2d_text_align: %d\n", align);
    // todo
}

static vec2f lv_nanovg_vg_2d_text_bounds(lv_context * ctx, const char *text)
{
    lv_debug("trace: lv_nanovg_vg_2d_text_bounds: %s\n", text);
    // todo
    vec2f r = { 0, 0 };
    return r;
}

static void lv_nanovg_vg_2d_text_draw(lv_context * ctx, vec2f v0, const char *text)
{
    lv_debug("trace: lv_nanovg_vg_2d_text_draw: %f %f %s\n", v0.x, v0.y, text);
    // todo
}

static void lv_nanovg_vg_3d_transform(lv_context * ctx, mat4x4 m0)
{
    lv_debug("trace: lv_nanovg_vg_3d_transform: ( %f, %f, %f, %f ), "
        "( %f, %f, %f, %f ), ( %f, %f, %f, %f ), ( %f, %f, %f, %f )\n",
        m0[0][0], m0[0][1], m0[0][2], m0[0][3],
        m0[1][0], m0[1][1], m0[1][2], m0[1][3],
        m0[2][0], m0[2][1], m0[2][2], m0[2][3],
        m0[3][0], m0[3][1], m0[3][2], m0[3][3]);
    abort();
}

static void lv_nanovg_vg_3d_translate(lv_context * ctx, vec3f v0)
{
    lv_debug("trace: lv_nanovg_vg_3d_translate: ( %f, %f, %f )\n",
        v0.x, v0.y, v0.z);
    abort();
}

static void lv_nanovg_vg_3d_rotate(lv_context * ctx, vec3f v0, float a0)
{
    lv_debug("trace: lv_nanovg_vg_3d_rotate: ( %f, %f, %f ), %f\n",
        v0.x, v0.y, v0.z, a0);
    abort();
}

static void lv_nanovg_vg_3d_scale(lv_context * ctx, vec3f v0)
{
    lv_debug("trace: lv_nanovg_vg_3d_scale: ( %f, %f, %f )\n",
        v0.x, v0.y, v0.z);
    abort();
}

static void lv_nanovg_vg_3d_move_to(lv_context * ctx, vec3f p0)
{
    lv_debug("trace: lv_nanovg_vg_3d_move_to: ( %f, %f, %f )\n",
        p0.x, p0.y, p0.z);
    abort();
}

static void lv_nanovg_vg_3d_line_to(lv_context * ctx, vec3f p0)
{
    lv_debug("trace: lv_nanovg_vg_3d_line_to: ( %f, %f, %f )\n",
        p0.x, p0.y, p0.z);
    abort();
}

static void lv_nanovg_vg_3d_quadratic_to(lv_context * ctx, vec3f c0, vec3f p0)
{
    lv_debug("trace: lv_nanovg_vg_3d_quadratic_to: ( %f, %f, %f ), ( %f, %f, %f )\n",
        c0.x, c0.y, c0.z, p0.x, p0.y, p0.z);
    abort();
}

static void lv_nanovg_vg_3d_bezier_to(lv_context * ctx, vec3f c0, vec3f c1, vec3f p0)
{
    lv_debug("trace: lv_nanovg_vg_3d_bezier_to: ( %f, %f, %f ), ( %f, %f, %f ), "
        "( %f, %f, %f )\n",
        c0.x, c0.y, c0.z, c1.x, c1.y, c1.z, p0.x, p0.y, p0.z);
    abort();
}

static const lv_vg_ops lv_nanovg_vg_ops =
{
    lv_nanovg_vg_init,
    lv_nanovg_vg_destroy,

    lv_nanovg_vg_begin_frame,
    lv_nanovg_vg_end_frame,
    lv_nanovg_vg_push,
    lv_nanovg_vg_pop,
    lv_nanovg_vg_reset,
    lv_nanovg_vg_begin_path,
    lv_nanovg_vg_close_path,
    lv_nanovg_vg_path_winding,
    lv_nanovg_vg_miter_limit,
    lv_nanovg_vg_line_cap,
    lv_nanovg_vg_line_join,
    lv_nanovg_vg_fill_color,
    lv_nanovg_vg_fill_paint,
    lv_nanovg_vg_fill,
    lv_nanovg_vg_stroke_color,
    lv_nanovg_vg_stroke_paint,
    lv_nanovg_vg_stroke_width,
    lv_nanovg_vg_stroke,

    lv_nanovg_vg_2d_translate,
    lv_nanovg_vg_2d_rotate,
    lv_nanovg_vg_2d_skew_x,
    lv_nanovg_vg_2d_skew_y,
    lv_nanovg_vg_2d_scale,
    lv_nanovg_vg_2d_move_to,
    lv_nanovg_vg_2d_line_to,
    lv_nanovg_vg_2d_quadratic_to,
    lv_nanovg_vg_2d_bezier_to,
    lv_nanovg_vg_2d_arc_to,
    lv_nanovg_vg_2d_arc,
    lv_nanovg_vg_2d_rect,
    lv_nanovg_vg_2d_rounded_rect,
    lv_nanovg_vg_2d_ellipse,
    lv_nanovg_vg_2d_circle,

    lv_nanovg_vg_2d_text_font,
    lv_nanovg_vg_2d_text_size,
    lv_nanovg_vg_2d_text_leading,
    lv_nanovg_vg_2d_text_tracking,
    lv_nanovg_vg_2d_text_blur,
    lv_nanovg_vg_2d_text_align,
    lv_nanovg_vg_2d_text_bounds,
    lv_nanovg_vg_2d_text_draw,

    lv_nanovg_vg_3d_transform,
    lv_nanovg_vg_3d_translate,
    lv_nanovg_vg_3d_rotate,
    lv_nanovg_vg_3d_scale,
    lv_nanovg_vg_3d_move_to,
    lv_nanovg_vg_3d_line_to,
    lv_nanovg_vg_3d_quadratic_to,
    lv_nanovg_vg_3d_bezier_to,
};

#ifdef __cplusplus
}
#endif
