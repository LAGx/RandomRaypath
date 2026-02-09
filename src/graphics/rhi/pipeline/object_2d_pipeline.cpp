#include "object_2d_pipeline.h"


using namespace ray;
using namespace ray::graphics;

#if RAY_GRAPHICS_ENABLE

/*
class pipeline {
public:
        struct vertex {
                float pos[2];
                float uv[2];
        };

        pipeline(VkFormat swapchain_format);
        ~pipeline();

        bool init_ok = false;

        void draw_commands(VkCommandBuffer command_buffer);

protected:
        glm::u32 find_memory_type(glm::u32 typeBits, VkMemoryPropertyFlags props);
        bool create_buffer(VkDeviceSize size, VkBufferUsageFlags usage,
                VkMemoryPropertyFlags props, VkBuffer& outBuf, VkDeviceMemory& outMem);
        VkShaderModule create_shader_module_from_file(const char* path);

        bool create_buffers();
        void destroy_buffers();

        VkPipelineLayout vk_pipeline_layout = VK_NULL_HANDLE;
        VkPipeline vk_pipeline = VK_NULL_HANDLE;

        VkBuffer vbuf = VK_NULL_HANDLE;
        VkDeviceMemory vmem = VK_NULL_HANDLE;
        VkBuffer ibuf = VK_NULL_HANDLE;
        VkDeviceMemory imem = VK_NULL_HANDLE;
};
 */
/*
VkShaderModule pipeline::create_shader_module_from_file(const char* path) {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file) {
                std::println("renderer: failed to open shader file: {}", path);
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
                std::println("renderer: vkCreateShaderModule failed: {}", path);
                return VK_NULL_HANDLE;
        }
        return mod;
}


pipeline::pipeline(VkFormat swapchain_format) {
        VkDevice device = g_app_driver::thread_safe().device;
        if (device == VK_NULL_HANDLE) {
                return;
        }

        VkShaderModule vert = create_shader_module_from_file("../shaders/quad.vert.spv");
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

        VkPushConstantRange pust_constant {};
        pust_constant.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        pust_constant.offset = 0;
        pust_constant.size = sizeof(float);

        VkPipelineLayoutCreateInfo pipeline_layout_cinf { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
        pipeline_layout_cinf.pushConstantRangeCount = 1;
        pipeline_layout_cinf.pPushConstantRanges = &pust_constant;

        if (vkCreatePipelineLayout(device, &pipeline_layout_cinf, nullptr, &vk_pipeline_layout) != VK_SUCCESS) {
                std::println("renderer: vkCreatePipelineLayout failed");
                return;
        }

        VkVertexInputBindingDescription vertex_input_state_bind_desc {};
        vertex_input_state_bind_desc.binding = 0;
        vertex_input_state_bind_desc.stride = sizeof(vertex);
        vertex_input_state_bind_desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        VkVertexInputAttributeDescription vertex_input_attr_desc[2] {};
        vertex_input_attr_desc[0].location = 0;
        vertex_input_attr_desc[0].binding = 0;
        vertex_input_attr_desc[0].format = VK_FORMAT_R32G32_SFLOAT;
        vertex_input_attr_desc[0].offset = offsetof(vertex, pos);

        vertex_input_attr_desc[1].location = 1;
        vertex_input_attr_desc[1].binding = 0;
        vertex_input_attr_desc[1].format = VK_FORMAT_R32G32_SFLOAT;
        vertex_input_attr_desc[1].offset = offsetof(vertex, uv);

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
        render_cinf.pColorAttachmentFormats = &swapchain_format;

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
                std::println("renderer: vkCreateGraphicsPipelines failed");
                init_ok = false;
                return;
        }

        init_ok = create_buffers();
}


pipeline::~pipeline () {
        VkDevice device = g_app_driver::thread_safe().device;
        if (device == VK_NULL_HANDLE) {
                return;
        }

        destroy_buffers();

        if (vk_pipeline != VK_NULL_HANDLE) {
                vkDestroyPipeline(device, vk_pipeline, nullptr);
                vk_pipeline = VK_NULL_HANDLE;
        }

        if (vk_pipeline_layout != VK_NULL_HANDLE) {
                vkDestroyPipelineLayout(device, vk_pipeline_layout, nullptr);
                vk_pipeline_layout = VK_NULL_HANDLE;
        }
}


glm::u32 pipeline::find_memory_type(glm::u32 typeBits, VkMemoryPropertyFlags props) {
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


bool pipeline::create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags props, VkBuffer& outBuf, VkDeviceMemory& outMem) {
        VkDevice device = g_app_driver::thread_safe().device;
        if (device == VK_NULL_HANDLE) {
                return false;
        }

        VkBufferCreateInfo bci{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        bci.size = size;
        bci.usage = usage;
        bci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(device, &bci, nullptr, &outBuf) != VK_SUCCESS) {
                std::println("renderer: vkCreateBuffer failed");
                return false;
        }

        VkMemoryRequirements mr{};
        vkGetBufferMemoryRequirements(device, outBuf, &mr);

        uint32_t memType = find_memory_type(mr.memoryTypeBits, props);
        if (memType == UINT32_MAX) {
                std::println("renderer: find_memory_type failed");
                return false;
        }

        VkMemoryAllocateInfo mai{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
        mai.allocationSize = mr.size;
        mai.memoryTypeIndex = memType;

        if (vkAllocateMemory(device, &mai, nullptr, &outMem) != VK_SUCCESS) {
                std::println("renderer: vkAllocateMemory failed");
                return false;
        }

        vkBindBufferMemory(device, outBuf, outMem, 0);

        return true;
}

bool pipeline::create_buffers() {
        VkDevice device = g_app_driver::thread_safe().device;
        if (device == VK_NULL_HANDLE) {
                return false;
        }

        static const vertex verts[] = {
                {{-0.8f, -0.6f}, {0.0f, 1.0f}},
                {{ 0.8f, -0.6f}, {1.0f, 1.0f}},
                {{ 0.8f,  0.6f}, {1.0f, 0.0f}},
                {{-0.8f,  0.6f}, {0.0f, 0.0f}},
            };
        static const uint16_t idx[] = { 0, 1, 2, 2, 3, 0 };

        const bool vert_success = create_buffer(sizeof(verts), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vbuf, vmem);
        const bool idx_success = create_buffer(sizeof(idx), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, ibuf, imem);

        if (!vert_success || !idx_success) {
                return false;
        }
        void* p = nullptr;
        vkMapMemory(device, vmem, 0, sizeof(verts), 0, &p);
        std::memcpy(p, verts, sizeof(verts));
        vkUnmapMemory(device, vmem);

        vkMapMemory(device, imem, 0, sizeof(idx), 0, &p);
        std::memcpy(p, idx, sizeof(idx));
        vkUnmapMemory(device, imem);

        return true;
}

void pipeline::destroy_buffers() {
        VkDevice device = g_app_driver::thread_safe().device;
        if (device == VK_NULL_HANDLE) {
                return;
        }

        if (vbuf) {
                vkDestroyBuffer(device, vbuf, nullptr);
        }
        if (vmem) {
                vkFreeMemory(device, vmem, nullptr);
        }
        if (ibuf) {
                vkDestroyBuffer(device, ibuf, nullptr);
        }
        if (imem) {
                vkFreeMemory(device, imem, nullptr);
        }

        vbuf = VK_NULL_HANDLE;
        vmem = VK_NULL_HANDLE;
        ibuf = VK_NULL_HANDLE;
        imem = VK_NULL_HANDLE;
}


void pipeline::draw_commands (VkCommandBuffer command_buffer) {
        vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline);

        VkDeviceSize off = 0;
        vkCmdBindVertexBuffers(command_buffer, 0, 1, &vbuf, &off);
        vkCmdBindIndexBuffer(command_buffer, ibuf, 0, VK_INDEX_TYPE_UINT16);

        // TODO: start_ticks implement in scene logic
        int start_ticks = 235312;
        float t = (float)((now_ticks_ns() - start_ticks) / 1.0e9);
        vkCmdPushConstants(command_buffer, vk_pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(float), &t);

        vkCmdDrawIndexed(command_buffer, 6, 1, 0, 0, 0);
}
*/

#endif