#version 450

layout(std430, set = 0, binding = 0) readonly buffer input_buffer_a_t{
    float input_a[];
};
layout(std430, set = 0, binding = 1) readonly buffer input_buffer_b_t{
    float input_b[];
};
layout(std430, set = 0, binding = 2) readonly buffer input_buffer_c_t{
    float input_c[];
};

layout(std430, set = 1, binding = 0) writeonly buffer output_buffer_t{
    float result[];
};

layout(local_size_x_id = 0) in;
void main()
{
    uint index = gl_GlobalInvocationID.x;
    result[index] = input_a[index] * input_b[index] + input_c[index];
}
