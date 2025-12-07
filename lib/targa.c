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
#include <string.h>

#include "lv_osdef.h"
#include "lv_targa.h"

typedef unsigned char uchar;

struct lv_targa
{
    uchar length, palette, type, cmap_offset_lo, cmap_offset_hi,
        cmap_length_lo, cmap_length_hi, cmap_depth,
        xorigin_lo, xorigin_hi, yorigin_lo, yorigin_hi,
        width_lo, width_hi, height_lo, height_hi, bits, flags;
};

int lv_targa_write_bgra(const char *filename, size_t w, size_t h,
    void *data, int type, size_t in_stride, size_t out_stride)
{
    size_t pixels = w * h, is = in_stride, os = out_stride;
    struct lv_targa H;
    FILE *f;

    uchar *p = (unsigned char *)data;
    uchar *buf = alloca(os * 128);

    if (w > 65535 || h > 65535) return -1;

    memset(&H, 0, sizeof(H));
    H.type      = type;
    H.width_lo  = (uchar)((w)      & 0xff);
    H.width_hi  = (uchar)((w >> 8) & 0xff);
    H.height_lo = (uchar)((h)      & 0xff);
    H.height_hi = (uchar)((h >> 8) & 0xff);
    switch (os) {
    case 1:
        H.bits = 8;
        break;
    case 3:
        H.bits = 24;
        break;
    case 4:
        H.bits = 32;
        H.flags = lv_targa_flag_alpha;
        break;
    default:
        return -1;
    }

    if (!(f = fopen(filename, "wb"))) return -1;

    if (fwrite(&H, 1, sizeof(H), f) != sizeof(H)) goto err;

    switch (type) {
    case lv_targa_type_rgb_rle:
    case lv_targa_type_gray_rle:
        for (size_t i = 0; i < pixels;) {
            size_t l = 1;

            while (i + l < pixels && l < 128 &&
                memcmp(p + i * is, p + (i + l) * is, os) == 0) l++;

            if (l > 1) {
                uchar header = 0x80 | (uchar)(l - 1);
                if (fwrite(&header, 1, 1, f) != 1) goto err;
                if (fwrite(p + i * is, os, 1, f) != 1) goto err;
                i += l;
            } else {
                while (i + l < pixels && l < 128 &&
                    memcmp(p + (i + l - 1) * is, p + (i + l) * is, os) != 0) l++;

                for (size_t j = 0; j < l; j++) {
                    memcpy(buf + j * os, p + (i + j) * is, os);
                }

                uchar header = (uchar)(l - 1);
                if (fwrite(&header, 1, 1, f) != 1) goto err;
                if (fwrite(buf, os, l, f) != l) goto err;
                i += l;
            }
        }
        break;
    case lv_targa_type_rgb:
    case lv_targa_type_gray:
        if (is == os) {
            if (fwrite(p, os, pixels, f) != pixels) goto err;
        } else {
            for (size_t i = 0; i < pixels; i += 128) {
                size_t l = pixels - i > 128 ? 128 : pixels - i;
                for (size_t j = 0; j < l; j++) {
                    memcpy(buf + j * os, p + (i + j) * is, os);
                }
                if (fwrite(buf, os, l, f) != l) goto err;
            }
        }
        break;
    }

    fclose(f);
    return 0;

err:
    fclose(f);
    return -1;
}
