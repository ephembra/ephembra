/*
 * glui - imgui solar system demo control
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
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <time.h>

#define _USE_MATH_DEFINES
#include <math.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "demolib.h"

#include "nanovg.h"
#define NANOVG_GLES3
#include "nanovg_gl.h"
#include "nanovg_gl_utils.h"

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

#include "gldemo.h"

#define countof(arr) (sizeof(arr)/sizeof(arr[0]))

static const int font_sizes[] = {
    8, 9, 10, 11, 12, 14, 16, 18, 20, 22, 24, 26, 28, 36, 48, 72
};

void lv_vg_uinit(lv_app* app)
{
    app->ctx_nanovg = (lv_context*)calloc(1, sizeof(lv_context));
    app->ctx_buffer = (lv_context*)calloc(1, sizeof(lv_context));
    app->ctx_xform = (lv_context*)calloc(1, sizeof(lv_context));
    lv_vg_init(app->ctx_nanovg, &lv_nanovg_vg_ops, NULL);
    lv_vg_init(app->ctx_buffer, &lv_buffer_vg_ops, NULL);
    lv_vg_init(app->ctx_xform, &lv_xform_vg_ops, app->ctx_nanovg);
}

void lv_vg_udestroy(lv_app* app)
{
    lv_vg_destroy(app->ctx_nanovg);
    lv_vg_destroy(app->ctx_buffer);
    lv_vg_destroy(app->ctx_xform);
    free(app->ctx_nanovg);
    free(app->ctx_buffer);
    free(app->ctx_xform);
}

void lv_app_imgui_init(lv_app *app)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(app->window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    ImFontConfig icons_config;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;
    ImWchar icons_ranges[] = { 0xf000, 0xf3ff, 0 };

    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear();
    io.Fonts->AddFontFromFileTTF(ephembra_mono_font, 14.0f);
    io.Fonts->AddFontFromFileTTF(ephembra_awes_font, 14.0f,
        &icons_config, icons_ranges);
    io.Fonts->Build();
    io.FontGlobalScale = app->ui_scale;
}

void lv_app_imgui_destroy()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();	
}

void lv_date_picker(lv_date *date)
{
    int dim;

    ImGui::Text("Date:");
    ImGui::SameLine();
    ImGui::PushItemWidth(200);
    ImGui::InputInt("YYYY", &date->year);

    if (date->year < 1700) {
        date->year = 1700;
        date->month = 1;
        date->day = 1;
    } else if (date->year > 2399) {
        date->year = 2399;
        date->month = 12;
        date->day = 31;
    }

    ImGui::PopItemWidth();
    ImGui::PushItemWidth(150);
    ImGui::SameLine();
    ImGui::InputInt("MM", &date->month);

    if (date->month < 1) {
        date->year--;
        date->month = 12;
        if (date->year < 1700) {
            date->year = 1700;
            date->month = 1;
            date->day = 1;
        }
    } else if (date->month > 12) {
        date->year++;
        date->month = 1;
        if (date->year > 2399) {
            date->year = 2399;
            date->month = 12;
            date->day = 31;
        }
    }

    ImGui::SameLine();
    ImGui::InputInt("DD", &date->day);
    ImGui::PopItemWidth();

    dim = lv_days_in_month(date->year, date->month-1);
    if (date->day < 1) {
        date->month--;
        if (date->month < 1) {
            date->year--;
            date->month = 12;
            if (date->year < 1700) {
                date->year = 1700;
                date->month = 1;
                date->day = 1;
            } else {
                date->day = lv_days_in_month(date->year, date->month-1);
            }
        } else {
            date->day = lv_days_in_month(date->year, date->month-1);
        }
    } else if (date->day > dim) {
        date->month++;
        if (date->month > 12) {
            date->month = 1;
            date->year++;
            if (date->year > 2399) {
                date->year = 2399;
                date->month = 12;
                date->day = 31;
            } else {
                date->day = 1;
            }
        } else {
            date->day = 1;
        }
    }
}

void lv_font_size(const char *label, int *font_size)
{
    char font_size_str[16];
    snprintf(font_size_str, sizeof(font_size_str), "%u", *font_size);

    if (ImGui::BeginCombo(label, font_size_str)) {
        for (int i = 0; i < IM_ARRAYSIZE(font_sizes); i++)
        {
            bool sel = (*font_size == font_sizes[i]);
            snprintf(font_size_str, sizeof(font_size_str), "%u", font_sizes[i]);
            if (ImGui::Selectable(font_size_str, sel)) {
                *font_size = font_sizes[i];
            }
            if (sel) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
}

void lv_imgui(lv_app* app, float w, float h, float r)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    ImGui::NewFrame();
    ImGui::Begin("Controller", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::PushItemWidth(1000.0f);
    if (ImGui::SliderInt("Julian Date", &app->cjd, app->sjd, app->ejd)) {
        lv_update_date(app);
    }
    if (ImGui::SliderInt("Fine Adjust", &app->cjdf, app->sjdf, app->ejdf)) {
        lv_update_date(app);
    }
    ImGui::PopItemWidth();
    if (ImGui::Button(app->playback  ? "\uf04c##playback"
                                     : "\uf04b##playback", ImVec2(36, 36))) {
        app->playback = !app->playback;
    }
    ImGui::SameLine();
    lv_date_picker(&app->date);
    ImGui::End();

    ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("Model");
    ImGui::Separator();
    ImGui::Checkbox("IAU 2006 Precession", &app->precession);
    ImGui::SliderFloat("Rotate X", &app->rot[0], -180.0f, 180.0f);
    ImGui::SliderFloat("Rotate Y", &app->rot[1], -180.0f, 180.0f);
    ImGui::SliderFloat("Rotate Z", &app->rot[2], -180.0f, 180.0f);
    ImGui::SliderFloat("Translate X", &app->trans[0], -10.0f, 10.0f);
    ImGui::SliderFloat("Translate Y", &app->trans[1], -10.0f, 10.0f);
    ImGui::SliderFloat("Translate Z", &app->trans[2], -10.0f, 10.0f);

    ImGui::Text("Style");
    ImGui::Separator();
    ImGui::Checkbox("Cartoon Scaling", &app->cartoon);
    ImGui::SliderFloat("Trail Width", &app->trail_width, 0.0f, 12.0f);
    ImGui::SliderFloat("Line Width", &app->line_width, 0.0f, 6.0f);
    ImGui::SliderFloat("Planet Scale", &app->planet_scale, 0.0f, 5.0f);
    lv_font_size("Font Size", &app->font_size);
    lv_font_size("Symbol Size", &app->symbol_size);

    ImGui::Text("Grid");
    ImGui::Separator();
    ImGui::Checkbox("Grid Layer", &app->grid_layer);
    ImGui::SliderInt("Grid Divisions", &app->grid_steps, 1, 20);
    ImGui::SliderFloat("Grid Scale", &app->grid_scale, 0.0f, 20.0f);

    ImGui::Text("Zodiac");
    ImGui::Separator();
    ImGui::Checkbox("Zodiac Layer", &app->zodiac_layer);
    ImGui::SliderFloat("Symbol Offset", &app->symbol_offset, -0.1f, 0.1f);
    ImGui::SliderFloat("Zodiac Offset", &app->zodiac_offset, 0.0f, 20.0f);
    ImGui::SliderFloat("Zodiac Scale", &app->zodiac_scale, 0.0f, 20.0f);

    ImGui::Text("Legends");
    ImGui::Separator();
    ImGui::Checkbox("Symbols", &app->sym_legend);
    ImGui::Checkbox("Names", &app->name_legend);
    ImGui::Checkbox("Distances", &app->dist_legend);

    ImGui::End();

    ImGui::Begin("Chart", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    if (ImGui::BeginTable("Chart", 3))
    {
        mat4x4 m, im;
        vec3 e, o, d;
        vec4 q, p;
        float a;
        int deg, min, sid;

        ImGui::TableSetupColumn("Planet");
        ImGui::TableSetupColumn("Pos");
        ImGui::TableSetupColumn("Sign");
        ImGui::TableHeadersRow();

        lv_ephem_object_vec3(app, ephem_id_EarthMoon, 0, e, 1.0f);
        lv_iau2006_dynamic_matrix(app, m);
        mat4x4_invert(im, m);

        for (size_t oid = 0; oid < oid_count; oid++)
        {
            if (oid == ephem_id_EarthMoon) continue;

            lv_ephem_object_vec3(app, oid, 0, o, 1.0);
            vec3_sub(d, o, e);
            vec4_vec3_w1(q, d);
            mat4x4_mul_vec4(p, im, q);
            a = vector_angle_deg(p[0], p[1]);

            deg = (int)floorf(a);
            min = (int)floorf((a - deg) * 60.0);
            sid = deg / 30;

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%s %s", data[oid].symbol, data[oid].name);
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%3d°%02d′", deg, min);
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%s %s", signs[sid].symbol, signs[sid].name);
        }
        ImGui::EndTable();
    }
    ImGui::End();

    ImGui::Render();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
