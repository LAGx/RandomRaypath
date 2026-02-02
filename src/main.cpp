#include <filesystem>
#include <print>

import ray.config;
import ray.graphics.window;

int main() {
        auto config_res = ray::config::client_renderer::load(std::filesystem::path {"../config/client_renderer.toml"});

        if (!config_res) {
                std::println("Failed to load config file {}", config_res.error());
                return 1;
        }

#if RAY_GRAPHICS_ENABLE
        ray::graphics::window window(config_res->window);

        window.blocking_loop();
#endif

        return 0;
}