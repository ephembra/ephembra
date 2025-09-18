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

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

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

#ifdef __cplusplus
}
#endif
