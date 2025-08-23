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
    return mod_2pi(atan2f(r * sinf(a) - y0, r * cosf(a) - x0));
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

static void object_to_screen(vec3 r, vec3 p, mat4x4 matrix, int w, int h)
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

static void screen_to_object(vec3 r, vec3 p, mat4x4 invmatrix, int w, int h)
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

/*
 * date functions
 */

static double lv_date_to_julian(lv_date d)
{
    int Y = d.year;
    int M = d.month;
    int D = d.day;

    if (M <= 2) {
        Y -= 1;
        M += 12;
    }

    int A = Y / 100;
    int B = 2 - A + (A / 4);

    double JD = floor(365.25 * (Y + 4716))
              + floor(30.6001 * (M + 1))
              + D + B - 1524.5;

    return fabs(JD);
}

static lv_date lv_julian_to_date(double jd)
{
    jd = fabs(jd + 0.5);

    long Z = (long)floor(jd);
    double F = jd - Z;

    long alpha = (long)((Z - 1867216.25) / 36524.25);
    long A = Z + 1 + alpha - alpha / 4;

    long B = A + 1524;
    long C = (long)((B - 122.1) / 365.25);
    long D = (long)(365.25 * C);
    long E = (long)((B - D) / 30.6001);

    double day = B - D - (long)(30.6001 * E) + F;
    int month = (E < 14) ? E - 1 : E - 13;
    int year  = (month > 2) ? C - 4716 : C - 4715;

    lv_date d = {
        year,
        month,
        (int)floor(day),
    };

    return d;
}

static int lv_days_in_month(int year, int month)
{
    static const int dim[12] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
    if (month == 1 && (year%4==0 && (year%100!=0 || year%400==0))) {
        return 29;
    }
    return dim[month];
}

/*
 * astronomical functions
 */

/* Convert arcseconds to radians */
#define ASEC2RAD (M_PI / (180.0 * 3600.0))

/* create precession rotation matrix (IAU 2006) for julian date */
static void lv_iau2006_precession_matrix(mat4x4 R, double jd)
{
    double T = (jd - 2451545.0) / 36525.0;

    /* Precession angles in arcseconds */
    double zeta_A = (2306.083227*T + 0.2988499*T*T + 0.01801828*T*T*T
        - 0.000005971*T*T*T*T - 0.0000003173*T*T*T*T*T) * ASEC2RAD;

    double theta_A = (2004.191903*T - 0.4294934*T*T - 0.04182264*T*T*T
        - 0.000007089*T*T*T*T - 0.0000001274*T*T*T*T*T) * ASEC2RAD;

    double z_A = (2306.077181*T + 1.0927348*T*T + 0.01826837*T*T*T
        - 0.000028596*T*T*T*T - 0.0000002904*T*T*T*T*T) * ASEC2RAD;

    mat4x4 Rz1, Ry, Rz2, Rtmp;

    mat4x4_identity(Rz1);
    mat4x4_identity(Ry);
    mat4x4_identity(Rz2);

    /* Rz(-zeta_A) */
    Rz1[0][0] = cos(-zeta_A); Rz1[0][1] = -sin(-zeta_A);
    Rz1[1][0] = sin(-zeta_A); Rz1[1][1] =  cos(-zeta_A);

    /* Ry(theta_A) */
    Ry[0][0] =  cos(theta_A); Ry[0][2] = sin(theta_A);
    Ry[2][0] = -sin(theta_A); Ry[2][2] = cos(theta_A);

    /* Rz(-z_A) */
    Rz2[0][0] = cos(-z_A); Rz2[0][1] = -sin(-z_A);
    Rz2[1][0] = sin(-z_A); Rz2[1][1] =  cos(-z_A);

    /* Multiply: R = Rz2 * Ry * Rz1 */
    mat4x4_mul(Rtmp, Ry, Rz1);
    mat4x4_mul(R, Rz2, Rtmp);
}

/* mean obliquity of the ecliptic at J2000 (in radians) */
#define EPS0_MEAN_OBLIQ_J2000 (84381.406 * ASEC2RAD)

/* mean obliquity of date IAU 2006/2000A series */
static double lv_iau2006_obliquity_eps(double jd)
{
    double T = (jd - 2451545.0) / 36525.0;
    double eps_arcsec =
        84381.406
      - 46.836769*T
      - 0.0001831*T*T
      + 0.00200340*T*T*T
      - 0.000000576*T*T*T*T
      - 0.0000000434*T*T*T*T*T;
    return eps_arcsec * ASEC2RAD;
}

static void lv_iau2006_obliquity_matrix(mat4x4 R, double jd)
{
    mat4x4_identity(R);

    double eps = lv_iau2006_obliquity_eps(jd);

    float c = (float)cos(eps);
    float s = (float)sin(eps);

    /* rotation about X axis */
    R[1][1] =  c;
    R[1][2] =  s;
    R[2][1] = -s;
    R[2][2] =  c;
}

static void lv_iau2006_obliquity_basis(vec3 x0, vec3 y0, vec3 z0, double jd)
{
    mat4x4 m;
    lv_iau2006_obliquity_matrix(m, jd);

    vec4 X0 = {1, 0, 0, 1};
    vec4 Y0 = {0, 1, 0, 1};
    vec4 Z0 = {0, 0, 1, 1};

    mat4x4_mul_vec4(x0, m, X0);
    mat4x4_mul_vec4(y0, m, Y0);
    mat4x4_mul_vec4(z0, m, Z0);

    vec4_norm(x0, x0);
    vec4_norm(y0, y0);
    vec4_norm(z0, z0);
}

static void lv_iau2006_combined_matrix(mat4x4 R, double jd)
{
    mat4x4 Rpre, Robl;

    lv_iau2006_precession_matrix(Rpre, jd);
    lv_iau2006_obliquity_matrix(Robl, jd);

    mat4x4_mul(R, Rpre, Robl);
}

static void lv_iau2006_combined_basis(vec3 x0, vec3 y0, vec3 z0, double jd)
{
    mat4x4 R;

    lv_iau2006_combined_matrix(R, jd);

    vec4 X0 = {1, 0, 0, 1};
    vec4 Y0 = {0, 1, 0, 1};
    vec4 Z0 = {0, 0, 1, 1};

    mat4x4_mul_vec4(x0, R, X0);
    mat4x4_mul_vec4(y0, R, Y0);
    mat4x4_mul_vec4(z0, R, Z0);

    vec4_norm(x0, x0);
    vec4_norm(y0, y0);
    vec4_norm(z0, z0);
}

/*
 * color functions
 */

lv_color lv_rgb_to_hsv(lv_color c)
{
    lv_color r;

    float max = fmaxf(c.r, fmaxf(c.g, c.b));
    float min = fminf(c.r, fminf(c.g, c.b));
    float delta = max - min;

    r.v = max;
    r.a = c.a;

    r.s = delta / (max + FLT_EPSILON);

    if (max == c.r) {
        r.h = 60.0f * fmodf(((c.g - c.b) / (delta + FLT_EPSILON)), 6.0f);
    } else if (max == c.g) {
        r.h = 60.0f * (((c.b - c.r) / (delta + FLT_EPSILON)) + 2.0f);
    } else {
        r.h = 60.0f * (((c.r - c.g) / (delta + FLT_EPSILON)) + 4.0f);
    }

    if (r.h < 0.0f) {
        r.h += 360.0f;
    }

    return r;
}

lv_color lv_hsv_to_rgb(lv_color c)
{
    lv_color r;

    float vs = c.v * c.s;
    float x = vs * (1.0f - fabsf(fmodf(c.h / 60.0f, 2.0f) - 1.0f));
    float m = c.v - vs;

    float rp, gp, bp;

    if (c.h < 60.0f) {
        rp = vs; gp = x; bp = 0.0f;
    } else if (c.h < 120.0f) {
        rp = x; gp = vs; bp = 0.0f;
    } else if (c.h < 180.0f) {
        rp = 0.0f; gp = vs; bp = x;
    } else if (c.h < 240.0f) {
        rp = 0.0f; gp = x; bp = vs;
    } else if (c.h < 300.0f) {
        rp = x; gp = 0.0f; bp = vs;
    } else {
        rp = vs; gp = 0.0f; bp = x;
    }

    r.r = rp + m;
    r.g = gp + m;
    r.b = bp + m;
    r.a = c.a;

    return r;
}

static lv_color lv_color_adjust(lv_color c, float t_bright, float t_saturate)
{
    lv_color h;
    h = lv_rgb_to_hsv(c);
    h.v = fminf(fmaxf(h.v * t_bright, 0.0f), 1.0f);
    h.s = fminf(fmaxf(h.s * t_saturate, 0.0f), 1.0f);
    return lv_hsv_to_rgb(h);
}

#ifdef __cplusplus
}
#endif
