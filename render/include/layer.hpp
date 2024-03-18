#pragma once

#include <common.hpp>
#include <error.hpp>

namespace VULKAN_RENDER_NAMESPACE
{
    template<typename Alloc = allocator<VkLayerProperties>>
    expected<vector<VkLayerProperties, Alloc>, error_t> getAllLayerProperties(const Alloc& alloc = Alloc())
    {
        using vector_type = vector<VkLayerProperties, Alloc>;
        using size_type = typename vector_type::size_type;

        static_assert(is_convertible_v<uint32_t, size_type>);
        static_assert(is_convertible_v<typename allocator_traits<Alloc>::pointer, VkLayerProperties*>);

        uint32_t property_count;
        VkResult result = vkEnumerateInstanceLayerProperties(&property_count, nullptr);
        if(result != VK_SUCCESS)
            return unexpected(error_t{result});

        vector_type properties(static_cast<size_type>(property_count), {}, alloc);

        result = vkEnumerateInstanceLayerProperties(&property_count, properties.data());
        if(result != VK_SUCCESS)
            return unexpected(error_t{result});

        return properties;
    }
}