#pragma once

#include <common.hpp>

vector<cl_device_id> getDevices(cl_platform_id platform, cl_device_type deviceType);
template<typename T>
requires is_trivial_v<T>
T getDeviceParam(cl_device_id device, cl_device_info paramName)
{
    T value;
    assert(clGetDeviceInfo(device, paramName, sizeof(value), &value, nullptr) == CL_SUCCESS);
    return value;
}

string getDeviceString(cl_device_id device, cl_device_info paramName);
version_t getDeviceVersion(cl_device_id device);
vector<extension_t> getDeviceExtensions(cl_device_id device);
opencl_compiler_version_t getDeviceOpenCLCompilerVersion(cl_device_id device);