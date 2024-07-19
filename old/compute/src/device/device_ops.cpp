#include <device/device_ops.hpp>

std::optional<error> release(cl_device_id device)
{
    cl_int result = clReleaseDevice(device);
    if(result != CL_SUCCESS)
        return error_t{result};
    return std::nullopt;
}

std::optional<error> retain(cl_device_id device)
{
    cl_int result = clRetainDevice(device);
    if(result != CL_SUCCESS)
        return error_t{result};
    return std::nullopt;
}
