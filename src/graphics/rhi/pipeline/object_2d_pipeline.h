#pragma once
#include "pipeline.h"

namespace ray::graphics {

enum class e_space_type : glm::u8 {
        screen = 0,
        world
};

#if RAY_GRAPHICS_ENABLE


struct object_2d_pipeline_data_model {
        struct pipeline : base_pipeline_data_model::pipeline {
                glm::vec4 camera_transform = {}; // x_px, y_px, scale, 1.0
        };

        struct draw_obj : base_pipeline_data_model::draw_obj {
                e_space_type space_basis = e_space_type::screen;
                glm::u32 z_order = 0; // bigger on top, e_space_type::screen get +16000 priority
                glm::vec4 transform = {}; // x_pos, y_pos, x_size, y_size
        };
};

template<class PipelineDataModel = object_2d_pipeline_data_model>
class object_2d_pipeline : public base_pipeline<PipelineDataModel> {
public:
        using base_pipeline<PipelineDataModel>::base_pipeline;

        virtual void draw_commands(VkCommandBuffer command_buffer) override {}
        virtual void update_swapchain(VkFormat swapchain_format) override {}
};

// RAINBOW

struct rainbow_rect_pipeline_data_model {
        struct pipeline : object_2d_pipeline_data_model::pipeline {
                glm::u64 time_ms = 0; // since startup, TODO: move to object_2d_pipeline
        };

        struct draw_obj : object_2d_pipeline_data_model::draw_obj {
        };
};

class rainbow_rect_pipeline final : public object_2d_pipeline<rainbow_rect_pipeline_data_model> {
public:
        using object_2d_pipeline::object_2d_pipeline;
};


// SOLID RECT

struct solid_rect_pipeline_data_model {
        struct pipeline : object_2d_pipeline_data_model::pipeline {
        };

        struct draw_obj : object_2d_pipeline_data_model::draw_obj {
                glm::vec4 color {};
        };
};

class solid_rect_pipeline final : public object_2d_pipeline<solid_rect_pipeline_data_model> {
public:
        using object_2d_pipeline::object_2d_pipeline;
};

// TODO: texture_pipeline, text_pipeline, raypath_pipeline
#endif
};