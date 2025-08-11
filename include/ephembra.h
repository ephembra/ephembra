/*
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

typedef struct ephem_ctx ephem_ctx;

struct ephem_ctx
{
    size_t rows;
    size_t cols;
    double *PC;
};

enum {
    ephem_id_Sun = 0,
    ephem_id_Mercury = 1,
    ephem_id_Venus = 2,
    ephem_id_EarthMoon = 3,
    ephem_id_Mars = 4,
    ephem_id_Jupiter = 5,
    ephem_id_Saturn = 6,
    ephem_id_Uranus = 7,
    ephem_id_Neptune = 8,
    ephem_id_Pluto = 9,
    ephem_id_Moon = 10,
    ephem_id_Nutations = 11,
    ephem_id_Librations = 12,
    ephem_id_Last = 13
};

void de440_create_ephem(ephem_ctx *ctx, const char *ephem_bin);
void de440_destroy_ephem(ephem_ctx *ctx);
size_t de440_find_row(ephem_ctx *ctx, double jd);
void de440_ephem_obj(ephem_ctx *ctx, double jd, size_t row, size_t oid,
    double *obj);
const char* de440_object_name(size_t oid);

#ifdef __cplusplus
}
#endif
