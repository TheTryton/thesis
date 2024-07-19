#pragma once

#include <vulkan/vulkan.hpp>

#include <optional>

#include <memory>

#include <string>

#include <vector>
#include <array>

#include <system_error>
#include <type_traits>

#define RENDER_NAMESPACE render
#define VULKAN_RENDER_NAMESPACE RENDER_NAMESPACE::vulkan

namespace VULKAN_RENDER_NAMESPACE
{
using std::vector;
using std::allocator;
using std::unexpected;
using std::allocator_traits;
using std::is_convertible_v;
}
