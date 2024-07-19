#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv0;
layout(location = 3) in vec2 uv1;

layout(set = 0, binding = 0) uniform vec3 albedo;
layout(set = 0, binding = 1) uniform texture2D albedoTexture;

layout(set = 0, binding = 2) uniform float metallic;
layout(set = 0, binding = 3) uniform texture2D metallicTexture;

layout(set = 0, binding = 4) uniform float roughness;
layout(set = 0, binding = 5) uniform texture2D roughnessTexture;

layout(set = 0, binding = 6) uniform float ambientOcclusion;
layout(set = 0, binding = 7) uniform texture2D ambientOcclusionTexture;

struct attenuation_curve
{
    float c;
    float b;
    float a;
};

struct directional_light
{
    vec3 direction;
    vec4 color;
};

struct spot_light
{
    vec3 position;
    vec3 direction;
    vec4 color;
    float cutoff;
    attenuation_curve attenuation;
};

struct point_light
{
    vec3 position;
    vec4 color;
    float cutoff;
    attenuation_curve attenuation;
};

layout(set = 1, binding = 0) buffer directional_light directionalLights;
layout(set = 1, binding = 1) buffer spot_light spotLights;
layout(set = 1, binding = 2) buffer point_light pointLights;

layout(set = 2, binding = 0) buffer uint[] perTileDirectionalLightData;
layout(set = 2, binding = 1) buffer uint[] perTileDirectionalLightDataOffsets;
layout(set = 2, binding = 2) buffer uint[] perTileSpotLightData;
layout(set = 2, binding = 3) buffer uint[] perTileSpotLightDataOffsets;
layout(set = 2, binding = 4) buffer uint[] tilePointLightData;
layout(set = 2, binding = 5) buffer uint[] perTilePointLightDataOffsets;