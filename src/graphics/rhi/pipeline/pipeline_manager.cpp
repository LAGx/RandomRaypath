#include "pipeline_manager.h"
#include "graphics/graphic_libs.h"

#include <memory>

using namespace ray;
using namespace ray::graphics;

#if RAY_GRAPHICS_ENABLE

void pipeline_manager::renderer_set_swapchain_format(VkFormat in_swapchain_format) {
        swapchain_format = std::move(in_swapchain_format);
}

/*
void pipeline_manager::perform_draw(VkCommandBuffer command_buffer) {
        if (!pipeline_obj) {
                return;
        }

        pipeline_obj->draw_commands(command_buffer);
}

bool pipeline_manager::create_pipeline(VkFormat swapchain_format) {
        pipeline_obj = std::make_unique<pipeline>(swapchain_format);

        if (!pipeline_obj) {
                return false;
        }

        if (!pipeline_obj->init_ok) {
                pipeline_obj.reset();
                return false;
        }

        return true;
}


void pipeline_manager::destroy_pipeline () {
        pipeline_obj.reset();
}*/

#endif