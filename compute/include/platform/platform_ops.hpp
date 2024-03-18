#pragma once

#include <common.hpp>
#include <error.hpp>

template<typename Alloc = std::allocator<cl_platform_id>>
[[nodiscard]] std::expected<std::vector<cl_platform_id, Alloc>, error_t> get_platforms(const Alloc& alloc = Alloc())
{
    using value_type = std::vector<cl_platform_id, Alloc>;
    using size_type = typename value_type::size_type;
    static_assert(std::is_convertible_v<typename value_type::pointer, cl_platform_id*>);

    cl_uint platform_count;
    cl_int result;

    result = clGetPlatformIDs(static_cast<cl_uint>(0), nullptr, &platform_count);
    if(result != CL_SUCCESS)
        return std::unexpected(error_t{result});

    value_type platforms(static_cast<size_type>(platform_count), alloc);

    result = clGetPlatformIDs(platform_count, static_cast<cl_platform_id*>(platforms.data()), nullptr);
    if(result != CL_SUCCESS)
        return std::unexpected(error_t{result});

    return platforms;
}