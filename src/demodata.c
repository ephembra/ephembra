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
        { 1.00f, 0.84f, 0.00f, 1.0f } /* golden-yellow photosphere */
    },
    {
        ephem_id_Moon,
        "☽", "Moon",         149598023,      3474,     27.320,
        { 0.75f, 0.75f, 0.75f, 1.0f } /* pale grey */
    },
    {
        ephem_id_Mercury_Barycenter,
        "☿", "Mercury",       57909227,      4879,     87.969,
        { 0.60f, 0.60f, 0.60f, 1.0f } /* mid-grey, rocky */
    },
    {
        ephem_id_Venus_Barycenter,
        "♀", "Venus",        108209475,     12104,    224.701,
        { 0.96f, 0.89f, 0.70f, 1.0f } /* pale golden cream */
    },
    {
        ephem_id_Earth,
        "♁", "Earth",        149598023,     12742,    365.256,
        { 0.27f, 0.55f, 0.68f, 1.0f } /* blue-green oceans/land */
    },
    {
        ephem_id_Mars_Barycenter,
        "♂", "Mars",         227939200,      6779,    686.980,
        { 0.70f, 0.40f, 0.35f, 1.0f } /* reddish-orange dusty soil */
    },
    {
        ephem_id_Jupiter_Barycenter,
        "♃", "Jupiter",      778340821,    139820,   4332.589,
        { 0.87f, 0.72f, 0.53f, 1.0f } /* beige bands with light brown */
    },
    {
        ephem_id_Saturn_Barycenter,
        "♄", "Saturn",      1426666422,    116460,  10759.220,
        { 0.93f, 0.85f, 0.63f, 1.0f } /* pale yellow-brown */
    },
    {
        ephem_id_Uranus_Barycenter,
        "♅", "Uranus",      2870658186,     50724,  30687.000,
        { 0.56f, 0.75f, 0.82f, 1.0f } /* pale cyan */
    },
    {
        ephem_id_Neptune_Barycenter,
        "♆", "Neptune",     4498396441,     49244,  60190.000,
        { 0.28f, 0.35f, 0.68f, 1.0f } /* deep azure blue */
    },
    {
        ephem_id_Pluto_Barycenter,
        "♇", "Pluto",       5906376272,      2377,  90560.000,
        { 0.72f, 0.62f, 0.57f, 1.0f } /* light brown-grey, icy patches */
    },
};

lv_sign signs[12] = {
    { "♈", "Aries",       { 0.937f, 0.325f, 0.314f, 1.0f } }, /* U+2648 */
    { "♉", "Taurus",      { 1.000f, 0.439f, 0.263f, 1.0f } }, /* U+2649 */
    { "♊", "Gemini",      { 1.000f, 0.655f, 0.149f, 1.0f } }, /* U+264A */
    { "♋", "Cancer",      { 1.000f, 0.800f, 0.196f, 1.0f } }, /* U+264B */
    { "♌", "Leo",         { 0.988f, 0.894f, 0.220f, 1.0f } }, /* U+264C */
    { "♍", "Virgo",       { 0.612f, 0.800f, 0.396f, 1.0f } }, /* U+264D */
    { "♎", "Libra",       { 0.400f, 0.733f, 0.416f, 1.0f } }, /* U+264E */
    { "♏", "Scorpio",     { 0.149f, 0.651f, 0.604f, 1.0f } }, /* U+264F */
    { "♐", "Sagittarius", { 0.980f, 0.463f, 0.824f, 1.0f } }, /* U+2650 */
    { "♑", "Capricorn",   { 0.494f, 0.341f, 0.761f, 1.0f } }, /* U+2651 */
    { "♒", "Aquarius",    { 0.671f, 0.278f, 0.737f, 1.0f } }, /* U+2652 */
    { "♓", "Pisces",      { 0.925f, 0.251f, 0.478f, 1.0f } }  /* U+2653 */
};

size_t data_count = countof(data);
size_t sign_count = countof(signs);
