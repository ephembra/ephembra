/*
 * gldemo - experiment to render the solar system
 *
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
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#define _USE_MATH_DEFINES
#include <math.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_MODULE_H
#include FT_GLYPH_H
#include FT_OUTLINE_H

#include "nanovg.h"
#define NANOVG_GLES3_IMPLEMENTATION
#include "nanovg_gl.h"
#include "nanovg_gl_utils.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "linmath.h"
#include "gl2_util.h"
#include "ephembra.h"

#include "lv_model.h"
#include "lv_ops_nanovg.h"
#include "lv_ops_buffer.h"
#include "lv_ops_xform.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

typedef struct lv_app lv_app;
typedef struct lv_oid lv_oid;

struct lv_app
{
    GLFWwindow* window;
    lv_context* ctx_nanovg;
    lv_context* ctx_buffer;
    lv_context* ctx_xform;
    mat4x4 m_mvp;
    mat4x4 m_inv;
    vec3 rotation;
    vec2f mouse;
    vec2f origin;
    float zoom;
    vec2f last_mouse;
    float last_zoom;
    size_t steps;
    size_t divs;
    int sjd, sjdf;
    int ejd, ejdf;
    int cjd, cjdf;
    int ljd, ljdf;
    int cartoon;
    int rotate;
    int loop;
    double *eph;
    ephem_ctx ctx;
};

struct lv_oid
{
    double dist;
    double orbit;
    lv_color *color;
};

static const char* ephembra_data_file = "build/data/DE440Coeff.bin";
static const char* ephembra_font_file = "build/fonts/DejaVuSansMono.ttf";
static const float min_zoom = 2.0f, max_zoom = 2048.0f;
static const float global_scale = 1e12;

static int opt_help;
static int opt_width = 1280;
static int opt_height = 720;

static lv_color blue;
static lv_color orange;
static lv_color green;
static lv_color red;
static lv_color purple;
static lv_color brown;
static lv_color pink;
static lv_color grey;
static lv_color olive;
static lv_color turquoise;
static lv_color yellow;
static lv_color charcoal;
static lv_color black;
static lv_color white;

static lv_oid data[10] = {
    {},
    /* Mercury   */ {   57909227,    87.969, &pink      },
    /* Venus     */ {  108209475,   224.701, &orange    },
    /* EarthMoon */ {  149598023,   365.256, &white     },
    /* Mars      */ {  227939200,   686.980, &red       },
    /* Jupiter   */ {  778340821,  4332.589, &olive     },
    /* Saturn    */ { 1426666422, 10759.220, &yellow    },
    /* Uranus    */ { 2870658186, 30685.400, &turquoise },
    /* Neptune   */ { 4498396441, 60190.030, &blue      },
    /* Pluto     */ { 5906376272, 90560.000, &grey      }
};

static void lv_vg_uinit(lv_app* app)
{
    app->ctx_nanovg = (lv_context*)calloc(1, sizeof(lv_context));
    app->ctx_buffer = (lv_context*)calloc(1, sizeof(lv_context));
    app->ctx_xform = (lv_context*)calloc(1, sizeof(lv_context));
    lv_vg_init(app->ctx_nanovg, &lv_nanovg_vg_ops, NULL);
    lv_vg_init(app->ctx_buffer, &lv_buffer_vg_ops, NULL);
    lv_vg_init(app->ctx_xform, &lv_xform_vg_ops, app->ctx_nanovg);
}

static void lv_vg_udestroy(lv_app* app)
{
    lv_vg_destroy(app->ctx_nanovg);
    lv_vg_destroy(app->ctx_buffer);
    lv_vg_destroy(app->ctx_xform);
    free(app->ctx_nanovg);
    free(app->ctx_buffer);
    free(app->ctx_xform);
}

static void lv_init_colors()
{
    blue = lv_rgb(0x1f,0x77,0xb4);
    orange = lv_rgb(0xff,0x7f,0x0e);
    green = lv_rgb(0x2c,0xa0,0x2c);
    red = lv_rgb(0xd6,0x27,0x28);
    purple = lv_rgb(0x94,0x67,0xbd);
    brown = lv_rgb(0x8c,0x56,0x4b);
    pink = lv_rgb(0xe3,0x77,0xc2);
    grey = lv_rgb(0x7f,0x7f,0x7f);
    olive = lv_rgb(0xbc,0xbd,0x22);
    turquoise = lv_rgb(0x17,0xbe,0xcf);
    yellow = lv_rgb(0xff,0xc0,0x00);
    charcoal = lv_rgb(0x20,0x20,0x20);
    black = lv_rgb(0x00,0x00,0x00);
    white = lv_rgb(0xd0,0xd0,0xd0);
}

static float deg_rad(float a) { return a * M_PI / 180.0f; }

static void model_matrix_transform(mat4x4 m, vec3 scale, vec3 trans,
    vec3 rot, float a, float r)
{
    mat4x4 m_model, m_proj;
    mat4x4_identity(m_model);
    mat4x4_translate_in_place(m_model, trans[0], trans[1], trans[2]);
    mat4x4_scale_aniso(m_model, m_model, scale[0], scale[1], scale[2]);
    mat4x4_rotate_X(m_model, m_model, deg_rad(rot[0]));
    mat4x4_rotate_Y(m_model, m_model, deg_rad(rot[1]) + deg_rad(a));
    mat4x4_rotate_Z(m_model, m_model, deg_rad(rot[2]));
    mat4x4_perspective(m_proj, deg_rad(30.f), r, 1.f, 1e6f);
    mat4x4_mul(m, m_proj, m_model);
}

static void lv_ephem_init(lv_app *app, size_t steps, size_t divs,
    double sjd, double ejd)
{
    app->steps = steps;
    app->divs = divs;
    app->cjd = sjd + (ejd - sjd) / 2.0;
    app->sjd = sjd;
    app->ejd = ejd;
    app->cjdf = 0.0;
    app->sjdf = -500.0;
    app->ejdf = 500.0;
    app->cartoon = 1;
    app->rotate = 0;
    app->loop = 0;
    de440_create_ephem(&app->ctx, ephembra_data_file);
    app->eph = (double*)malloc(10 * steps * sizeof(double) * 3);
}

static void lv_ephem_calc(lv_app *app, double jd)
{
    size_t steps = app->steps;
    for (size_t oid = 1; oid < 10; oid++) {
        for (size_t i = 0; i < steps; i++) {
            double interval = data[oid].orbit / steps;
            double tjd = jd - (i * interval);
            size_t row = de440_find_row(&app->ctx, tjd);
            double *obj = app->eph + oid * steps * 3 + i * 3;
            de440_ephem_obj(&app->ctx, tjd, row, oid, obj);
        }
    }
}

static void lv_planet(lv_app *app, lv_context* ctx, size_t oid,
    float s, lv_color color)
{
    size_t steps = app->steps, edges = app->steps / app->divs;

    lv_vg_stroke_width(ctx, 6.0f);
    for (size_t i = 0; i < steps; i += edges) {
        float alpha = (float)(steps-1-i) / steps;
        lv_vg_begin_path(ctx);
        double *o = app->eph + oid * steps * 3 +  i * 3;
        if (o[0] != o[0]) continue;
        lv_vg_3d_move_to(ctx,
            lv_point_3d(o[0]*s, o[1]*s, o[2]*s)
        );
        for (size_t j = i + 1; j <= i + edges && j < steps; j++) {
            double *o = app->eph + oid * steps * 3 + j * 3;
            if (o[0] != o[0]) break;
            lv_vg_3d_line_to(ctx,
                lv_point_3d(o[0]*s, o[1]*s, o[2]*s)
            );
        }
        lv_vg_stroke_color(ctx, lv_color_af(color, alpha));
        lv_vg_stroke(ctx);
    }
}

static void lv_render(lv_app* app, float w, float h, float r)
{
    static float a = 0.f;

    vec3 rot = { app->rotation[0], app->rotation[1], app->rotation[2] };
    vec3 scale = { 1/global_scale, 1/global_scale, 1/global_scale };
    vec3 trans = { 0.0f, 0.0f, app->zoom };
    vec2f origin = { app->origin.x, app->origin.y };
    mat4x4 m_model, m_proj;
    lv_context* ctx;
    int fb_width, fb_height;
    bool cartoon;
    bool rotate;
    bool loop;

    if (app->rotate) {
        a += 1.0f;
    }

    if (app->loop) {
        if (++app->cjdf > app->ejdf) app->cjdf = app->sjdf;
    }

    if (app->cjd != app->ljd || app->cjdf != app->ljdf) {
        lv_ephem_calc(app, app->cjd + app->cjdf);
        app->ljd = app->cjd;
        app->ljdf = app->cjdf;
    }

    ctx = app->ctx_buffer;
    lv_buffer_vg_clear(ctx);

    for (size_t oid = 1; oid < 10; oid++) {
        double r, s;
        if (app->cartoon) {
            r = data[oid].dist / data[ephem_id_Pluto].dist;
            s = (oid / 9.0) / r;
        } else { 
            s = 1.0;
        }
        lv_planet(app, ctx, oid, s, *data[oid].color);
    }

    model_matrix_transform(app->m_mvp, scale, trans, rot, a, r);
    mat4x4_invert(app->m_inv, app->m_mvp);

    ctx = app->ctx_xform;
    lv_xform_proj_matrix(app->ctx_xform, app->m_mvp, 1);
    lv_vg_begin_frame(ctx, w, h, r);
    lv_vg_reset(ctx);
    lv_vg_push(ctx);
    lv_buffer_vg_playback(app->ctx_buffer, ctx);
    lv_vg_pop(ctx);
    lv_vg_end_frame(ctx);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::Begin("Controller", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::PushItemWidth(1000.0f);
    ImGui::SliderInt("Julian Date", &app->cjd, app->sjd, app->ejd);
    ImGui::SliderInt("Fine Adjust", &app->cjdf, app->sjdf, app->ejdf);
    ImGui::PopItemWidth();
    cartoon = app->cartoon;
    rotate = app->rotate;
    loop = app->loop;
    ImGui::Checkbox("Cartoon", &cartoon);
    ImGui::SameLine();
    ImGui::Checkbox("Rotation", &rotate);
    ImGui::SameLine();
    ImGui::Checkbox("Loop", &loop);
    app->cartoon = cartoon;
    app->rotate = rotate;
    app->loop = loop;
    ImGui::End();
    ImGui::Render();

    glfwGetFramebufferSize(app->window, &fb_width, &fb_height);
    glViewport(0, 0, fb_width, fb_height);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

static void image_set_alpha(uchar* image, int w, int h, int stride, uchar a)
{
    int x, y;
    for (y = 0; y < h; y++) {
        uchar* row = &image[y*stride];
        for (x = 0; x < w; x++)
            row[x*4+3] = a;
    }
}

static void image_flip_horiz(uchar* image, int w, int h, int stride)
{
    int i = 0, j = h-1, k;
    while (i < j) {
        uchar* ri = &image[i * stride];
        uchar* rj = &image[j * stride];
        for (k = 0; k < w*4; k++) {
            uchar t = ri[k];
            ri[k] = rj[k];
            rj[k] = t;
        }
        i++;
        j--;
    }
}

static void lv_save_screenshot(int w, int h, const char* filename)
{
    uchar* image = (uchar*)malloc(w*h*4);
    if (image == NULL) return;
    glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, image);
    image_set_alpha(image, w, h, w*4, 255);
    image_flip_horiz(image, w, h, w*4);
#ifdef STB_IMAGE_WRITE_IMPLEMENTATION
    stbi_write_png(filename, w, h, 4, image, w*4);
#endif
    free(image);
}

static void key(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    NVG_NOTUSED(scancode);
    NVG_NOTUSED(mods);

    lv_app *ctx = (lv_app*)glfwGetWindowUserPointer(window);

    if(action == GLFW_RELEASE) return;

    switch (key) {
    case GLFW_KEY_ESCAPE:
        if (action == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
        break;
    case GLFW_KEY_Z: ctx->rotation[2] += 5.f; break;
    case GLFW_KEY_C: ctx->rotation[2] -= 5.f; break;
    case GLFW_KEY_W: ctx->rotation[0] += 5.f; break;
    case GLFW_KEY_S: ctx->rotation[0] -= 5.f; break;
    case GLFW_KEY_A: ctx->rotation[1] += 5.f; break;
    case GLFW_KEY_D: ctx->rotation[1] -= 5.f; break;
    case GLFW_KEY_P: {
        int fb_width, fb_height;
        glfwGetFramebufferSize(window, &fb_width, &fb_height);
        lv_save_screenshot(fb_width, fb_height, "screenshot.png");
        break;
    }
    default: return;
    }
}

static void scroll(GLFWwindow* window, double xoffset, double yoffset)
{
    lv_app *ctx = (lv_app*)glfwGetWindowUserPointer(window);

    lv_trace("scroll\n");

    float quantum = ctx->zoom / 16.f;
    float ratio = 1.f + (float)quantum / (float)ctx->zoom;
    if (yoffset < 0. && ctx->zoom < max_zoom) {
        ctx->origin.x *= ratio;
        ctx->origin.y *= ratio;
        ctx->zoom += quantum;
    } else if (yoffset > 0. && ctx->zoom > min_zoom) {
        ctx->origin.x /= ratio;
        ctx->origin.y /= ratio;
        ctx->zoom -= quantum;
    }
}

static int mouse_left_drag;
static int mouse_right_drag;

static void mouse_button(GLFWwindow* window, int button, int action, int mods)
{
    lv_app *ctx = (lv_app*)glfwGetWindowUserPointer(window);

    switch (button) {
    case GLFW_MOUSE_BUTTON_LEFT:
        mouse_left_drag = (action == GLFW_PRESS);
        ctx->last_mouse = ctx->mouse;
        ctx->last_zoom = ctx->zoom;
        break;
    case GLFW_MOUSE_BUTTON_RIGHT:
        mouse_right_drag = (action == GLFW_PRESS);
        ctx->last_mouse = ctx->mouse;
        ctx->last_zoom = ctx->zoom;
        break;
    }
}

static void cursor_position(GLFWwindow* window, double xpos, double ypos)
{
    lv_app *ctx = (lv_app*)glfwGetWindowUserPointer(window);

    ctx->mouse = (vec2f) { (float)xpos, (float)ypos };

    if (mouse_left_drag) {
        ctx->origin.x += ctx->mouse.x - ctx->last_mouse.x;
        ctx->origin.y += ctx->mouse.y - ctx->last_mouse.y;
        ctx->last_mouse = ctx->mouse;
    }
    if (mouse_right_drag) {
        float delta0 = ctx->mouse.x - ctx->last_mouse.x;
        float delta1 = ctx->mouse.y - ctx->last_mouse.y;
        float zoom = ctx->last_zoom * powf(65.0f/64.0f,(float)-delta1);
        if (zoom != ctx->zoom && zoom > min_zoom && zoom < max_zoom) {
            ctx->zoom = zoom;
            ctx->origin.x = (ctx->origin.x * (zoom / ctx->zoom));
            ctx->origin.y = (ctx->origin.y * (zoom / ctx->zoom));
        }
    }
}

static void lv_main_loop(GLFWwindow* window, lv_app *ctx)
{
    double target_fps = 60.0;
    double frame_time = 1.0 / target_fps;
    double last_time;

    while (!glfwWindowShouldClose(window))
    {
        double start_time = glfwGetTime();
        double delta      = start_time - last_time;

        int win_width, win_height;
        int fb_width, fb_height;
        float win_ratio;

        glfwGetWindowSize(window, &win_width, &win_height);
        glfwGetFramebufferSize(window, &fb_width, &fb_height);
        win_ratio = (float)win_width / (float)win_height;

        glViewport(0, 0, fb_width, fb_height);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
        lv_render(ctx, win_width, win_height, win_ratio);
        glfwSwapBuffers(window);
        glfwPollEvents();

        for (;;) {
            double end_time = glfwGetTime();
            double elapsed  = end_time - start_time;
            double wait_time = frame_time - elapsed;
            if (wait_time < 0.0) break;
            glfwWaitEventsTimeout(wait_time);
        }

        last_time = glfwGetTime();
      }
}

/*
 * option processing
 */

static void print_help(int argc, char **argv)
{
    lv_info(
        "usage: %s [options]\n"
        "\n"
        "Options:\n"
        "  -l, (info|debug|trace)             debug level\n"
        "  -w, --width <int>                  buffer width\n"
        "  -h, --height <int>                 buffer height\n"
        "  -h, --help                         command line help\n",
        argv[0]
    );
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
            opt_help++;
            i++;
        } else if (match_opt(argv[i], "-l", "--level")) {
            char* level = argv[++i];
            if (strcmp(level, "none") == 0) {
                lv_ll = lv_ll_none;
            } else if (strcmp(level, "info") == 0) {
                lv_ll = lv_ll_info;
            } else if (strcmp(level, "debug") == 0) {
                lv_ll = lv_ll_debug;
            } else if (strcmp(level, "trace") == 0) {
                lv_ll = lv_ll_trace;
            }
            i++;
        } else if (match_opt(argv[i], "-w", "--width")) {
            opt_width = atoi(argv[++i]);
            i++;
        } else if (match_opt(argv[i], "-h", "--height")) {
            opt_height = atoi(argv[++i]);
            i++;
        } else {
            lv_error("error: unknown option: %s\n", argv[i]);
            opt_help++;
            break;
        }
    }

    if (opt_help) {
        print_help(argc, argv);
        exit(1);
    }
}

static void errorcb(int error, const char* desc)
{
    lv_error("GLFW error %d: %s\n", error, desc);
}

void gllv_app(int argc, char **argv)
{
    GLFWwindow* window;
    lv_app app;

    memset(&app, 0, sizeof(app));
    app.zoom = 16.0f;
    app.rotation[0] = 45.0f;

    if (!glfwInit()) {
        lv_panic("glfwInit failed\n");
    }

    glfwSetErrorCallback(errorcb);

    glfwWindowHint(GLFW_SCALE_TO_MONITOR , GL_TRUE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    app.window = window = glfwCreateWindow(opt_width, opt_height,
        "ephembra", NULL, NULL);
    if (!window) {
        lv_panic("glfwCreateWindow failed\n");
    }

    glfwSetWindowUserPointer(window, &app);
    glfwSetKeyCallback(window, key);
    glfwSetScrollCallback(window, scroll);
    glfwSetMouseButtonCallback(window, mouse_button);
    glfwSetCursorPosCallback(window, cursor_position);
    glfwMakeContextCurrent(window);
    gladLoadGL();
    glfwSwapInterval(0);
    glfwSetTime(0);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear();
    io.Fonts->AddFontFromFileTTF(ephembra_font_file, 14.0f);
    io.Fonts->Build();
    io.FontGlobalScale = 2.0f;
    ImGui::GetStyle().ScaleAllSizes(1.5f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glClearColor(0.f, 0.f, 0.f, 1.f);

    lv_ephem_init(&app, 360, 36, 2323710.5, 2615904.5);

    lv_init_colors();
    lv_vg_uinit(&app);
    lv_main_loop(window, &app);
    lv_vg_udestroy(&app);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    de440_destroy_ephem(&app.ctx);
    free(app.eph);

    glfwTerminate();
}

/*
 * main program
 */

int main(int argc, char **argv)
{
    lv_ll = lv_ll_info;
    parse_options(argc, argv);
    gllv_app(argc, argv);
    return 0;
}
