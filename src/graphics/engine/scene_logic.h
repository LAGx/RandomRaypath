#pragma once
#include "graphics/rhi/pipeline/object_2d_pipeline.h"
#include "graphics/rhi/pipeline/pipeline.h"
#include "graphics/rhi/pipeline/impl/rainbow_rect_pipeline.h"
#include "graphics/rhi/pipeline/impl/solid_rect_pipeline.h"
#include "graphics/rhi/pipeline/impl/glyph_pipeline.h"
#include "logical_system/logical_text_line.h"

#include <vector>


namespace ray::graphics {

#if RAY_GRAPHICS_ENABLE
class window;
class renderer;

class scene_logic {
public:
        scene_logic(window& win, renderer& rend);
        ~scene_logic();

        bool tick(window& win, renderer& rend);

        void cleanup(window& win, renderer& rend);

public:
        void tick_camera_movement(window& win, renderer& rend); // todo: move camera to logical_system

        glm::vec4 transform_dyn_3 = {};
        glm::vec4 transform_dyn_4 = {};

        std::vector<pipeline_handle<object_2d_pipeline<>>> all_pipelines;

        draw_obj_handle<rainbow_rect_pipeline> rainbow_1_screen;
        draw_obj_handle<rainbow_rect_pipeline> rainbow_2_screen;
        draw_obj_handle<solid_rect_pipeline> rect_3_dyn_world;
        draw_obj_handle<solid_rect_pipeline> rect_4_dyn_world;
        draw_obj_handle<solid_rect_pipeline> rect_5_world;

        draw_obj_handle<glyph_pipeline> text_1_handle;
        draw_obj_handle<glyph_pipeline> text_2_handle;

        glm::u64 last_time_ns = 0; // TODO: move fps and other HUD stats to logical_system
        glm::u64 last_delta_time_ns = 0;

        glm::vec4 camera_transform = {0, 0, 1, 0};
        std::optional<glm::vec2> base_move_position = std::nullopt;

        logical_text_line_manager text_line_manager;
        logical_text_line_handler new_line_1 = nullptr;
};

#endif
};