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
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <time.h>

#define _USE_MATH_DEFINES
#include <math.h>

#ifdef HAVE_GLAD
#include <glad/glad.h>
#else
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#endif
#include <GLFW/glfw3.h>

#include "demolib.h"

/*
 * object space screen space projection
 */

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

/*
 * date functions
 */

static int is_gregorian_date(int y, int m, int d)
{
    return (y > 1582 ||
           (y == 1582 && m > 10) ||
           (y == 1582 && m == 10 && d >= 15));
}

static int is_gregorian_leap(int y)
{
    return (y % 4 == 0) && (y % 100 != 0 || y % 400 == 0);
}

static int is_julian_leap(int y)
{
    return (y % 4) == 0;
}

double lv_date_to_julian(lv_date d)
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

    if (!is_gregorian_date(d.year, d.month, d.day)) B = 0;

    double JD = floor(365.25 * (Y + 4716))
              + floor(30.6001 * (M + 1))
              + (d.hour + d.minute / 60.0 + d.second / 3600.0) / 24.0
              + D + B - 1524.5;

    return JD;
}

lv_date lv_julian_to_date(double jd)
{
    long Z = (long)floor(jd + 0.5);
    double F = jd + 0.5 - Z;

    long A;
    if (Z < 2299161L) {
        A = Z;
    } else {
        long alpha = (long)floor((Z - 1867216.25) / 36524.25);
        A = Z + 1 + alpha - (alpha / 4);
    }

    long B = A + 1524;
    long C = (long)floor((B - 122.1) / 365.25);
    long D = (long)floor(365.25 * C);
    long E = (long)floor((B - D) / 30.6001);

    double dayf = B - D - floor(30.6001 * E) + F;
    int day = (int)floor(dayf);
    double frac = dayf - day;

    int month = (E < 14) ? (int)(E - 1) : (int)(E - 13);
    int year  = (month > 2) ? (int)(C - 4716) : (int)(C - 4715);

    double hh = frac * 24.0;
    int hour = (int)floor(hh);
    double mmf = (hh - hour) * 60.0;
    int minute = (int)floor(mmf);
    double ssf = (mmf - minute) * 60.0;
    int second = (int)floor(ssf + 0.5);

    if (second >= 60) {
        second -= 60;
        minute += 1;
    }

    if (minute >= 60) {
        minute -= 60;
        hour += 1;
    }

    if (hour >= 24) {
        hour -= 24;
        day += 1;

        int dim = lv_days_in_month(year, month - 1);
        if (day > dim) {
            day = 1;
            month += 1;
            if (month > 12) {
                month = 1;
                year += 1;
            }
        }
    }

    lv_date d = {
        year,
        month,
        day,
        hour,
        minute,
        second
    };

    return d;
}

int lv_days_in_month(int year, int month)
{
    const int dim[12] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
    if (month == 2) {
        if (is_gregorian_date(year, month, 15)) {
            if (is_gregorian_leap(year)) return 29;
        } else {
            if (is_julian_leap(year)) return 29;
        }
    }
    return dim[month - 1];
}

size_t lv_format_date(char *buf, size_t buflen, lv_date *d)
{
    static const char *month_names[] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };
    return snprintf(buf, buflen, "%02d %3s %04d %02d:%02d:%02d GMT",
        d->day, month_names[d->month-1], d->year,
        d->hour, d->minute, d->second);
}

/*
 * astronomical functions
 */

/* create precession rotation matrix (IAU 2006) for julian date */
void lv_iau2006_precession_matrix(mat4x4 R, double jd)
{
    double T = (jd - 2451545.0) / 36525.0;

    /* Precession angles in arcseconds */
    float zeta_A = (float)
        ((2306.083227*T + 0.2988499*T*T + 0.01801828*T*T*T
          - 0.000005971*T*T*T*T - 0.0000003173*T*T*T*T*T) * ASEC2RAD);

    float theta_A = (float)
        ((2004.191903*T - 0.4294934*T*T - 0.04182264*T*T*T
          - 0.000007089*T*T*T*T - 0.0000001274*T*T*T*T*T) * ASEC2RAD);

    float z_A = (float)
        ((2306.077181*T + 1.0927348*T*T + 0.01826837*T*T*T
          - 0.000028596*T*T*T*T - 0.0000002904*T*T*T*T*T) * ASEC2RAD);

    mat4x4 Rz1, Ry, Rz2, Rtmp;

    mat4x4_identity(Rz1);
    mat4x4_identity(Ry);
    mat4x4_identity(Rz2);

    /* Rz(-zeta_A) */
    Rz1[0][0] = cosf(-zeta_A); Rz1[0][1] = -sinf(-zeta_A);
    Rz1[1][0] = sinf(-zeta_A); Rz1[1][1] =  cosf(-zeta_A);

    /* Ry(theta_A) */
    Ry[0][0] =  cosf(theta_A); Ry[0][2] = sinf(theta_A);
    Ry[2][0] = -sinf(theta_A); Ry[2][2] = cosf(theta_A);

    /* Rz(-z_A) */
    Rz2[0][0] = cosf(-z_A); Rz2[0][1] = -sinf(-z_A);
    Rz2[1][0] = sinf(-z_A); Rz2[1][1] =  cosf(-z_A);

    /* Multiply: R = Rz2 * Ry * Rz1 */
    mat4x4_mul(Rtmp, Ry, Rz1);
    mat4x4_mul(R, Rz2, Rtmp);
}

/* mean obliquity of date IAU 2006/2000A series */
double lv_iau2006_obliquity_eps(double jd)
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

void lv_iau2006_obliquity_matrix(mat4x4 R, double jd)
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

void lv_iau2006_obliquity_basis(vec3 x0, vec3 y0, vec3 z0, double jd)
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

void lv_iau2006_combined_matrix(mat4x4 R, double jd)
{
    mat4x4 Rpre, Robl;

    lv_iau2006_precession_matrix(Rpre, jd);
    lv_iau2006_obliquity_matrix(Robl, jd);

    mat4x4_mul(R, Rpre, Robl);
}

void lv_iau2006_combined_basis(vec3 x0, vec3 y0, vec3 z0, double jd)
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

lv_color lv_color_adjust(lv_color c, float t_bright, float t_saturate)
{
    lv_color h;
    h = lv_rgb_to_hsv(c);
    h.v = fminf(fmaxf(h.v * t_bright, 0.0f), 1.0f);
    h.s = fminf(fmaxf(h.s * t_saturate, 0.0f), 1.0f);
    return lv_hsv_to_rgb(h);
}
