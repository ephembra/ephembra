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

#include <stdio.h>
#include <stddef.h>
#include <math.h>

#include "lv_date.h"

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
