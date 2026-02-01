#include <filesystem>
#include <print>

import ray.config;

int main() {
        auto config1 = ray::config::client_renderer::load(std::filesystem::path {"../config/client_renderer.toml"});
        auto config2 = ray::config::client_renderer::load(std::filesystem::path {"../config/client_renderer2.toml"});
        auto config3 = ray::config::client_renderer::load(std::filesystem::path {"../config/client_renderer3.toml"});

        std::println("Config 1: \n{}\n---------", config1);
        std::println("Config 2: \n{}\n---------", config2);
        std::println("Config 3: \n{}\n---------", config3);

        return 0;
}