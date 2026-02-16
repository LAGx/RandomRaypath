#pragma once
#include "../graphic_libs.h"
#include "graphics/rhi/renderer.h"

#include "glm/glm.hpp"

#include <memory>

namespace ray::graphics {

enum class e_window_mode : glm::i8 {
        none = 0,
        windowed = none,
        fullscreen = 1,
        count
};

class window {
public:
        struct config {
                bool graphics_window_enabled;
                e_window_mode window_mode; // 0 - windowed, 1 - fullscreen
                glm::i32vec2 window_position;
                glm::i32vec2 window_size;
                glm::f32 zoom_speed;
        };

        enum class e_mouse_cursor : glm::i8 {
                arrow_pointer = 0,
                move_hand = 1,
                count
        };

#if RAY_GRAPHICS_ENABLE
public:
        window(const config& in_config);
        ~window();

        window(const window&) = delete;
        window& operator=(const window&) = delete;
        window(window&&) noexcept = default;
        window& operator=(window&&) noexcept = default;

        bool draw_window(bool& out_valid_view);

        [[nodiscard]]
        std::weak_ptr<GLFWwindow> get_gl_window() const;

        // mouse
        glm::vec2 get_mouse_position() const;
        bool get_mouse_button_left() const;
        bool get_mouse_button_right() const;
        glm::f64 get_mouse_wheel_delta() const; // per window frame

        void set_mouse_cursor(e_mouse_cursor in_type);

private:
        static void glfw_mouse_scroll_callback(GLFWwindow* glfw_win, double x_offset, double y_offset);
        void glfw_alloc_cursors();
        void glfw_dealloc_cursors();

private:
        config used_config;
        std::shared_ptr<GLFWwindow> gl_win;

        glm::f64 mouse_wheel_delta_frame;
        std::array<GLFWcursor*, (int)e_mouse_cursor::count> glfw_hot_cursors;
#endif
};
};