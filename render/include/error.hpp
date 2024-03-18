#pragma once

#include <typedefs.hpp>

namespace VULKAN_RENDER_NAMESPACE
{
    struct error_t
    {
    private:
        VkResult _value;
    public:
        error_t() = default;
        constexpr error_t(VkResult value)
            : _value(value)
        {
        }
    public:
        [[nodiscard]] constexpr VkResult value() const
        {
            return _value;
        }
    };
}

namespace std
{
    template <>
    struct is_error_code_enum<VULKAN_RENDER_NAMESPACE::error_t> : true_type {};

    error_code make_error_code(VULKAN_RENDER_NAMESPACE::error_t error);
}
