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

#include <stddef.h>

#include "lv_color.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct lv_oid lv_oid;
typedef struct lv_oid_idx lv_oid_idx;
typedef struct lv_sign lv_sign;

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

struct lv_oid_idx
{
    size_t idx;
    float pos[3];
};

struct lv_sign
{
    const char *symbol;
    const char *name;
    lv_color color;
};

extern lv_oid data[];
extern lv_sign signs[];
extern size_t data_count;
extern size_t sign_count;

#ifdef __cplusplus
}
#endif
