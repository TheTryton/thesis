#pragma once

#include <common.hpp>
#include <error.hpp>

template<typename Alloc = std::allocator<cl_device_id>>
[[nodiscard]] std::expected<std::vector<cl_device_id, Alloc>, error> get_devices(
    cl_platform_id platform,
    cl_device_type device_type = CL_DEVICE_TYPE_ALL,
    const Alloc& alloc = Alloc()
)
{
    using value_type = std::vector<cl_device_id, Alloc>;
    using size_type = typename value_type::size_type;
    static_assert(std::is_convertible_v<typename value_type::pointer, cl_device_id*>);

    cl_uint device_count;
    cl_int result;

    result = clGetDeviceIDs(platform, device_type, static_cast<cl_uint>(0), nullptr, &device_count);
    if(result == CL_DEVICE_NOT_FOUND)
        return {};
    if(result != CL_SUCCESS)
        return std::unexpected(error_t{result});

    value_type devices(static_cast<size_type>(device_count), alloc);

    result = clGetDeviceIDs(platform, device_type, device_count, static_cast<cl_device_id*>(devices.data()), nullptr);
    if(result != CL_SUCCESS)
        return std::unexpected(error_t{result});

    return devices;
}

template<typename Alloc = std::allocator<cl_device_id>>
[[nodiscard]] std::expected<std::vector<cl_device_id, Alloc>, error> create_sub_devices(
        cl_device_id device,
        const cl_device_partition_property* properties,
        const Alloc& alloc = Alloc()
)
{
    using value_type = std::vector<cl_device_id, Alloc>;
    using size_type = typename value_type::size_type;
    static_assert(std::is_convertible_v<typename value_type::pointer, cl_device_id*>);

    cl_uint device_count;
    cl_int result;

    result = clCreateSubDevices(device, properties, static_cast<cl_uint>(0), nullptr, &device_count);
    if(result != CL_SUCCESS)
        return std::unexpected(error_t{result});

    value_type devices(static_cast<size_type>(device_count), alloc);

    result = clCreateSubDevices(device, properties, device_count, static_cast<cl_device_id*>(devices.data()), nullptr);
    if(result != CL_SUCCESS)
        return std::unexpected(error_t{result});

    return devices;
}

std::optional<error> release(cl_device_id device);
std::optional<error> retain(cl_device_id device);
