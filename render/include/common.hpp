#pragma once

#include <vulkan/vulkan.hpp>

#include <expected>
#include <optional>

#include <memory>

#include <string>

#include <vector>
#include <array>

#include <system_error>
#include <type_traits>

#define VULKAN_RENDER_NAMESPACE render::vulkan

namespace VULKAN_RENDER_NAMESPACE
{
    using std::vector;
    using std::expected;
    using std::allocator;
    using std::unexpected;
    using std::allocator_traits;
    using std::is_convertible_v;
}
