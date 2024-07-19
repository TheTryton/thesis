#pragma once

#include <common.hpp>

vector<cl_platform_id> getPlatforms();
string getPlatformString(cl_platform_id platform, cl_platform_info parameter_name);
version_t getPlatformVersion(cl_platform_id platform);
vector<extension_t> getPlatformExtensions(cl_platform_id platform);