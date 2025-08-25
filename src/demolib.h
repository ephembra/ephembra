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

#include <math.h>
#include <float.h>

#include "linmath.h"
#include "gl2_util.h"
#include "lv_model.h"

#define countof(arr) (sizeof(arr)/sizeof(arr[0]))

typedef struct lv_date lv_date;

struct lv_date
{
    int year, month, day;
};

/*
 * math functions
 */

static inline float deg_rad(float a) { return a * M_PI / 180.0f; }

static inline float rad_deg(float a) { return a * 180.0f / M_PI; }

static inline float vector_angle_deg(float x, float y)
{
    float a = atan2f(y, x) * 180.0f / M_PI;
    return (a < 0) ? a + 360.0f : a;
}

static inline float clampf(float x, float min_val, float max_val)
{
    return fminf(fmaxf(x, min_val), max_val);
}

static inline float mod_2pi(float x)
{
    float m = fmodf(x, 2.0f * M_PI);
    return (m < 0.0f) ? m + 2.0f * M_PI : m;
}

/*
 * point within circle angular map
 *
 * map angles from a point (x0, y0) inside of a circle of radius 'r'
 * relative to origin (r, r) to points on the circle circumference.
 *
 *  • θ (theta): angle relative to circle center [0, 2π).
 *  • φ (phi): angle from (x0,y0) toward boundary [0, 2π).
 *
 * domain: |(x0,y0)| < r, r > 0, results normalized to [0, 2π)
 *
 * forward: θ → φ:   phi = atan2(r*sinθ - y0, r*cosθ - x0)
 * inverse: φ → θ:   theta = phi + arcsin((y0*cosφ - x0*sinφ)/r)
 */

static inline float angle_circle_to_point(float a, float x0, float y0, float r)
{
    return atan2f(r * sinf(a) - y0, r * cosf(a) - x0);
}

static inline float angle_point_to_circle(float a, float x0, float y0, float r)
{
    return (a + asinf((y0 * cosf(a) - x0 * sinf(a)) / r));
}

/*
 * vector functions
 */

static inline void vec4_vec3_w1(vec4 r, vec3 p)
{
    r[0] = p[0];
    r[1] = p[1];
    r[2] = p[2];
    r[3] = 1.0f;
}

static inline void vec3_multiply_add(vec3 r, vec3 b, float s, vec3 p)
{
    r[0] = b[0] * s + p[0];
    r[1] = b[1] * s + p[1];
    r[2] = b[2] * s + p[2];
}

static inline void vec3_sincos_basis(vec3 r, float theta,
    vec3 x0, vec3 y0, vec3 z0, float f, float g)
{
    float ct = cosf(theta) * g, st = sinf(theta) * g;
    r[0] = z0[0] * f + x0[0] * ct + y0[0] * st;
    r[1] = z0[1] * f + x0[1] * ct + y0[1] * st;
    r[2] = z0[2] * f + x0[2] * ct + y0[2] * st;
}

static inline float vec2_dist_point_line(vec2 p, vec2 a, vec2 b)
{
    float dx = b[0] - a[0];
    float dy = b[1] - a[1];
    float l2 = dx * dx + dy * dy;
    float k2 = (p[0] - a[0]) * dx + (p[1] - a[1]) * dy;
    float t = clampf(k2 / (l2 + FLT_EPSILON), 0.0, 1.0);
    float cx = a[0] + t * dx;
    float cy = a[1] + t * dy;
    float dcx = p[0] - cx;
    float dcy = p[1] - cy;
    return sqrtf(dcx * dcx + dcy * dcy);
}

/* project point p0 onto plane defined by orthonormal basis in x0, y0 */
static inline void vec3_project_to_basis(vec3 r, vec3 p0, vec3 x0, vec3 y0)
{
    vec3 x1, y1;
    float cx = vec3_mul_inner(p0,x0) / vec3_mul_inner(x0,x0);
    float cy = vec3_mul_inner(p0,y0) / vec3_mul_inner(y0,y0);
    vec3_scale(x1, x0, cx);
    vec3_scale(y1, y0, cy);
    vec3_add(r, x1, y1);
}

/*
 * object space screen space projection
 */

void object_to_screen(vec3 r, vec3 p, mat4x4 matrix, int w, int h);

void screen_to_object(vec3 r, vec3 p, mat4x4 invmatrix, int w, int h);

/*
 * date functions
 */

double lv_date_to_julian(lv_date d);

lv_date lv_julian_to_date(double jd);

int lv_days_in_month(int year, int month);

/*
 * astronomical functions
 */

/* Convert arcseconds to radians */
#define ASEC2RAD (M_PI / (180.0 * 3600.0))

/* create precession rotation matrix (IAU 2006) for julian date */
void lv_iau2006_precession_matrix(mat4x4 R, double jd);

/* mean obliquity of the ecliptic at J2000 (in radians) */
#define EPS0_MEAN_OBLIQ_J2000 (84381.406 * ASEC2RAD)

/* mean obliquity of date IAU 2006/2000A series */
double lv_iau2006_obliquity_eps(double jd);

void lv_iau2006_obliquity_matrix(mat4x4 R, double jd);

void lv_iau2006_obliquity_basis(vec3 x0, vec3 y0, vec3 z0, double jd);

void lv_iau2006_combined_matrix(mat4x4 R, double jd);

void lv_iau2006_combined_basis(vec3 x0, vec3 y0, vec3 z0, double jd);

/*
 * color functions
 */

lv_color lv_rgb_to_hsv(lv_color c);

lv_color lv_hsv_to_rgb(lv_color c);

lv_color lv_color_adjust(lv_color c, float t_bright, float t_saturate);

#ifdef __cplusplus
}
#endif
