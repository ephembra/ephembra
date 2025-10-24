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

struct lv_color {
        union {
                float rgba[4];
                float hsva[4];
                struct { float r,g,b,a; };
                struct { float h,s,v; };
        };
};
typedef struct lv_color lv_color;

static lv_color lv_rgbf(float r, float g, float b)
{
    lv_color c = { r, g, b, 1.0f }; return c;
}

static lv_color lv_rgbaf(float r, float g, float b, float a)
{
    lv_color c = { r, g, b, a }; return c;
}

static lv_color lv_rgbf_array(float col[3])
{
    lv_color c = { col[0], col[1], col[2], 1.0f }; return c;
}

static lv_color lv_rgbaf_array(float col[4])
{
    lv_color c = { col[0], col[1], col[2], col[3] }; return c;
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

lv_color lv_rgb_to_hsv(lv_color c);

lv_color lv_hsv_to_rgb(lv_color c);

lv_color lv_color_adjust(lv_color c, float t_bright, float t_saturate);

#ifdef __cplusplus
}
#endif
