#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "lv_ephem.h"
#include "lv_osdef.h"
#include "lv_osutil.h"

static const char* ephem_bin = "resources/data/DE440Coeff.bin";

static void de440_print_planet(const char *name, double *r)
{
    printf("%22s: (%10.3le,%10.3le,%10.3le)\n", name, r[0], r[1], r[2]);
}

void de440_print_ephemeris(ephem_ctx *ctx, double jd)
{
    size_t row = de440_find_row(ctx, jd);

    printf("%22s: %10.2lf\n", "MJD", jd);

    for (size_t oid = 1; oid < ephem_id_Last; oid++) {
        double obj[3];
        de440_ephem_obj(ctx, jd, row, oid, obj);
        de440_print_planet(de440_object_name(oid), obj);
    }
}

void demo(double jd)
{
    ephem_ctx ctx;
    char ephem_bin_rsrc[PATH_MAX];

    if (get_resource_path(ephem_bin_rsrc, sizeof(ephem_bin_rsrc),
        ephem_bin) != 0)
    {
        fprintf(stderr, "demo: failed to locate resources\n");
        exit(1);
    }

    memset(&ctx, 0, sizeof(ctx));
    de440_create_ephem(&ctx, ephem_bin_rsrc);
    de440_print_ephemeris(&ctx, jd);
    de440_destroy_ephem(&ctx);
}

int main(int argc, char **argv)
{
    double jd = 2460680.5;
    if (argc == 2) {
        jd = strtod(argv[1], NULL);
    } else if (argc > 2) {
        fprintf(stderr, "%s [julian_date]\n", argv[0]);
        exit(1);
    }
    demo(jd);
    return 0;
}
