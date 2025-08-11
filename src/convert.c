#include <stdio.h>
#include <stdlib.h>

#include "matio.h"
#include "ephembra.h"

#define VA_ARGS(...) , ##__VA_ARGS__
#define ephem_error(fmt, ...) \
    fprintf(stderr, fmt "\n" VA_ARGS(__VA_ARGS__)); exit(1);

void convert(const char *ephem_mat, const char *ephem_bin)
{
    mat_t *f;
    matvar_t *v;
    size_t rows, cols, dsize, nbytes;
    double *src, *buf;
    FILE *out;

    f = Mat_Open(ephem_mat, MAT_ACC_RDONLY);
    v = Mat_VarRead(f, "DE440Coeff");
    if (!v || !v->data) {
        ephem_error("mat_open: failed to read: %s", ephem_mat);
    }

    rows = v->dims[0];
    cols = v->dims[1];
    dsize = rows * cols * sizeof(double);
    src = v->data;
    buf = malloc(dsize);
    if (!buf) {
        ephem_error("malloc: failed to allocate %zu bytes", dsize);
    }

    for (size_t i = 0; i < rows; i++) {
        for (size_t j = 0; j < cols; j++) {
            double *r = src + j * rows + i;
            double *w = buf + i * cols + j;
            *w = *r;
        }
    }

    out = fopen(ephem_bin, "w");
    if (!out) {
        ephem_error("fopen: failed: %s", ephem_bin);
    }
    nbytes = fwrite(&rows, 1, sizeof(size_t), out);
    if (nbytes != sizeof(size_t)) {
        ephem_error("fwrite: invalid size: %zu != %zu", nbytes, sizeof(size_t));
    }
    nbytes = fwrite(&cols, 1, sizeof(size_t), out);
    if (nbytes != sizeof(size_t)) {
        ephem_error("fwrite: invalid size: %zu != %zu", nbytes, sizeof(size_t));
    }
    nbytes = fwrite((char*)buf, 1, dsize, out);
    if (nbytes != dsize) {
        ephem_error("fwrite: invalid size: %zu != %zu", nbytes, dsize);
    }
    fclose(out);

    free(buf);

    Mat_VarFree(v);
    Mat_Close(f);
}

int main(int argc, char **argv)
{
    if (argc != 3) {
        fprintf(stderr, "usage: %s [DE440Coeff.mat] [DE440Coeff.bin]\n", argv[0]);
        exit(1);
    }
    convert(argv[1], argv[2]);
    return 0;
}
