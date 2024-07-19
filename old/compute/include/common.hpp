#pragma once

#include <iostream>
#include <vector>
#include <cstdint>
#include <format>
#include <string>
#include <variant>
#include <cassert>
#include <algorithm>
#include <expected>
#include <optional>

#define CL_TARGET_OPENCL_VERSION 300
#include <CL/cl.h>

using namespace std;

static_assert(sizeof(cl_version) == sizeof(uint32_t));

struct numeric_version_t
{
    numeric_version_t() = default;
    numeric_version_t(cl_version version)
            : major(CL_VERSION_MAJOR(version))
            , minor(CL_VERSION_MINOR(version))
            , patch(CL_VERSION_PATCH(version))
    {}

    uint16_t major;
    uint16_t minor;
    uint16_t patch;
};

template <>
struct formatter<numeric_version_t> : formatter<string> {
    auto format(const numeric_version_t& p, format_context& ctx) const {
        return formatter<string>::format(std::format("{}.{}.{}", p.major, p.minor, p.patch), ctx);
    }
};

struct string_version_t
{
    string version;
};

template <>
struct formatter<string_version_t> : formatter<string> {
    auto format(const string_version_t& p, format_context& ctx) const {
        return formatter<string>::format(std::format("{}", p.version), ctx);
    }
};

struct version_t
{
    variant<numeric_version_t, string_version_t> version;
};

template <>
struct formatter<version_t> : formatter<string> {
    auto format(const version_t& p, format_context& ctx) const {
        if(p.version.index() == 0)
        {
            return formatter<string>::format(std::format("{}", get<0>(p.version)), ctx);
        }
        else
        {
            return formatter<string>::format(std::format("{}", get<1>(p.version)), ctx);
        }
    }
};

struct extension_without_version_t
{
    string extension;
};

template <>
struct formatter<extension_without_version_t> : formatter<string> {
    auto format(const extension_without_version_t& p, format_context& ctx) const {
        return formatter<string>::format(std::format("{}", p.extension), ctx);
    }
};

struct extension_with_version_t
{
    string extension;
    version_t version;
};

template <>
struct formatter<extension_with_version_t> : formatter<string> {
    auto format(const extension_with_version_t& p, format_context& ctx) const {
        return formatter<string>::format(std::format("{} [{}]", p.extension, p.version), ctx);
    }
};

struct extension_t
{
    variant<extension_without_version_t, extension_with_version_t> extension;
};

template <>
struct formatter<extension_t> : formatter<string> {
    auto format(const extension_t& p, format_context& ctx) const {
        if(p.extension.index() == 0)
        {
            return formatter<string>::format(std::format("{}", get<0>(p.extension)), ctx);
        }
        else
        {
            return formatter<string>::format(std::format("{}", get<1>(p.extension)), ctx);
        }
    }
};

struct opencl_compiler_version_t
{
    variant<string_version_t, vector<numeric_version_t>> version;
};

template <>
struct formatter<opencl_compiler_version_t> : formatter<string> {
    auto format(const opencl_compiler_version_t& p, format_context& ctx) const {
        if(p.version.index() == 0)
        {
            return formatter<string>::format(std::format("{}", get<0>(p.version)), ctx);
        }
        else
        {
            auto curr = ctx.out();
            curr = std::format_to(curr, "[");
            auto versions = get<1>(p.version);
            for(int i=0; i<versions.size(); i++)
            {
                curr = std::format_to(curr, "{}", versions[i]);
                if(i != versions.size() - 1)
                    curr = std::format_to(curr, ", ");
            }
            curr = std::format_to(curr, "]");
            return curr;
        }
    }
};

struct mhz_t
{
    cl_uint mhz;
};

template <>
struct formatter<mhz_t> : formatter<string> {
    auto format(const mhz_t& p, format_context& ctx) const {
        return formatter<string>::format(std::format("{} MHz", p.mhz), ctx);
    }
};

struct mem_size_t
{
    cl_ulong mem_size;
};

template <>
struct formatter<mem_size_t> : formatter<string> {
    auto format(const mem_size_t& p, format_context& ctx) const {
        if(p.mem_size >= 1024 * 1024 * 1024)
        {
            return formatter<string>::format(std::format("{:.4f} GB", p.mem_size / (double)(1024 * 1024 * 1024)), ctx);
        }
        else if(p.mem_size >= 1024 * 1024)
        {
            return formatter<string>::format(std::format("{:.4f} MB", p.mem_size / (double)(1024 * 1024)), ctx);
        }
        else if(p.mem_size >= 1024)
        {
            return formatter<string>::format(std::format("{:.4f} KB", p.mem_size / (double)(1024)), ctx);
        }
        else
        {
            return formatter<string>::format(std::format("{:.4f} B", p.mem_size / (double)(1)), ctx);
        }
    }
};

vector<string> split(string s, string delimiter);
