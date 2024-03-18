#include <device.hpp>

vector<cl_device_id> getDevices(cl_platform_id platform, cl_device_type deviceType)
{
    cl_uint device_count = 0;
    auto result = clGetDeviceIDs(platform, deviceType, 0,nullptr, &device_count);
    if(result == CL_DEVICE_NOT_FOUND)
        return {};
    assert(result == CL_SUCCESS);
    auto _devices = vector<cl_device_id>(device_count);
    assert(clGetDeviceIDs(platform, deviceType, device_count, _devices.data(), nullptr) == CL_SUCCESS);
    return _devices;
}

string getDeviceString(cl_device_id device, cl_device_info paramName)
{
    size_t string_length = 0;
    assert(clGetDeviceInfo(device, paramName, 0, nullptr, &string_length) == CL_SUCCESS);
    auto device_string = string(string_length, '\0');
    assert(clGetDeviceInfo(device, paramName, string_length, device_string.data(), nullptr) == CL_SUCCESS);
    return device_string;
}

version_t getDeviceVersion(cl_device_id device)
{
    cl_version version;
    if(clGetDeviceInfo(device, CL_DEVICE_NUMERIC_VERSION, sizeof(cl_version), &version, nullptr) == CL_SUCCESS)
    {
        return version_t{numeric_version_t{version}};
    }
    else
    {
        return version_t{string_version_t{getDeviceString(device, CL_DEVICE_VERSION)}};
    }
}

vector<extension_t> getDeviceExtensions(cl_device_id device)
{
    size_t count = 0;
    if(clGetDeviceInfo(device, CL_DEVICE_EXTENSIONS_WITH_VERSION, 0, nullptr, &count) == CL_SUCCESS)
    {
        auto extensionsWithVersionRaw = vector<cl_name_version>(count / sizeof(cl_name_version));
        assert(clGetDeviceInfo(device, CL_DEVICE_EXTENSIONS_WITH_VERSION, count, extensionsWithVersionRaw.data(), nullptr) == CL_SUCCESS);
        auto extensionsWithVersion = vector<extension_t>(count / sizeof(cl_name_version));
        transform(begin(extensionsWithVersionRaw), end(extensionsWithVersionRaw), begin(extensionsWithVersion), [](const cl_name_version& nameVersion)
        {
            return extension_t{extension_with_version_t{nameVersion.name, nameVersion.version}};
        });
        return extensionsWithVersion;
    }
    else
    {
        auto extensions = getDeviceString(device, CL_DEVICE_EXTENSIONS);
        auto extensionsWithoutVersionRaw = split(extensions, " ");
        auto extensionsWithoutVersion = vector<extension_t>(extensionsWithoutVersionRaw.size());
        transform(begin(extensionsWithoutVersionRaw), end(extensionsWithoutVersionRaw), begin(extensionsWithoutVersion), [](const string& name)
        {
            return extension_t{extension_without_version_t{name}};
        });
        return extensionsWithoutVersion;
    }
}

opencl_compiler_version_t getDeviceOpenCLCompilerVersion(cl_device_id device)
{
    size_t count = 0;
    if(clGetDeviceInfo(device, CL_DEVICE_OPENCL_C_ALL_VERSIONS, 0, nullptr, &count) == CL_SUCCESS)
    {
        auto compilerVersionsRaw = vector<cl_name_version>(count / sizeof(cl_name_version));
        assert(clGetDeviceInfo(device, CL_DEVICE_OPENCL_C_ALL_VERSIONS, count, compilerVersionsRaw.data(), nullptr) == CL_SUCCESS);
        auto compilerVersions = vector<numeric_version_t>(count / sizeof(cl_name_version));
        transform(begin(compilerVersionsRaw), end(compilerVersionsRaw), begin(compilerVersions), [](const cl_name_version& nameVersion)
        {
            return numeric_version_t{nameVersion.version};
        });
        return opencl_compiler_version_t{compilerVersions};
    }
    else
    {
        auto compilerVersion = getDeviceString(device, CL_DEVICE_OPENCL_C_VERSION);
        return opencl_compiler_version_t{string_version_t{compilerVersion}};
    }
}