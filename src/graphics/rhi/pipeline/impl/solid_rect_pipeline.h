#pragma once
#include "../object_2d_pipeline.h"


namespace ray::graphics {

#if RAY_GRAPHICS_ENABLE


struct solid_rect_pipeline_data_model {
        struct pipeline : object_2d_pipeline_data_model::pipeline {
        };

        struct draw_obj : object_2d_pipeline_data_model::draw_obj {
               // glm::vec4 color {};
        };
};

class solid_rect_pipeline final : public object_2d_pipeline<solid_rect_pipeline_data_model> {
public:
        using object_2d_pipeline::object_2d_pipeline;
};

#endif
};
