#pragma once
#include <memory>
#include <thread>


namespace ray::graphics {
/*
class async_graphical_loop {
public:
        async_graphical_loop();
        ~async_graphical_loop();

        void signal_terminate();
        void wait_blocking();

        async_graphical_loop(const async_graphical_loop&) = delete;
        async_graphical_loop& operator=(const async_graphical_loop&) = delete;

        async_graphical_loop(async_graphical_loop&&) noexcept = default;
        async_graphical_loop& operator=(async_graphical_loop&&) noexcept = default;

private:
        struct state {
                void init();
                void blocking_work();

                std::shared_ptr<renderer> renderer_instance; // ownner
                std::shared_ptr<window> window_instance; // ownner
        };

        std::shared_ptr<state> st;
        std::thread worker_t;
};*/
};

