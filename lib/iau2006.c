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

#define _USE_MATH_DEFINES
#include <math.h>

#include "linmath.h"

#include "lv_iau2006.h"

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
