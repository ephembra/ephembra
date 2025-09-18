/*
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

#include "linmath.h"

#include "lv_screen.h"

void object_to_screen(vec3 r, vec3 p, mat4x4 matrix, int w, int h)
{
    vec4 u = { p[0], p[1], p[2], 1.0f };
    vec4 q;

    mat4x4_mul_vec4(q, matrix, u);

    q[0] /= q[3];
    q[1] /= q[3];
    q[2] /= q[3];

    r[0] = (q[0] * 0.5f + 0.5f) * w;
    r[1] = (q[1] * 0.5f + 0.5f) * h;
    r[2] = q[2];
}

void screen_to_object(vec3 r, vec3 p, mat4x4 invmatrix, int w, int h)
{
    vec4 u = {
        (p[0] / w) * 2.0f - 1.0f, (p[1] / h) * 2.0f - 1.0f, p[2], 1.0f
    };
    vec4 q;

    mat4x4_mul_vec4(q, invmatrix, u);

    r[0] = q[0] / q[3];
    r[1] = q[1] / q[3];
    r[2] = q[2] / q[3];
}
