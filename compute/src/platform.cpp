#include <platform.hpp>

vector<cl_platform_id> getPlatforms()
{
    cl_uint platform_count = 0;
    assert(clGetPlatformIDs(0, nullptr, &platform_count) == CL_SUCCESS);
    auto _platforms = vector<cl_platform_id>(platform_count);
    assert(clGetPlatformIDs(platform_count, _platforms.data(), nullptr) == CL_SUCCESS);
    return _platforms;
}

string getPlatformString(cl_platform_id platform, cl_platform_info parameter_name)
{
    size_t string_length = 0;
    assert(clGetPlatformInfo(platform, parameter_name, 0, nullptr, &string_length) == CL_SUCCESS);
    auto platform_string = string(string_length, '\0');
    assert(clGetPlatformInfo(platform, parameter_name, string_length, platform_string.data(), nullptr) == CL_SUCCESS);
    return platform_string;
}

version_t getPlatformVersion(cl_platform_id platform)
{
    cl_version version;
    if(clGetPlatformInfo(platform, CL_PLATFORM_NUMERIC_VERSION, sizeof(cl_version), &version, nullptr) == CL_SUCCESS)
    {
        return version_t{numeric_version_t{version}};
    }
    else
    {
        return version_t{string_version_t{getPlatformString(platform, CL_PLATFORM_VERSION)}};
    }
}

vector<extension_t> getPlatformExtensions(cl_platform_id platform)
{
    size_t count = 0;
    if(clGetPlatformInfo(platform, CL_PLATFORM_EXTENSIONS_WITH_VERSION, 0, nullptr, &count) == CL_SUCCESS)
    {
        auto extensionsWithVersionRaw = vector<cl_name_version>(count / sizeof(cl_name_version));
        assert(clGetPlatformInfo(platform, CL_PLATFORM_EXTENSIONS_WITH_VERSION, count, extensionsWithVersionRaw.data(), nullptr) == CL_SUCCESS);
        auto extensionsWithVersion = vector<extension_t>(count / sizeof(cl_name_version));
        transform(begin(extensionsWithVersionRaw), end(extensionsWithVersionRaw), begin(extensionsWithVersion), [](const cl_name_version& nameVersion)
        {
            return extension_t{extension_with_version_t{nameVersion.name, nameVersion.version}};
        });
        return extensionsWithVersion;
    }
    else
    {
        auto extensions = getPlatformString(platform, CL_PLATFORM_EXTENSIONS);
        auto extensionsWithoutVersionRaw = split(extensions, " ");
        auto extensionsWithoutVersion = vector<extension_t>(extensionsWithoutVersionRaw.size());
        transform(begin(extensionsWithoutVersionRaw), end(extensionsWithoutVersionRaw), begin(extensionsWithoutVersion), [](const string& name)
        {
            return extension_t{extension_without_version_t{name}};
        });
        return extensionsWithoutVersion;
    }
}
