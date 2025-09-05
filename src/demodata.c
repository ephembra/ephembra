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

#include "demodata.h"

#define countof(arr) (sizeof(arr)/sizeof(arr[0]))

lv_oid data[11] = {
    {
        ephem_id_Sun,
        "☉", "Sun",            1000000,    695700,      0.000,
        { 1.00, 0.84, 0.00, 1.0 } /* golden-yellow photosphere */
    },
    {
        ephem_id_Moon,
        "☽", "Moon",         149598023,      3474,     27.320,
        { 0.75, 0.75, 0.75, 1.0 } /* pale grey */
    },
    {
        ephem_id_Mercury_Barycenter,
        "☿", "Mercury",       57909227,      4879,     87.969,
        { 0.60, 0.60, 0.60, 1.0 } /* mid-grey, rocky */
    },
    {
        ephem_id_Venus_Barycenter,
        "♀", "Venus",        108209475,     12104,    224.701,
        { 0.96, 0.89, 0.70, 1.0 } /* pale golden cream */
    },
    {
        ephem_id_Earth,
        "♁", "Earth",        149598023,     12742,    365.256,
        { 0.27, 0.55, 0.68, 1.0 } /* blue-green oceans/land */
    },
    {
        ephem_id_Mars_Barycenter,
        "♂", "Mars",         227939200,      6779,    686.980,
        { 0.70, 0.40, 0.35, 1.0 } /* reddish-orange dusty soil */
    },
    {
        ephem_id_Jupiter_Barycenter,
        "♃", "Jupiter",      778340821,    139820,   4332.589,
        { 0.87, 0.72, 0.53, 1.0 } /* beige bands with light brown */
    },
    {
        ephem_id_Saturn_Barycenter,
        "♄", "Saturn",      1426666422,    116460,  10759.220,
        { 0.93, 0.85, 0.63, 1.0 } /* pale yellow-brown */
    },
    {
        ephem_id_Uranus_Barycenter,
        "♅", "Uranus",      2870658186,     50724,  30687.000,
        { 0.56, 0.75, 0.82, 1.0 } /* pale cyan */
    },
    {
        ephem_id_Neptune_Barycenter,
        "♆", "Neptune",     4498396441,     49244,  60190.000,
        { 0.28, 0.35, 0.68, 1.0 } /* deep azure blue */
    },
    {
        ephem_id_Pluto_Barycenter,
        "♇", "Pluto",       5906376272,      2377,  90560.000,
        { 0.72, 0.62, 0.57, 1.0 } /* light brown-grey, icy patches */
    },
};

lv_sign signs[12] = {
    { "♈", "Aries",       { 0.937, 0.325, 0.314, 1.000 } }, /* U+2648 */
    { "♉", "Taurus",      { 1.000, 0.439, 0.263, 1.000 } }, /* U+2649 */
    { "♊", "Gemini",      { 1.000, 0.655, 0.149, 1.000 } }, /* U+264A */
    { "♋", "Cancer",      { 1.000, 0.800, 0.196, 1.000 } }, /* U+264B */
    { "♌", "Leo",         { 0.988, 0.894, 0.220, 1.000 } }, /* U+264C */
    { "♍", "Virgo",       { 0.612, 0.800, 0.396, 1.000 } }, /* U+264D */
    { "♎", "Libra",       { 0.400, 0.733, 0.416, 1.000 } }, /* U+264E */
    { "♏", "Scorpio",     { 0.149, 0.651, 0.604, 1.000 } }, /* U+264F */
    { "♐", "Sagittarius", { 0.980, 0.463, 0.824, 1.000 } }, /* U+2650 */
    { "♑", "Capricorn",   { 0.494, 0.341, 0.761, 1.000 } }, /* U+2651 */
    { "♒", "Aquarius",    { 0.671, 0.278, 0.737, 1.000 } }, /* U+2652 */
    { "♓", "Pisces",      { 0.925, 0.251, 0.478, 1.000 } }  /* U+2653 */
};

size_t data_count = countof(data);
size_t sign_count = countof(signs);
