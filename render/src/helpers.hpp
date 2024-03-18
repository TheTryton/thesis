#pragma once

#include <vulkan/vulkan_raii.hpp>

#include <set>
#include <vector>
#include <string>
#include <iostream>
#include <format>
#include <map>
#include <tuple>
#include <cstdlib>

using namespace std;

struct version_wrapper_t
{
public:
    uint32_t version;
};

template <>
struct formatter<version_wrapper_t> : formatter<string> {
    auto format(const version_wrapper_t& vw, format_context& ctx) const {
        const auto major = VK_VERSION_MAJOR(vw.version);
        const auto minor = VK_VERSION_MINOR(vw.version);
        const auto patch = VK_VERSION_PATCH(vw.version);
        return formatter<string>::format(std::format("{}.{}.{}", major, minor, patch), ctx);
    }
};

template <>
struct formatter<vk::QueueFlags> : formatter<string> {
    auto format(const vk::QueueFlags& qf, format_context& ctx) const {
        const auto graphics = static_cast<bool>(qf & vk::QueueFlagBits::eGraphics);
        const auto compute = static_cast<bool>(qf & vk::QueueFlagBits::eCompute);
        const auto transfer = static_cast<bool>(qf & vk::QueueFlagBits::eTransfer);
        const auto videoDecode = static_cast<bool>(qf & vk::QueueFlagBits::eVideoDecodeKHR);
        const auto videoEncode = static_cast<bool>(qf & vk::QueueFlagBits::eVideoEncodeKHR);

        auto current = std::format_to(ctx.out(), "QueueFlags{{");
        if(graphics)
            current = std::format_to(current, "graphics, ");
        if(compute)
            current = std::format_to(current, "compute, ");
        if(transfer)
            current = std::format_to(current, "transfer, ");
        if(videoDecode)
            current = std::format_to(current, "videoDecode, ");
        if(videoEncode)
            current = std::format_to(current, "videoEncode, ");
        current = std::format_to(current, "}}");
        return current;
    }
};

void* aligned_alloc(size_t size, size_t alignment)
{
    if(alignment < alignof(void*))
    {
        alignment = alignof(void*);
    }
    size_t space = size + alignment - 1;
    void* allocated_mem = std::malloc(space + sizeof(void*));
    void* aligned_mem = static_cast<void*>(static_cast<char*>(allocated_mem) + sizeof(void*));
    std::align(alignment, size, aligned_mem, space);
    *(static_cast<void**>(aligned_mem) - 1) = allocated_mem;
    return aligned_mem;
}

void* aligned_realloc(void* ptr, size_t size, size_t alignment)
{
    if(alignment < alignof(void*))
    {
        alignment = alignof(void*);
    }
    size_t space = size + alignment - 1;
    void* allocated_mem = realloc(*(static_cast<void**>(ptr) - 1), space + sizeof(void*));
    void* aligned_mem = static_cast<void*>(static_cast<char*>(allocated_mem) + sizeof(void*));
    std::align(alignment, size, aligned_mem, space);
    *(static_cast<void**>(aligned_mem) - 1) = allocated_mem;
    return aligned_mem;
}

void aligned_free(void* ptr) noexcept
{
    if(ptr == nullptr)
        return;
    free(*(static_cast<void**>(ptr) - 1));
}

namespace VULKAN_HPP_NAMESPACE
{
    AllocationCallbacks createDefaultAllocationCallbacks()
    {
        return {
            nullptr,
            [](void* userData, size_t size, size_t alignment, VkSystemAllocationScope allocationScope)
            {
                return aligned_alloc(size, alignment);
            },
            [](void* userData, void* ptr, size_t size, size_t alignment, VkSystemAllocationScope allocationScope)
            {
                return aligned_realloc(ptr, size, alignment);
            },
            [](void* userData, void* ptr)
            {
                return aligned_free(ptr);
            },
            [](void* userData, size_t size, VkInternalAllocationType internalAllocationType, VkSystemAllocationScope allocationScope)
            {

            },
            [](void* userData, size_t size, VkInternalAllocationType internalAllocationType, VkSystemAllocationScope allocationScope)
            {

            }
        };
    }
}
