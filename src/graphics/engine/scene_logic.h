#pragma once
#include "graphics/rhi/pipeline/object_2d_pipeline.h"
#include "graphics/rhi/pipeline/pipeline.h"

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
        glm::vec4 transform_dyn_3 = {};
        glm::vec4 transform_dyn_4 = {};
        float delta_kek = 0;
        float i = 0;

        std::vector<pipeline_handle<base_pipeline<>>> all_pipelines;
        std::vector<pipeline_handle<object_2d_pipeline<>>> world_pipelines;

        std::vector<pipeline_handle<rainbow_rect_pipeline>> time_pipelines;

        draw_obj_handle<rainbow_rect_pipeline> rainbow_1_screen;
        draw_obj_handle<rainbow_rect_pipeline> rainbow_2_world;
        draw_obj_handle<solid_rect_pipeline> rect_3_dyn_screen;
        draw_obj_handle<solid_rect_pipeline> rect_4_dyn_world;
        draw_obj_handle<solid_rect_pipeline> rect_5_world;
};

#endif
};