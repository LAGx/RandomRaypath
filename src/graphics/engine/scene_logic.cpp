#include "scene_logic.h"
#include "graphics/window/window.h"
#include "graphics/rhi/renderer.h"


static uint64_t now_ticks_ns() {
        return (uint64_t)std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::steady_clock::now().time_since_epoch()
        ).count();
}


using namespace ray;
using namespace ray::graphics;

#if RAY_GRAPHICS_ENABLE

scene_logic::scene_logic(window& win, renderer& rend) {
        pipeline_handle<rainbow_rect_pipeline> rainbow_pipeline = rend.pipe.create_pipeline<rainbow_rect_pipeline>();
        pipeline_handle<solid_rect_pipeline> rect_pipeline = rend.pipe.create_pipeline<solid_rect_pipeline>();

        if (!rainbow_pipeline.is_valid () || !rect_pipeline.is_valid ()) {
                return;
        }

        pipeline_handle<base_pipeline<>> casted_pipeline = rainbow_pipeline;

        all_pipelines.push_back(rainbow_pipeline);
        all_pipelines.push_back(rect_pipeline);

        world_pipelines.push_back(rainbow_pipeline);
        world_pipelines.push_back(rect_pipeline);

        time_pipelines.push_back(rainbow_pipeline);

        rainbow_1_screen = rend.pipe.create_draw_obj<rainbow_rect_pipeline>(rainbow_pipeline);
        rainbow_2_world = rend.pipe.create_draw_obj<rainbow_rect_pipeline>(rainbow_pipeline);
        rect_3_dyn_screen = rend.pipe.create_draw_obj<solid_rect_pipeline>(rect_pipeline);
        rect_4_dyn_world = rend.pipe.create_draw_obj<solid_rect_pipeline>(rect_pipeline);
        rect_5_world = rend.pipe.create_draw_obj<solid_rect_pipeline>(rect_pipeline);

        if (auto rainbow_1_screen_data = rend.pipe.access_draw_obj_data(rainbow_1_screen)) {
                rainbow_1_screen_data->space_basis = e_space_type::screen;
                rainbow_1_screen_data->z_order = 1;
                rainbow_1_screen_data->transform = glm::vec4(20, 100, 200, 400);
        }

        if (auto rainbow_2_world_data = rend.pipe.access_draw_obj_data(rainbow_2_world)) {
                rainbow_2_world_data->space_basis = e_space_type::world;
                rainbow_2_world_data->z_order = 2;
                rainbow_2_world_data->transform = glm::vec4(0, 0, 80, 80);
        }

        if (auto rect_3_dyn_screen_data = rend.pipe.access_draw_obj_data(rect_3_dyn_screen)) {
                rect_3_dyn_screen_data->space_basis = e_space_type::screen;
                rect_3_dyn_screen_data->z_order = 3;
                transform_dyn_3 = glm::vec4(250, 100, 200, 400);
                rect_3_dyn_screen_data->transform = transform_dyn_3;
                rect_3_dyn_screen_data->color = glm::vec4();
        }

        if (auto rect_4_dyn_world_data = rend.pipe.access_draw_obj_data(rect_4_dyn_world)) {
                rect_4_dyn_world_data->space_basis = e_space_type::world;
                rect_4_dyn_world_data->z_order = 4;
                transform_dyn_4 = glm::vec4(470, 120, 150, 350);
                rect_4_dyn_world_data->transform = transform_dyn_4;
                rect_4_dyn_world_data->color = glm::vec4();
        }

        if (auto rect_5_world_data = rend.pipe.access_draw_obj_data(rect_5_world)) {
                rect_5_world_data->space_basis = e_space_type::world;
                rect_5_world_data->z_order = 5;
                rect_5_world_data->transform = glm::vec4(100, 250, 150, 800);
                rect_5_world_data->color = glm::vec4();
        }
}

bool scene_logic::tick(window& win, renderer& rend) {
        {
                const int kek_antispeed = 100;
                delta_kek = sin(++i / kek_antispeed) * 20;
        }

        // update camera
        for (auto& world_pipeline : world_pipelines) {
                auto world_data = rend.pipe.access_pipeline_data(world_pipeline);
                if (!world_data) {
                        return false;
                }

                world_data->camera_transform = glm::vec4(); //TODO: camera logic
        }

        // update time
        for (auto& time_pipeline : time_pipelines) {
                auto time_data = rend.pipe.access_pipeline_data(time_pipeline);
                if (!time_data) {
                        return false;
                }

                time_data->time_ms = delta_kek; //TODO: proper time
        }

        if (auto rect_3_dyn_screen_data = rend.pipe.access_draw_obj_data(rect_3_dyn_screen)) {
                rect_3_dyn_screen_data->transform = transform_dyn_3 + delta_kek;
        } else {
                return false;
        }

        if (auto rect_4_dyn_world_data = rend.pipe.access_draw_obj_data(rect_4_dyn_world)) {
                rect_4_dyn_world_data->transform = transform_dyn_4 + delta_kek;
        } else {
                return false;
        }

        return true;
}


void scene_logic::cleanup(window& win, renderer& rend) {
        for (auto p : all_pipelines) {
                rend.pipe.destroy_pipeline(p);
        }
}


scene_logic::~scene_logic() {
}

#endif