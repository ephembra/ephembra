#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define _USE_MATH_DEFINES
#include <math.h>

#include "linmath.h"

#include "lv_data.h"
#include "lv_date.h"
#include "lv_iau2006.h"
#include "lv_ephem.h"
#include "lv_osdef.h"
#include "lv_osutil.h"
#include "lv_vecalg.h"

static const char* ephem_bin = "resources/data/DE440Coeff.bin";

static double sjd = 2415020.5;
static double ejd = 2488069.5;
static double step = 0.25;
static int align = 6;
static int help = 0;

static void search_ephem(ephem_ctx *ctx,
    double sjd, double ejd, double step, int align)
{
    lv_date t1, t2;
    char ds1[64], ds2[64];

    t1 = lv_julian_to_date(sjd);
    t2 = lv_julian_to_date(ejd);
    lv_format_date(ds1, sizeof(ds1), &t1);
    lv_format_date(ds2, sizeof(ds2), &t2);

    printf("start date : %s\n", ds1);
    printf("end date   : %s\n", ds2);
    printf("\n");

    for (double jd = sjd; jd <=ejd; jd += step)
    {
        mat4x4 m, im;
        vec3 e, o, d;
        vec4 q, p;
        float a;
        double obj[3];
        int deg, min, sid;
        lv_date t;
        float chart[11];
        int zcount[12] = { 0 };
        int found = 0;
        char ds[64];

        size_t row = de440_find_row(ctx, jd);

        de440_ephem_obj(ctx, jd, row, ephem_id_Earth, obj);
        vec3_double3(e, obj);
        lv_iau2006_combined_matrix(m, jd);
        mat4x4_invert(im, m);

        memset(chart, 0, sizeof(chart));
        for (size_t idx = 0; idx < data_count; idx++)
        {
            size_t oid = data[idx].oid;
            if (oid == ephem_id_Earth) continue;
            de440_ephem_obj(ctx, jd, row, oid, obj);
            vec3_double3(o, obj);
            vec3_sub(d, o, e);
            vec4_vec3_w1(q, d);
            mat4x4_mul_vec4(p, im, q);
            a = vector_angle_deg(p[0], p[1]);
            chart[idx] = a;
            deg = (int)floorf(a);
            sid = deg / 30;
            zcount[sid]++;
        }

        for (size_t sid = 0; sid < 12; sid++) {
            if (zcount[sid] >= align) found = 1;
        }
        if (!found) continue;

        t = lv_julian_to_date(jd);
        lv_format_date(ds, sizeof(ds), &t);
        printf("julian date: %12.3f, %s\n", jd, ds);

        for (size_t idx = 0; idx < data_count; idx++)
        {
            size_t oid = data[idx].oid;

            if (oid == ephem_id_Earth) continue;

            a = chart[idx];
            deg = (int)floorf(a);
            min = (int)floorf((a - deg) * 60.0f);
            sid = deg / 30;

            printf("%s %-20s %2d°%02d′ %s %-20s\n",
                data[idx].symbol, data[idx].name,
                deg % 30, min,
                signs[sid].symbol, signs[sid].name);
        }
        printf("\n");
    }
}

static void do_search()
{
    ephem_ctx ctx;
    char ephem_bin_rsrc[PATH_MAX];

    if (get_resource_path(ephem_bin_rsrc, sizeof(ephem_bin_rsrc),
        ephem_bin) != 0)
    {
        fprintf(stderr, "search: failed to locate resources\n");
        exit(1);
    }

    memset(&ctx, 0, sizeof(ctx));
    de440_create_ephem(&ctx, ephem_bin_rsrc);
    search_ephem(&ctx, sjd, ejd, step, align);
    de440_destroy_ephem(&ctx);
}

static void print_help(int argc, char **argv)
{
    fprintf(stderr,
        "Usage: %s [options]\n"
        "\n"
        "Options:\n"
        "  -S, --start <double>            start julian date\n"
        "  -E, --end <double>              end julian date\n"
        "  -I, --step <double>             julian date increment\n"
        "  -A, --align <int>               count of aligned objects\n"
        "  -h, --help                      command line help\n",
        argv[0]);
}

static int match_opt(const char *arg, const char *opt, const char *longopt)
{
    return strcmp(arg, opt) == 0 || strcmp(arg, longopt) == 0;
}

static void parse_options(int argc, char **argv)
{
    int i = 1;
    while (i < argc) {
        if (match_opt(argv[i], "-h", "--help")) {
            help++;
            i++;
        } else if (match_opt(argv[i], "-S", "--start")) {
            sjd = atof(argv[++i]);
            i++;
        } else if (match_opt(argv[i], "-E", "--end")) {
            ejd = atof(argv[++i]);
            i++;
        } else if (match_opt(argv[i], "-I", "--step")) {
            step = atof(argv[++i]);
            i++;
        } else if (match_opt(argv[i], "-A", "--align")) {
            align = atoi(argv[++i]);
            i++;
        } else {
            fprintf(stderr, "error: unknown option: %s\n", argv[i]);
            help++;
            break;
        }
    }

    if (help) {
        print_help(argc, argv);
        exit(1);
    }
}

int main(int argc, char **argv)
{
    parse_options(argc, argv);
    do_search();
    return 0;
}
