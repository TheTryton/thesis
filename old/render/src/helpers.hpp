#pragma once

#define NOMINMAX

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <set>
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <map>
#include <span>
#include <filesystem>
#include <fstream>
#include <tuple>
#include <cstdlib>
#include <cassert>
#include <bit>
#include <cstddef>
#include <random>
#include <memory_resource>
#include <sail-c++/sail-c++.h>
/*
#include <QApplication>
#include <QWidget>
#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
*/
#include <vulkan/vulkan_video.hpp>
#include <vk_video/vulkan_video_codec_av1std.h>

using namespace std;

struct version_wrapper_t
{
public:
    uint32_t version;
public:
    friend ostream& operator<<(ostream& os, const version_wrapper_t& vw);
};

ostream& operator<<(ostream& os, const version_wrapper_t& vw)
{
    const auto major = VK_VERSION_MAJOR(vw.version);
    const auto minor = VK_VERSION_MINOR(vw.version);
    const auto patch = VK_VERSION_PATCH(vw.version);
    os << major << '.' << minor << '.' << patch;
    return os;
}

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

template <>
struct formatter<VkSystemAllocationScope> : formatter<string> {
    auto format(const VkSystemAllocationScope& allocationScope, format_context& ctx) const {
        switch (allocationScope)
        {
            case VK_SYSTEM_ALLOCATION_SCOPE_COMMAND:
                return std::format_to(ctx.out(), "command");
            case VK_SYSTEM_ALLOCATION_SCOPE_OBJECT:
                return std::format_to(ctx.out(), "object");
            case VK_SYSTEM_ALLOCATION_SCOPE_CACHE:
                return std::format_to(ctx.out(), "cache");
            case VK_SYSTEM_ALLOCATION_SCOPE_DEVICE:
                return std::format_to(ctx.out(), "device");
            case VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE:
                return std::format_to(ctx.out(), "instance");
            default:
                return std::format_to(ctx.out(), "unknown");
        }
    }
};

template <>
struct formatter<VkInternalAllocationType> : formatter<string> {
    auto format(const VkInternalAllocationType& allocationType, format_context& ctx) const {
        switch (allocationType)
        {
            case VK_INTERNAL_ALLOCATION_TYPE_EXECUTABLE:
                return std::format_to(ctx.out(), "executable");
            default:
                return std::format_to(ctx.out(), "unknown");
        }
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

class tracking_memory_resource
    : public pmr::memory_resource
{
private:
    pmr::memory_resource* _memoryResource;
    size_t _allocations;
    size_t _deallocations;
    map<size_t, size_t, less<size_t>, pmr::polymorphic_allocator<pair<const size_t, size_t>>> _allocationSizeCounts;
    map<size_t, size_t, less<size_t>, pmr::polymorphic_allocator<pair<const size_t, size_t>>> _allocationAlignmentCounts;
public:
    tracking_memory_resource(pmr::memory_resource* memoryResource)
        : _memoryResource(memoryResource)
        , _allocationSizeCounts(pmr::polymorphic_allocator<pair<const size_t, size_t>>(memoryResource))
        , _allocationAlignmentCounts(pmr::polymorphic_allocator<pair<const size_t, size_t>>(memoryResource))
        , _allocations(0)
        , _deallocations(0)
    {}
public:
    [[nodiscard]] size_t allocations() const noexcept
    {
        return _allocations;
    }
    [[nodiscard]] size_t deallocations() const noexcept
    {
        return _deallocations;
    }
    [[nodiscard]] const auto& allocationSizeCounts() const noexcept
    {
        return _allocationSizeCounts;
    }
    [[nodiscard]] const auto& allocationAlignmentCounts() const noexcept
    {
        return _allocationAlignmentCounts;
    }
protected:
    void* do_allocate(size_t bytes, size_t alignment) override
    {
        ++_allocationSizeCounts[bytes];
        ++_allocationAlignmentCounts[alignment];

        ++_allocations;

        return _memoryResource->allocate(bytes, alignment);
    }
    void do_deallocate(void* ptr, size_t bytes, size_t alignment) override
    {
        ++_deallocations;

        _memoryResource->deallocate(ptr, bytes, alignment);
    }
    [[nodiscard]] bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override
    {
        return this == &other;
    }
};

class vulkan_tracking_memory_resource
{
private:
    struct alignas(alignof(max_align_t)) metadata_t
    {
        size_t size;
        size_t alignment;
        VkSystemAllocationScope allocationScope;
    };

    constexpr static size_t metadataSize = sizeof(metadata_t);
private:
    tracking_memory_resource _commandResource;
    tracking_memory_resource _objectResource;
    tracking_memory_resource _cacheResource;
    tracking_memory_resource _deviceResource;
    tracking_memory_resource _instanceResource;
    tracking_memory_resource _unknownResource;
public:
    vulkan_tracking_memory_resource(pmr::memory_resource* upstream)
        : _commandResource(upstream)
        , _objectResource(upstream)
        , _cacheResource(upstream)
        , _deviceResource(upstream)
        , _instanceResource(upstream)
        , _unknownResource(upstream)
    {
    }
private:
    static void printResourceStats(const tracking_memory_resource& resource, string_view resourceName)
    {
        cout << format("Resource=\"{}\":", resourceName) << endl;
        cout << format("\t- allocations: {}", resource.allocations()) << endl;
        cout << format("\t- deallocations: {}", resource.deallocations()) << endl;
        cout << format("\t- allocationSizesWithCounts:") << endl;
        for(const auto& allocationSizeWithCount : resource.allocationSizeCounts())
        {
            cout << format("\t\t- [{}]: {}", allocationSizeWithCount.first, allocationSizeWithCount.second) << endl;
        }
        cout << format("\t- allocationAlignmentsWithCounts:") << endl;
        for(const auto& allocationAlignmentWithCount : resource.allocationAlignmentCounts())
        {
            cout << format("\t\t- [{}]: {}", allocationAlignmentWithCount.first, allocationAlignmentWithCount.second) << endl;
        }
    }
public:
    ~vulkan_tracking_memory_resource()
    {
        /*printResourceStats(_commandResource, "command");
        printResourceStats(_objectResource, "object");
        printResourceStats(_cacheResource, "cache");
        printResourceStats(_deviceResource, "device");
        printResourceStats(_instanceResource, "instance");*/
    }
private:
    tracking_memory_resource* selectResource(VkSystemAllocationScope allocationScope)
    {
        switch (allocationScope)
        {
            case VK_SYSTEM_ALLOCATION_SCOPE_COMMAND:
                return &_commandResource;
            case VK_SYSTEM_ALLOCATION_SCOPE_OBJECT:
                return &_objectResource;
            case VK_SYSTEM_ALLOCATION_SCOPE_CACHE:
                return &_cacheResource;
            case VK_SYSTEM_ALLOCATION_SCOPE_DEVICE:
                return &_deviceResource;
            case VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE:
                return &_instanceResource;
            default:
                return &_unknownResource;
        }
    }
    void* writeMetadata(void* ptr, const metadata_t& metadata)
    {
        memcpy(ptr, &metadata, sizeof(metadata_t));
        return reinterpret_cast<uint8_t*>(ptr) + sizeof(metadata_t);
    }
    metadata_t readMetadata(void* ptr)
    {
        ptr = reinterpret_cast<uint8_t*>(ptr) - sizeof(metadata_t);
        metadata_t metadata{};
        memcpy(&metadata, ptr, sizeof(metadata_t));
        return metadata;
    }
private:
    void* allocate(metadata_t metadata)
    {
        const auto resource = selectResource(metadata.allocationScope);
        const auto ptr = resource->allocate(metadata.size + sizeof(metadata_t), metadata.alignment);
        return writeMetadata(ptr, metadata);
    }
    void* reallocate(void* oldPtr, metadata_t oldMetadata, metadata_t newMetadata)
    {
        const auto newPtr = allocate(newMetadata);

        memcpy(newPtr, oldPtr, min(oldMetadata.size, newMetadata.size));

        free(oldPtr, oldMetadata);

        return newPtr;
    }
    void free(void* ptr, metadata_t metadata)
    {
        const auto resource = selectResource(metadata.allocationScope);
        resource->deallocate(reinterpret_cast<uint8_t*>(ptr) - sizeof(metadata_t), metadata.size + sizeof(metadata_t), metadata.alignment);
    }
public:
    void* allocate(size_t size, size_t alignment, VkSystemAllocationScope allocationScope)
    {
        return allocate({size, alignment, allocationScope});
    }
    void* reallocate(void* ptr, size_t size, size_t alignment, VkSystemAllocationScope allocationScope)
    {
        if(ptr == nullptr)
            return allocate({size, alignment, allocationScope});

        return reallocate(ptr, readMetadata(ptr), {size, alignment, allocationScope});
    }
    void free(void* ptr)
    {
        if(ptr != nullptr)
            free(ptr, readMetadata(ptr));
    }
};

namespace VULKAN_HPP_NAMESPACE
{
    static auto upstream_resource = pmr::synchronized_pool_resource(pmr::pool_options(64, 8192));
    static auto tracking_resource = vulkan_tracking_memory_resource(&upstream_resource);

    AllocationCallbacks createDefaultAllocationCallbacks()
    {
        return {
            nullptr,
            [](void* userData, size_t size, size_t alignment, VkSystemAllocationScope allocationScope)
            {
                return tracking_resource.allocate(size, alignment, allocationScope);
                //cout << format("Allocation{{size=[{}], alignment=[{}], scope=[{}]}}", size, alignment, allocationScope) << endl;
                //return aligned_alloc(size, alignment);
            },
            [](void* userData, void* ptr, size_t size, size_t alignment, VkSystemAllocationScope allocationScope)
            {
                return tracking_resource.reallocate(ptr, size, alignment, allocationScope);
                //cout << format("Reallocation{{ptr=[{}], size=[{}], alignment=[{}], scope=[{}]}}", ptr, size, alignment, allocationScope) << endl;
                //return aligned_realloc(ptr, size, alignment);
            },
            [](void* userData, void* ptr)
            {
                return tracking_resource.free(ptr);
                //cout << format("Free{{ptr=[{}]}}", ptr) << endl;
                //return aligned_free(ptr);
            },
            [](void* userData, size_t size, VkInternalAllocationType internalAllocationType, VkSystemAllocationScope allocationScope)
            {
                cout << format("Internal Allocation{{size=[{}], type=[{}], scope=[{}]}}", size, internalAllocationType, allocationScope) << endl;
            },
            [](void* userData, size_t size, VkInternalAllocationType internalAllocationType, VkSystemAllocationScope allocationScope)
            {
                cout << format("Internal Free{{size=[{}], type=[{}], scope=[{}]}}", size, internalAllocationType, allocationScope) << endl;
            }
        };
    }
}
