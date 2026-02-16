#version 450
layout(location = 0) in vec2 inPos;
layout(location = 1) in vec2 inUV;

layout(location = 0) out vec2 vUV;

layout(set = 0, binding = 0) uniform pipe_frame_ubo
{
    int time_ms;
    int _pad0;
    int _pad1;
    int _pad2;
    vec4 camera_transform_ndc; // x_px, y_px, scale, 1.0
} pipe_frame;


void main() {
    vUV = inUV;

    vec2 p = inPos;
    p = (p - pipe_frame.camera_transform_ndc.xy) * pipe_frame.camera_transform_ndc.z;
    gl_Position = vec4(p, 0.0, 1.0);
}