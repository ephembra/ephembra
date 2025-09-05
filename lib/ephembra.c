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

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <math.h>

#include "ephembra.h"

#define VA_ARGS(...) , ##__VA_ARGS__
#define ephem_error(fmt, ...) \
    fprintf(stderr, fmt "\n" VA_ARGS(__VA_ARGS__)); exit(1);

typedef struct de440_dim de440_dim;
typedef struct de440_idx de440_idx;

struct de440_dim
{
    size_t dim[3];
};

struct de440_idx
{
    size_t start;
    size_t addend;
    size_t end;
    size_t step;
    size_t offset;
};

const char* ephem_name[15] = {
    [ephem_id_Mercury_Barycenter]      = "Mercury Barycenter",
    [ephem_id_Venus_Barycenter]        = "Venus Barycenter",
    [ephem_id_Earth_Moon_Barycenter]   = "Earth Moon Barycenter",
    [ephem_id_Mars_Barycenter]         = "Mars Barycenter",
    [ephem_id_Jupiter_Barycenter]      = "Jupiter Barycenter",
    [ephem_id_Saturn_Barycenter]       = "Saturn Barycenter",
    [ephem_id_Uranus_Barycenter]       = "Uranus Barycenter",
    [ephem_id_Neptune_Barycenter]      = "Neuptune Barycenter",
    [ephem_id_Pluto_Barycenter]        = "Pluto Barycenter",
    [ephem_id_Sun]                     = "Sun",
    [ephem_id_Moon]                    = "Moon",
    [ephem_id_Earth]                   = "Earth",
    [ephem_id_Nutations]               = "Nutations",
    [ephem_id_Librations]              = "Librations"
};

const de440_idx ephem_idx[15] = {
    [ephem_id_Mercury_Barycenter]      = {   3, 14,  45,  8, 42 },
    [ephem_id_Venus_Barycenter]        = { 171, 10, 201, 16, 30 },
    [ephem_id_Earth_Moon_Barycenter]   = { 231, 13, 270, 16, 39 },
    [ephem_id_Mars_Barycenter]         = { 309, 11, 342, 32,  0 },
    [ephem_id_Jupiter_Barycenter]      = { 342,  8, 366, 32,  0 },
    [ephem_id_Saturn_Barycenter]       = { 366,  7, 387, 32,  0 },
    [ephem_id_Uranus_Barycenter]       = { 387,  6, 405, 32,  0 },
    [ephem_id_Neptune_Barycenter]      = { 405,  6, 423, 32,  0 },
    [ephem_id_Pluto_Barycenter]        = { 423,  6, 441, 32,  0 },
    [ephem_id_Sun]                     = { 753, 11, 786, 16, 33 },
    [ephem_id_Moon]                    = { 441, 13, 480,  4, 39 },
    [ephem_id_Nutations]               = { 819, 10, 839,  8, 20 },
    [ephem_id_Librations]              = { 899, 10, 929,  8, 30 }
};

void de440_create_ephem(ephem_ctx *ctx, const char *ephem_bin)
{
    FILE *f;
    size_t dsize, nbytes;

    f = fopen(ephem_bin, "r");
    if (!f) {
        ephem_error("fopen: failed: %s", ephem_bin);        
    }
    nbytes = fread(&ctx->rows, 1, sizeof(size_t), f);
    if (nbytes != sizeof(size_t)) {
        ephem_error("fread: invalid size: %zu != %zu", nbytes, sizeof(size_t));
    }
    nbytes = fread(&ctx->cols, 1, sizeof(size_t), f);
    if (nbytes != sizeof(size_t)) {
        ephem_error("fread: invalid size: %zu != %zu", nbytes, sizeof(size_t));
    }
    dsize = ctx->rows * ctx->cols * sizeof(double);
    ctx->PC = malloc(dsize);
    if (!ctx->PC) {
        ephem_error("malloc: failed to allocate %zu bytes", dsize);
    }
    nbytes = fread(ctx->PC, 1, dsize, f);
    if (nbytes != dsize) {
        ephem_error("fread: invalid size: %zu != %zu", nbytes, dsize);
    }

    fclose(f);
}

void de440_destroy_ephem(ephem_ctx *ctx)
{
    free(ctx->PC);
}

static void de440_cheb3d(double jd, size_t n, double jd0, double jd1,
    const double* Cx, const double* Cy, const double* Cz,
    double *r, float scale)
{
    double tau = 2*(jd - jd0)/(jd1 - jd0) - 1;

    double Tx_prev = 1.0;
    double Tx_curr = tau;
    double sum_x = Cx[0] * Tx_prev + Cx[1] * Tx_curr;

    double Ty_prev = 1.0;
    double Ty_curr = tau;
    double sum_y = Cy[0] * Ty_prev + Cy[1] * Ty_curr;

    double Tz_prev = 1.0;
    double Tz_curr = tau;
    double sum_z = Cz[0] * Tz_prev + Cz[1] * Tz_curr;

    for (size_t k = 2; k < n; ++k) {
        double Tx_next = 2 * tau * Tx_curr - Tx_prev;
        sum_x += Cx[k] * Tx_next;
        Tx_prev = Tx_curr;
        Tx_curr = Tx_next;

        double Ty_next = 2 * tau * Ty_curr - Ty_prev;
        sum_y += Cy[k] * Ty_next;
        Ty_prev = Ty_curr;
        Ty_curr = Ty_next;

        double Tz_next = 2 * tau * Tz_curr - Tz_prev;
        sum_z += Cz[k] * Tz_next;
        Tz_prev = Tz_curr;
        Tz_curr = Tz_next;
    }

    r[0] = sum_x * scale;
    r[1] = sum_y * scale;
    r[2] = sum_z * scale;
}

static de440_dim de440_index(size_t start, size_t addend, size_t end)
{
    de440_dim x;
    for (size_t i = 0; i < 3; i++) {
        x.dim[i] = start - 1 + (addend * i);
    }
    return x;
}

static de440_dim de440_add(de440_dim x, size_t a)
{
    for (size_t i = 0; i < 3; i++) {
        x.dim[i] += a;
    }
    return x;
}

static size_t de440_interval(double dt, size_t step, size_t interval)
{
    for (size_t i = 0; i < interval; i += step) {
        if (i <= dt && dt <= i + step) {
            return i / step;
        }
    }
    return interval / step;
}

static void de440_ephem_body(ephem_ctx *ctx, double jd, size_t row,
    size_t start, size_t addend, size_t end,
    size_t step, size_t offset, double *r)
{
    de440_dim temp;
    double *PC, t1, dt, jd0, *Cx, *Cy, *Cz, s = 1e3;
    size_t i;

    PC = ctx->PC + ctx->cols * row;

    t1 = PC[0];
    dt = jd - t1;
    i = de440_interval(dt, step, 32);
    temp = de440_index(start, addend, end);
    temp = de440_add(temp, offset * i);
    jd0 = t1 + step * i;
    Cx = PC + temp.dim[0];
    Cy = PC + temp.dim[1];
    Cz = PC + temp.dim[2];

    de440_cheb3d(jd, addend, jd0, jd0 + step, Cx, Cy, Cz, r, s);    
}

static inline int de440_cmp(ephem_ctx *ctx, double jd, size_t row)
{
    size_t c = ctx->cols;
    double *d = ctx->PC + c * row;
    double jd1 = d[0], jd2 = d[1];
    if (jd < jd1) return -1;
    else if (jd > jd2) return 1;
    else return 0;
}

size_t de440_find_row(ephem_ctx *ctx, double jd)
{
    size_t n = ctx->rows;
    size_t begin = 0, end = n;
    while (end != 0) {
        size_t half = (end >> 1), probe = begin + half;
        if (de440_cmp(ctx, jd, probe) > 0) {
            begin = probe + 1;
            end -= half + 1;
        } else {
            end = half;
        }
    }
    return de440_cmp(ctx, jd, begin) == 0 ? begin : -1;
}

static void de440_ephem_obj_internal(ephem_ctx *ctx, double jd,
    size_t row, size_t oid, double *obj)
{
    if (row == -1) {
        obj[0] = NAN; obj[1] = NAN; obj[2] = NAN;
    } else {
        de440_ephem_body(ctx, jd, row, ephem_idx[oid].start,
            ephem_idx[oid].addend, ephem_idx[oid].end,
            ephem_idx[oid].step, ephem_idx[oid].offset, obj);
    }
}

#define EMRAT 81.3005682214972154
#define EMRAT1 (1.0/(1.0+EMRAT))

void de440_ephem_obj(ephem_ctx *ctx, double jd,
    size_t row, size_t oid, double *obj)
{
    double t0[3], t1[3];
    switch (oid) {
    case ephem_id_Solar_System_Barycenter:
        obj[0] = 0.0;
        obj[1] = 0.0;
        obj[2] = 0.0;
        break;
    case ephem_id_Earth:
        de440_ephem_obj_internal(ctx, jd, row, ephem_id_Earth_Moon_Barycenter, t0);
        de440_ephem_obj_internal(ctx, jd, row, ephem_id_Moon, t1);
        obj[0] = t0[0] - t1[0] * EMRAT1;
        obj[1] = t0[1] - t1[1] * EMRAT1;
        obj[2] = t0[2] - t1[2] * EMRAT1;
        break;
    case ephem_id_Moon:
        de440_ephem_obj_internal(ctx, jd, row, ephem_id_Earth_Moon_Barycenter, t0);
        de440_ephem_obj_internal(ctx, jd, row, ephem_id_Moon, t1);
        obj[0] = t0[0] + t1[0];
        obj[1] = t0[1] + t1[1];
        obj[2] = t0[2] + t1[2];
        break;
    default:
        de440_ephem_obj_internal(ctx, jd, row, oid, obj);
        break;
    }
}

const char* de440_object_name(size_t oid)
{
    return ephem_name[oid];
}