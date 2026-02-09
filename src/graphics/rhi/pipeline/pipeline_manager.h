#pragma once
#include "object_2d_pipeline.h"
#include "pipeline.h"
#include "graphics/graphic_libs.h"
#include "utils/index_pool.h"

#include <memory>


namespace ray::graphics {

#if RAY_GRAPHICS_ENABLE

class pipeline_manager {
public:
        void renderer_set_swapchain_format(VkFormat in_swapchain_format); //TODO: notify pipelines, need for window resize
        void renderer_perform_draw(VkCommandBuffer command_buffer) {}
        void renderer_shutdown() {}

        template<class Pipeline>
        pipeline_handle<Pipeline> create_pipeline() {
                auto pipe = std::make_shared<Pipeline>(draw_object_index_pool, swapchain_format);

                pipe_instances.emplace_back(std::move(pipe));

                auto handle = pipeline_handle<Pipeline>();
                handle.obj_ptr = pipe;
                return handle;
        }

        void destroy_pipeline(pipeline_handle<> pipe_id) {
                auto pipe_ptr = pipe_id.obj_ptr.lock();
                if (!pipe_ptr) {
                        return;
                }

                auto found_it = std::find_if(pipe_instances.begin(), pipe_instances.end(),
                        [&pipe_ptr](const std::shared_ptr<i_pipeline>& p) { return p == pipe_ptr; });

                if (found_it != pipe_instances.end()) {
                        pipe_instances.erase(found_it);
                }
        }

        template<class Pipeline>
        draw_obj_handle<Pipeline> create_draw_obj(pipeline_handle<Pipeline> pipe_id) {
                auto pipe_ptr = pipe_id.obj_ptr.lock();
                if (!pipe_ptr) {
                        return draw_obj_handle<Pipeline>();
                }

                draw_obj_handle_id obj_handle_id = pipe_ptr->create_draw_obj();
                auto obj_handler = draw_obj_handle<Pipeline>();
                obj_handler.pipe_handle.obj_ptr = pipe_id.obj_ptr;
                obj_handler.obj_index = obj_handle_id;
                return obj_handler;
        }

        void destroy_draw_obj(draw_obj_handle<> obj_id) {
                auto pipe_ptr = obj_id.pipe_handle.obj_ptr.lock();
                if (!pipe_ptr) {
                        return;
                }

                pipe_ptr->destroy_draw_obj(obj_id.obj_index);
        }

        template<class Pipeline>
        Pipeline::pipeline_data_model_t* access_pipeline_data(pipeline_handle<Pipeline> pipe_id, bool set_need_update = true) const {
                auto pipe_ptr = pipe_id.obj_ptr.lock();
                if (!pipe_ptr) {
                        return nullptr;
                }

                auto model_ptr = pipe_ptr->template get_pipeline_model<Pipeline>();
                if (set_need_update && model_ptr) {
                        model_ptr->need_update = true;
                }

                return model_ptr;
        }

        template<class Pipeline>
        Pipeline::draw_obj_model_t* access_draw_obj_data(draw_obj_handle<Pipeline> draw_id, bool set_need_update = true) const {
                auto pipe_ptr = draw_id.pipe_handle.obj_ptr.lock();
                if (!pipe_ptr) {
                        return nullptr;
                }

                auto model_ptr = pipe_ptr->template get_draw_model<Pipeline>(draw_id.obj_index);

                if (set_need_update && model_ptr) {
                        model_ptr->need_update = true;
                }

                return model_ptr;
        }

private:
        std::vector<std::shared_ptr<i_pipeline>> pipe_instances; // single owner
        std::shared_ptr<index_pool> draw_object_index_pool = std::make_shared<index_pool>();

        VkFormat swapchain_format = VK_FORMAT_UNDEFINED;
};
#endif
};