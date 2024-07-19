#version 450

layout(constant_id = 0) const uint tileSizeX = 16;
layout(constant_id = 1) const uint tileSizeY = 16;

layout(constant_id = 4) const uint maxLightsPerTile = 32;

struct attenuation_curve
{
    float c;
    float b;
    float a;
};

struct spot_light
{
    vec3 position;
    vec3 direction;
    vec4 color;
    float cutoff;
    attenuation_curve attenuation;
};

layout(std430, set = 0, binding = 0) readonly buffer spot_lights{
    spot_light spotLights[];
} spotLights;

shared uint tileSpotLights[maxLightsPerTile];
shared uint tileSpotLightIndex;
shared uint tileSpotLightsCount;

shared uint tileWriteOffset;

layout(set = 1, binding = 0) buffer spot_light_total_count{
    uint totalCount;
} spotLightTotalCount;
layout(std430, set = 1, binding = 1) writeonly buffer spot_light_tile_offsets{
    uint tileOffsets[];
} spotLightTileOffsets;
layout(std430, set = 1, binding = 2) writeonly buffer spot_light_indices{
    uint indices[];
} spotLightIndices;

struct frustrum
{
    int v;
};

frustrum makeTileFrustrum(uint tileIndex)
{
    return frustrum(0);
}

bool isSpotLightVisible(uint spotLightIndex, frustrum tileFrustrum)
{
    return true;
}

layout(local_size_x_id = 3, local_size_y = 1, local_size_z = 1) in;
void main()
{
    tileSpotLightIndex = 0;
    tileSpotLightsCount = 0;

    memoryBarrierShared();

    uint tileIndex = gl_WorkGroupID.y * gl_WorkGroupSize.x + gl_WorkGroupID.x;
    uint localIndex = gl_LocalInvocationID.x;
    uint localSize = gl_WorkGroupSize.x;
    uint globalIndex = gl_GlobalInvocationID.x;

    uint spotLightCount = spotLights.spotLights.length();
    frustrum tileFrustrum = makeTileFrustrum(tileIndex);

    for(uint spotLightIndex = localIndex; spotLightIndex < spotLightCount; spotLightIndex += localSize)
    {
        if(isSpotLightVisible(spotLightIndex, tileFrustrum))
        {
            uint insertionIndex = atomicAdd(tileSpotLightIndex, 1);

            if(insertionIndex >= maxLightsPerTile)
                break;

            tileSpotLights[insertionIndex] = spotLightIndex;
            atomicAdd(tileSpotLightsCount, 1);
        }
    }

    memoryBarrierShared();
    if(localIndex == 0)
    {
        // reserve size to spotLightTileIndices SSBO
        tileWriteOffset = atomicAdd(spotLightTotalCount.totalCount, tileSpotLightsCount);
        // save it to spotLightTileOffsets
        spotLightTileOffsets.tileOffsets[tileIndex] = tileWriteOffset;
    }
    memoryBarrierShared();

    // copy shared data to global spotLightTileIndices SSBO
    for(uint sharedLightIndex = localIndex; sharedLightIndex < tileSpotLightsCount; sharedLightIndex += localSize)
    {
        spotLightIndices.indices[tileWriteOffset + sharedLightIndex] = tileSpotLights[sharedLightIndex];
    }
}