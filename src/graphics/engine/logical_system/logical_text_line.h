#pragma once
#include "graphics/rhi/pipeline/pipeline_manager.h"

namespace ray::graphics {

#if RAY_GRAPHICS_ENABLE

struct logical_text_line_args {
        std::string_view init_content_text;
        glm::u32 init_capacity = 48;
        e_space_type space_basis = e_space_type::screen;
        glm::vec4 transform = glm::vec4(0.f, 0.f, 0.f, 50.f); // x_pos_px, y_pos_px, 0, y_size_px (pivot top left)
        glm::u32 z_order = 0;
        glm::vec4 text_color = glm::vec4(1);
        glm::f32 outline_size_ndc = 0.1f;
        glm::vec4 outline_color = glm::vec4(0.5f, 1.f, 0.5f, 1.f);
        glm::vec4 background_color = glm::vec4(0);
};

class logical_text_line {
public:
        void update(std::string_view new_string) {}
};

using logical_text_line_handler = std::shared_ptr<logical_text_line>;

class logical_text_line_manager {
public:
        logical_text_line_manager() {}
        ~logical_text_line_manager() {}

        void init(const pipeline_manager& pipe, glm::u32 pipe_render_order) {}

        logical_text_line_handler create_text_line(logical_text_line_args in_args) {return nullptr;}

protected:
        std::vector<std::weak_ptr<logical_text_line>> text_lines;
};

#endif
};