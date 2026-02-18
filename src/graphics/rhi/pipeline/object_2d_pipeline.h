#pragma once
#include "pipeline.h"
#include "graphics/rhi/g_app_driver.h"

#include <cstring>
#include <fstream>
#include <print>

namespace ray::graphics {
enum class e_space_type : glm::u8 {
        screen = 0,
        world
};

#if RAY_GRAPHICS_ENABLE

struct ray_vertex {
        glm::float32 pos[2];
        glm::float32 uv[2];
};

struct object_2d_pipeline_data_model {
        struct pipeline : base_pipeline_data_model::pipeline {
                glm::u32 time_ms = 0;
                glm::vec4 camera_transform = {0,0,1,0}; // x_px, y_px, scale, 1.0
        };

        struct draw_obj : base_pipeline_data_model::draw_obj {
                e_space_type space_basis = e_space_type::screen;
                glm::u32 z_order = 0; // bigger on top, e_space_type::screen set significant bit to 1
                glm::vec4 transform = {}; // x_pos, y_pos, x_size, y_size
                glm::vec4 color {};

                glm::u32 get_render_order() const {
                        const glm::u32 space_type_bit = (space_basis == e_space_type::screen)
                                ? glm::u32(1) << (sizeof(glm::u32) * CHAR_BIT - 1)
                                : 0u;
                        return glm::u32(z_order) | space_type_bit;
                }
        };
};

// TODO: texture_pipeline, text_pipeline, raypath_pipeline
template<class PipelineDataModel = object_2d_pipeline_data_model>
class object_2d_pipeline : public base_pipeline<PipelineDataModel> {
public:
        object_2d_pipeline(std::weak_ptr<index_pool> in_index_pool, VkFormat in_swapchain_format, glm::uvec2 in_resolution);
        virtual ~object_2d_pipeline() override;

        virtual void draw_commands(VkCommandBuffer in_command_buffer, glm::u32 frame_index) override;
        virtual void update_swapchain(VkFormat in_swapchain_format, glm::uvec2 resolution) override;

protected:
        struct alignas(16) pipe_frame_ubo
        {
                glm::u32 time_ms = 0;
                glm::u32 _pad0 = 0, _pad1 = 0, _pad2 = 0;
                glm::vec4 camera_transform_ndc = glm::vec4(0,0,1,0);
        };
        static_assert(sizeof(pipe_frame_ubo) % 16 == 0);

        struct pipe_frame_ubo_gpu
        {
                VkBuffer buffer = VK_NULL_HANDLE;
                VkDeviceMemory memory = VK_NULL_HANDLE;
                void* mapped = nullptr; // pipe_frame_ubo
                bool actual_data = false;
        };

        struct render_order_entry {
                size_t index = UINT64_MAX;
                glm::u32 render_order = UINT32_MAX;
        };

protected:
        void rebuild_order();

        virtual draw_obj_handle_id add_new_draw_obj() override;
        virtual void remove_draw_obj(draw_obj_handle_id to_remove) override;

        void init_pipeline();
        void destroy_pipeline();

        void init_descriptor_sets(VkDevice device);
        void destroy_descriptor_sets(VkDevice device);

        glm::u32 find_memory_type(glm::u32 typeBits, VkMemoryPropertyFlags props);
        bool create_buffer(VkDeviceSize size, VkBufferUsageFlags usage,
                VkMemoryPropertyFlags props, VkBuffer& outBuf, VkDeviceMemory& outMem, VkDevice device);
        VkShaderModule create_shader_module_from_file(const char* path);

        void create_buffers();
        void destroy_buffers();

        void create_vertex_buffer(VkDevice device);
        void create_ubos_buffer(VkDevice device);
        void destroy_vertex_buffer(VkDevice device);
        void destroy_ubos_buffer(VkDevice device);

protected:
        bool render_order_dirty = false;
        std::vector<render_order_entry> render_order;

        VkPipelineLayout vk_pipeline_layout = VK_NULL_HANDLE;
        VkPipeline vk_pipeline = VK_NULL_HANDLE;

        VkDescriptorSetLayout vk_descriptor_set_layout = VK_NULL_HANDLE;
        VkDescriptorPool vk_descriptor_pool = VK_NULL_HANDLE;
        std::array<VkDescriptorSet, g_app_driver::k_frames_in_flight> vk_descriptor_sets {VK_NULL_HANDLE};

        // TODO: move to shared data resource block
        VkBuffer vk_vertex_buf = VK_NULL_HANDLE;
        VkDeviceMemory vk_vertex_mem = VK_NULL_HANDLE;
        VkBuffer vk_idx_buf = VK_NULL_HANDLE;
        VkDeviceMemory vk_idx_mem = VK_NULL_HANDLE;

        std::array<pipe_frame_ubo_gpu, g_app_driver::k_frames_in_flight> frame_ubos_data {};
};


template<class PipelineDataModel>
object_2d_pipeline<PipelineDataModel>::object_2d_pipeline(std::weak_ptr<index_pool> in_index_pool, VkFormat in_swapchain_format, glm::uvec2 in_resolution)
        : base_pipeline<PipelineDataModel>(in_index_pool, in_swapchain_format, in_resolution) {
        create_buffers();
        init_pipeline();
}


template<class PipelineDataModel>
object_2d_pipeline<PipelineDataModel>::~object_2d_pipeline() {
        destroy_pipeline();
        destroy_buffers();
}


template<class PipelineDataModel>
void object_2d_pipeline<PipelineDataModel>::draw_commands(VkCommandBuffer in_command_buffer, glm::u32 frame_index) {
#ifdef RAY_DEBUG_NO_OPT
        assert(frame_index < g_app_driver::k_frames_in_flight);
#endif

        if (render_order_dirty) {
                rebuild_order();
                render_order_dirty = false;
        }

        vkCmdBindPipeline(in_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline);

        VkDeviceSize off = 0;
        vkCmdBindVertexBuffers(in_command_buffer, 0, 1, &vk_vertex_buf, &off);
        vkCmdBindIndexBuffer(in_command_buffer, vk_idx_buf, 0, VK_INDEX_TYPE_UINT16);

        if (this->pipe_data.need_update || !frame_ubos_data[frame_index].actual_data) {
                for (int i = 0; i < frame_ubos_data.size(); ++i) {
                        frame_ubos_data[i].actual_data = false;
                }

                frame_ubos_data[frame_index].actual_data = true;
                this->pipe_data.need_update = false;

                pipe_frame_ubo* ubo_low_obj = reinterpret_cast<pipe_frame_ubo*>(frame_ubos_data[frame_index].mapped);
                ubo_low_obj->time_ms = this->pipe_data.time_ms;
                ubo_low_obj->camera_transform_ndc = this->pipe_data.camera_transform;
                ubo_low_obj->camera_transform_ndc.x /= this->resolution.x;
                ubo_low_obj->camera_transform_ndc.y /= this->resolution.y;
        }

        vkCmdBindDescriptorSets(
            in_command_buffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            vk_pipeline_layout,
            0,
            1,
            &vk_descriptor_sets[frame_index],
            0,
            nullptr
        );

        for (size_t i = 0; i < render_order.size(); ++i) {
                auto& z_indexing = render_order[i];
                const size_t draw_index = render_order[i].index;
                auto& draw_data = this->draw_obj_data[draw_index];

                render_order_dirty |= draw_data.get_render_order() != z_indexing.render_order;

                // TODO: draw here
        }


        // vkCmdDrawIndexed(in_command_buffer, 6, 1, 0, 0, 0); // instanceCount will iterate by gl_InstanceIndex inside shader
}


template<class PipelineDataModel>
void object_2d_pipeline<PipelineDataModel>::update_swapchain(VkFormat in_swapchain_format, glm::uvec2 in_resolution) {
        destroy_pipeline();
        this->swapchain_format = in_swapchain_format;
        this->resolution = in_resolution;
        init_pipeline();
}


template<class PipelineDataModel>
void object_2d_pipeline<PipelineDataModel>::rebuild_order() {
        render_order.clear();
        render_order.reserve(this->draw_obj_data.size());

        for (size_t i = 0; i < this->draw_obj_data.size(); ++i) {
                auto& obj_data = this->draw_obj_data[i];
                render_order.push_back({ i, obj_data.get_render_order() });
        }

        std::stable_sort(render_order.begin(), render_order.end(),
            [](const render_order_entry& a, const render_order_entry& b) {
                    return a.render_order < b.render_order;
            });
}


template<class PipelineDataModel>
draw_obj_handle_id object_2d_pipeline<PipelineDataModel>::add_new_draw_obj() {
        render_order_dirty = true;
        return base_pipeline<PipelineDataModel>::add_new_draw_obj();
}


template<class PipelineDataModel>
void object_2d_pipeline<PipelineDataModel>::remove_draw_obj(draw_obj_handle_id to_remove) {
        render_order_dirty = true;
        base_pipeline<PipelineDataModel>::remove_draw_obj(to_remove);
}


template<class PipelineDataModel>
void object_2d_pipeline<PipelineDataModel>::init_pipeline() {
        if (this->swapchain_format == VK_FORMAT_UNDEFINED) { // It is ok, maybe will be updated later.
                return;
        }

        if (vk_pipeline_layout != VK_NULL_HANDLE || vk_pipeline != VK_NULL_HANDLE) {
                std::println("object_2d_pipeline: can't init. vk_pipeline_layout or vk_pipeline init already.");
                return;
        }

        VkDevice device = g_app_driver::thread_safe().device;
        if (device == VK_NULL_HANDLE) {
                return;
        }

        init_descriptor_sets(device);

        //VkPushConstantRange pust_constant {};
        //pust_constant.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        //pust_constant.offset = 0;
        //pust_constant.size = sizeof(float); // max 128 for sure

        VkPipelineLayoutCreateInfo pipeline_layout_cinf { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
        //pipeline_layout_cinf.pushConstantRangeCount = 1;
        //pipeline_layout_cinf.pPushConstantRanges = &pust_constant;
        pipeline_layout_cinf.setLayoutCount = 1;
        pipeline_layout_cinf.pSetLayouts = &vk_descriptor_set_layout;

        if (vkCreatePipelineLayout(device, &pipeline_layout_cinf, nullptr, &vk_pipeline_layout) != VK_SUCCESS) {
                std::println("object_2d_pipeline: vkCreatePipelineLayout failed");
                return;
        }

        VkShaderModule vert = create_shader_module_from_file("../shaders/camera.vert.spv");
        VkShaderModule frag = create_shader_module_from_file("../shaders/rainbow.frag.spv");

        if (!vert || !frag) {
                return;
        }

        VkPipelineShaderStageCreateInfo pipeline_stages[2]{};
        pipeline_stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        pipeline_stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        pipeline_stages[0].module = vert;
        pipeline_stages[0].pName = "main";

        pipeline_stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        pipeline_stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        pipeline_stages[1].module = frag;
        pipeline_stages[1].pName = "main";

        VkVertexInputBindingDescription vertex_input_state_bind_desc {};
        vertex_input_state_bind_desc.binding = 0;
        vertex_input_state_bind_desc.stride = sizeof(ray_vertex);
        vertex_input_state_bind_desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        VkVertexInputAttributeDescription vertex_input_attr_desc[2] {};
        vertex_input_attr_desc[0].location = 0;
        vertex_input_attr_desc[0].binding = 0;
        vertex_input_attr_desc[0].format = VK_FORMAT_R32G32_SFLOAT;
        vertex_input_attr_desc[0].offset = offsetof(ray_vertex, pos);

        vertex_input_attr_desc[1].location = 1;
        vertex_input_attr_desc[1].binding = 0;
        vertex_input_attr_desc[1].format = VK_FORMAT_R32G32_SFLOAT;
        vertex_input_attr_desc[1].offset = offsetof(ray_vertex, uv);

        VkPipelineVertexInputStateCreateInfo vertex_input_state_cinf { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
        vertex_input_state_cinf.vertexBindingDescriptionCount = 1;
        vertex_input_state_cinf.pVertexBindingDescriptions = &vertex_input_state_bind_desc;
        vertex_input_state_cinf.vertexAttributeDescriptionCount = 2;
        vertex_input_state_cinf.pVertexAttributeDescriptions = vertex_input_attr_desc;

        VkPipelineInputAssemblyStateCreateInfo input_assembly_state_cinf { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
        input_assembly_state_cinf.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

        VkPipelineViewportStateCreateInfo viewport_state_cinf { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
        viewport_state_cinf.viewportCount = 1;
        viewport_state_cinf.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo rasterization_state_cinf { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
        rasterization_state_cinf.polygonMode = VK_POLYGON_MODE_FILL;
        rasterization_state_cinf.cullMode = VK_CULL_MODE_NONE;
        rasterization_state_cinf.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterization_state_cinf.depthClampEnable = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisample_state_cinf { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
        multisample_state_cinf.sampleShadingEnable = VK_FALSE;
        multisample_state_cinf.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState color_blend_attch_state {};
        color_blend_attch_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

        VkPipelineColorBlendStateCreateInfo color_blend_state_cinf { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
        color_blend_state_cinf.logicOpEnable = VK_FALSE;
        color_blend_state_cinf.attachmentCount = 1;
        color_blend_state_cinf.pAttachments = &color_blend_attch_state;

        VkDynamicState dynamic_states[2] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
        VkPipelineDynamicStateCreateInfo dynamic_state_cinf { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
        dynamic_state_cinf.dynamicStateCount = 2;
        dynamic_state_cinf.pDynamicStates = dynamic_states;

        VkPipelineRenderingCreateInfo render_cinf { VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
        render_cinf.colorAttachmentCount = 1;
        render_cinf.pColorAttachmentFormats = &this->swapchain_format;

        VkGraphicsPipelineCreateInfo pipeline_create_info { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
        pipeline_create_info.pNext = &render_cinf;
        pipeline_create_info.stageCount = 2;
        pipeline_create_info.pStages = pipeline_stages;
        pipeline_create_info.pVertexInputState = &vertex_input_state_cinf;
        pipeline_create_info.pInputAssemblyState = &input_assembly_state_cinf;
        pipeline_create_info.pViewportState = &viewport_state_cinf;
        pipeline_create_info.pRasterizationState = &rasterization_state_cinf;
        pipeline_create_info.pMultisampleState = &multisample_state_cinf;
        pipeline_create_info.pColorBlendState = &color_blend_state_cinf;
        pipeline_create_info.pDynamicState = &dynamic_state_cinf;
        pipeline_create_info.renderPass = VK_NULL_HANDLE;
        pipeline_create_info.subpass = 0;
        pipeline_create_info.layout = vk_pipeline_layout;

        const auto result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_create_info, nullptr, &vk_pipeline);

        vkDestroyShaderModule(device, vert, nullptr);
        vkDestroyShaderModule(device, frag, nullptr);

        if (result != VK_SUCCESS) {
                std::println("object_2d_pipeline: vkCreateGraphicsPipelines failed.");
                return;
        }
}


template<class PipelineDataModel>
void object_2d_pipeline<PipelineDataModel>::destroy_pipeline() {
        VkDevice device = g_app_driver::thread_safe().device;
        if (device == VK_NULL_HANDLE) {
                return;
        }

        destroy_descriptor_sets(device);

        if (vk_pipeline != VK_NULL_HANDLE) {
                vkDestroyPipeline(device, vk_pipeline, nullptr);
                vk_pipeline = VK_NULL_HANDLE;
        }

        if (vk_pipeline_layout != VK_NULL_HANDLE) {
                vkDestroyPipelineLayout(device, vk_pipeline_layout, nullptr);
                vk_pipeline_layout = VK_NULL_HANDLE;
        }
}


template<class PipelineDataModel>
void object_2d_pipeline<PipelineDataModel>::init_descriptor_sets(VkDevice device) {
        VkDescriptorSetLayoutBinding ubo_binding{};
        ubo_binding.binding = 0;
        ubo_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        ubo_binding.descriptorCount = 1;
        ubo_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutCreateInfo layout_info{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
        layout_info.bindingCount = 1;
        layout_info.pBindings = &ubo_binding;

        VkResult desc_layout_ok = vkCreateDescriptorSetLayout(device, &layout_info, nullptr, &vk_descriptor_set_layout);

        if (desc_layout_ok != VK_SUCCESS) {
                std::println("object_2d_pipeline: vkCreateDescriptorSetLayout failed.");
                return;
        }

        //

        VkDescriptorPoolSize pool_size{};
        pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        pool_size.descriptorCount = g_app_driver::k_frames_in_flight;

        VkDescriptorPoolCreateInfo pool_info{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = g_app_driver::k_frames_in_flight;
        pool_info.poolSizeCount = 1;
        pool_info.pPoolSizes = &pool_size;

        VkResult desc_pool_ok = vkCreateDescriptorPool(device, &pool_info, nullptr, &vk_descriptor_pool);

        if (desc_pool_ok != VK_SUCCESS) {
                std::println("object_2d_pipeline: vkCreateDescriptorPool failed.");
                return;
        }

        std::vector<VkDescriptorSetLayout> layouts(g_app_driver::k_frames_in_flight, vk_descriptor_set_layout);
        VkDescriptorSetAllocateInfo alloc_info{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
        alloc_info.descriptorPool = vk_descriptor_pool;
        alloc_info.descriptorSetCount = g_app_driver::k_frames_in_flight;
        alloc_info.pSetLayouts = layouts.data();

        VkResult desc_alloc_ok = vkAllocateDescriptorSets(device, &alloc_info, vk_descriptor_sets.data());

        if (desc_alloc_ok != VK_SUCCESS) {
                std::println("object_2d_pipeline: vkAllocateDescriptorSets failed.");
                return;
        }

        for (uint32_t i = 0; i < vk_descriptor_sets.size(); i++)
        {
                VkDescriptorBufferInfo buf_info{};
                buf_info.buffer = frame_ubos_data[i].buffer;
                buf_info.offset = 0;
                buf_info.range  = sizeof(pipe_frame_ubo);

                VkWriteDescriptorSet write{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
                write.dstSet = vk_descriptor_sets[i];
                write.dstBinding = 0;
                write.dstArrayElement = 0;
                write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                write.descriptorCount = 1;
                write.pBufferInfo = &buf_info;

                vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
        }
}


template<class PipelineDataModel>
void object_2d_pipeline<PipelineDataModel>::destroy_descriptor_sets(VkDevice device) {
        if (vk_descriptor_pool != VK_NULL_HANDLE) {
                vkFreeDescriptorSets(device, vk_descriptor_pool, g_app_driver::k_frames_in_flight, vk_descriptor_sets.data());
        }

        if (vk_descriptor_set_layout != VK_NULL_HANDLE) {
                vkDestroyDescriptorSetLayout(device, vk_descriptor_set_layout, nullptr);
        }

        if (vk_descriptor_pool != VK_NULL_HANDLE) {
                vkDestroyDescriptorPool(device, vk_descriptor_pool, nullptr);
        }

        vk_descriptor_pool = VK_NULL_HANDLE;
        vk_descriptor_set_layout = VK_NULL_HANDLE;
        vk_descriptor_sets.fill(VK_NULL_HANDLE);
}


template<class PipelineDataModel>
glm::u32 object_2d_pipeline<PipelineDataModel>::find_memory_type(glm::u32 typeBits, VkMemoryPropertyFlags props) {
        VkPhysicalDevice physical = g_app_driver::thread_safe().physical;
        if (physical == VK_NULL_HANDLE) {
                return UINT32_MAX;
        }

        VkPhysicalDeviceMemoryProperties mp{};
        vkGetPhysicalDeviceMemoryProperties(physical, &mp);

        for (glm::u32 i = 0; i < mp.memoryTypeCount; ++i) {
                if ((typeBits & (1u << i)) && ((mp.memoryTypes[i].propertyFlags & props) == props)) {
                        return i;
                }
        }

        return UINT32_MAX;
}


template<class PipelineDataModel>
bool object_2d_pipeline<PipelineDataModel>::create_buffer(
        VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags props, VkBuffer& outBuf, VkDeviceMemory& outMem, VkDevice device) {

        VkBufferCreateInfo bci{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        bci.size = size;
        bci.usage = usage;
        bci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(device, &bci, nullptr, &outBuf) != VK_SUCCESS) {
                std::println("object_2d_pipeline: vkCreateBuffer failed");
                return false;
        }

        VkMemoryRequirements mr{};
        vkGetBufferMemoryRequirements(device, outBuf, &mr);

        uint32_t memType = find_memory_type(mr.memoryTypeBits, props);
        if (memType == UINT32_MAX) {
                std::println("object_2d_pipeline: find_memory_type failed");
                return false;
        }

        VkMemoryAllocateInfo mai{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
        mai.allocationSize = mr.size;
        mai.memoryTypeIndex = memType;

        if (vkAllocateMemory(device, &mai, nullptr, &outMem) != VK_SUCCESS) {
                std::println("object_2d_pipeline: vkAllocateMemory failed");
                return false;
        }

        vkBindBufferMemory(device, outBuf, outMem, 0);

        return true;
}


template<class PipelineDataModel>
VkShaderModule object_2d_pipeline<PipelineDataModel>::create_shader_module_from_file(const char *path) {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file) {
                std::println("object_2d_pipeline: failed to open shader file: {}", path);
                return VK_NULL_HANDLE;
        }

        size_t size = (size_t)file.tellg();
        file.seekg(0);

        std::vector<uint32_t> code((size + 3) / 4);
        file.read((char*)code.data(), (std::streamsize)size);

        VkShaderModuleCreateInfo smci{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
        smci.codeSize = code.size() * sizeof(uint32_t);
        smci.pCode = code.data();

        VkDevice device = g_app_driver::thread_safe().device;
        if (device == VK_NULL_HANDLE) {
                return VK_NULL_HANDLE;
        }

        VkShaderModule mod = VK_NULL_HANDLE;
        if (vkCreateShaderModule(device, &smci, nullptr, &mod) != VK_SUCCESS) {
                std::println("object_2d_pipeline: vkCreateShaderModule failed: {}", path);
                return VK_NULL_HANDLE;
        }

        return mod;
}


template<class PipelineDataModel>
void object_2d_pipeline<PipelineDataModel>::create_buffers() {
        VkDevice device = g_app_driver::thread_safe().device;
        if (device == VK_NULL_HANDLE) {
                return;
        }

        create_vertex_buffer(device);
        create_ubos_buffer(device);
}


template<class PipelineDataModel>
void object_2d_pipeline<PipelineDataModel>::destroy_buffers() {
        VkDevice device = g_app_driver::thread_safe().device;
        if (device == VK_NULL_HANDLE) {
                return;
        }

        destroy_ubos_buffer(device);
        destroy_vertex_buffer(device);
}


template<class PipelineDataModel>
void object_2d_pipeline<PipelineDataModel>::create_vertex_buffer(VkDevice device) {
        static const ray_vertex verts[] = {
                {{-1.f, -1.f}, {0.f, 1.f}},
                {{1.f, -1.f}, {1.f, 1.f}},
                {{1.f, 1.f}, {1.f, 0.f}},
                {{-1.f, 1.f}, {0.f, 0.f}},
            };
        static const uint16_t idx[] = { 0, 1, 2, 2, 3, 0 };

        VkDeviceSize verts_size = sizeof(verts);
        VkDeviceSize idx_size = sizeof(idx);

        const bool vert_success = create_buffer(verts_size,
                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                vk_vertex_buf, vk_vertex_mem, device);
        const bool idx_success = create_buffer(idx_size,
                VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                vk_idx_buf, vk_idx_mem, device);

        if (!vert_success || !idx_success) {
                std::println("object_2d_pipeline: create_buffers failed. vert_success {}, idx_success {}", vert_success, idx_success);
                return;
        }

        void* p = nullptr;
        vkMapMemory(device, vk_vertex_mem, 0, verts_size, 0, &p); // means this buffer is writable for cpu
        std::memcpy(p, verts, verts_size); // might be not copy, but direct compute on p memoty
        vkUnmapMemory(device, vk_vertex_mem);

        vkMapMemory(device, vk_idx_mem, 0, idx_size, 0, &p);
        std::memcpy(p, idx, idx_size);
        vkUnmapMemory(device, vk_idx_mem);
}


template<class PipelineDataModel>
void object_2d_pipeline<PipelineDataModel>::create_ubos_buffer(VkDevice device) {

        VkDeviceSize obj_size = sizeof(pipe_frame_ubo);

        for (uint32_t i = 0; i < frame_ubos_data.size(); i++)
        {
                const bool buff_ok = create_buffer(
                    obj_size,
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                    frame_ubos_data[i].buffer,
                    frame_ubos_data[i].memory,
                    device
                );

                if (!buff_ok) {
                        std::printf("object_2d_pipeline::create_ubos_buffer failed to create buffer.");
                        break;
                }

                VkResult memory_ok = vkMapMemory(device, frame_ubos_data[i].memory, 0, obj_size, 0, &frame_ubos_data[i].mapped);

                if (memory_ok != VK_SUCCESS) {
                        std::printf("object_2d_pipeline::create_ubos_buffer failed to map memory.");
                        break;
                }
        }
}


template<class PipelineDataModel>
void object_2d_pipeline<PipelineDataModel>::destroy_vertex_buffer(VkDevice device){
        if (vk_vertex_buf) {
                vkDestroyBuffer(device, vk_vertex_buf, nullptr);
                vk_vertex_buf = VK_NULL_HANDLE;
        }
        if (vk_vertex_mem) {
                vkFreeMemory(device, vk_vertex_mem, nullptr);
                vk_vertex_mem = VK_NULL_HANDLE;
        }
        if (vk_idx_buf) {
                vkDestroyBuffer(device, vk_idx_buf, nullptr);
                vk_idx_buf = VK_NULL_HANDLE;
        }
        if (vk_idx_mem) {
                vkFreeMemory(device, vk_idx_mem, nullptr);
                vk_idx_mem = VK_NULL_HANDLE;
        }
}


template<class PipelineDataModel>
void object_2d_pipeline<PipelineDataModel>::destroy_ubos_buffer(VkDevice device) {
        for (uint32_t i = 0; i < frame_ubos_data.size(); i++) {
                if (frame_ubos_data[i].mapped) {
                        vkUnmapMemory(device, frame_ubos_data[i].memory);
                        frame_ubos_data[i].mapped = nullptr;
                }

                if (frame_ubos_data[i].buffer) {
                        vkDestroyBuffer(device, frame_ubos_data[i].buffer, nullptr);
                        frame_ubos_data[i].buffer = VK_NULL_HANDLE;
                }

                if (frame_ubos_data[i].memory) {
                        vkFreeMemory(device, frame_ubos_data[i].memory, nullptr);
                        frame_ubos_data[i].memory = VK_NULL_HANDLE;
                }

                frame_ubos_data[i].mapped = nullptr;
        }
}

#endif
};
