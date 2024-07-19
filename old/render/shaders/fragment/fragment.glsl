#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 uv;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform texture2D _texture;
layout(binding = 1) uniform sampler _sampler;

void main()
{
    outColor = texture(sampler2D(_texture, _sampler), uv);//texture(texSampler, uv);
}