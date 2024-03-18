#pragma once

#include <common.hpp>
#include <error.hpp>

template<cl_platform_info ParamName>
struct platform_property_t
{
public:
    [[nodiscard]] static std::expected<void, error_t> get(cl_platform_id platform)
    {
        static_assert(ParamName != ParamName, "Unsupported param name value! Please specialize platform_property_t struct!");
    }
};

namespace internal
{
    template<cl_platform_info ParamName, typename ValueType>
    requires std::is_trivial_v<ValueType>
    struct platform_property_base
    {
        [[nodiscard]] static std::expected<ValueType, error_t> get_value(cl_platform_id platform)
        {
            ValueType property_value;

            cl_int result = clGetPlatformInfo(platform, ParamName, sizeof(property_value), &property_value, nullptr);
            if(result != CL_SUCCESS)
                return std::unexpected(error_t{result});

            return property_value;
        }
    };

    template<cl_platform_info ParamName>
    struct string_platform_property_base
    {
        template<typename CharTraits = std::char_traits<char>, typename Alloc = std::allocator<char>>
        [[nodiscard]] static std::expected<std::basic_string<char, CharTraits, Alloc>, error_t> get_value(cl_platform_id platform, const Alloc& alloc = Alloc())
        {
            using string_type = std::basic_string<char, CharTraits, Alloc>;
            using size_type = typename string_type::size_type;
            static_assert(std::is_convertible_v<typename string_type::pointer, char*>);

            size_t param_size;
            cl_int result;

            result = clGetPlatformInfo(platform, ParamName, static_cast<size_t>(0), nullptr, &param_size);
            if(result != CL_SUCCESS)
                return std::unexpected(error_t{result});

            string_type property_value(static_cast<size_type>(param_size), {}, alloc);

            result = clGetPlatformInfo(platform, ParamName, param_size, static_cast<char*>(property_value.data()), nullptr);
            if(result != CL_SUCCESS)
                return std::unexpected(error_t{result});

            return property_value;
        }
    };

    template<cl_platform_info ParamName, typename Elem>
    requires std::is_trivial_v<Elem>
    struct vector_platform_property_base
    {
        template<typename Alloc = std::allocator<Elem>>
        [[nodiscard]] static std::expected<std::vector<Elem, Alloc>, error_t> get_value(cl_platform_id platform, const Alloc& alloc = Alloc())
        {
            using vector_type = std::vector<Elem, Alloc>;
            using size_type = typename vector_type::size_type;
            static_assert(std::is_convertible_v<typename vector_type::pointer, Elem*>);

            size_t param_size;
            cl_int result;

            result = clGetPlatformInfo(platform, ParamName, static_cast<size_t>(0), nullptr, &param_size);
            if(result != CL_SUCCESS)
                return std::unexpected(result);

            auto elements_count = static_cast<size_type>(param_size / sizeof(Elem));
            vector_type property_value(elements_count, alloc);

            result = clGetPlatformInfo(platform, ParamName, param_size, static_cast<cl_name_version*>(property_value.data()), nullptr);
            if(result != CL_SUCCESS)
                return std::unexpected(result);

            return property_value;
        }
    };
}

#define SPECIALIZE_PLATFORM_PROPERTY(ParamName, ValueType) \
template<>\
struct platform_property_t<ParamName> : protected internal::platform_property_base<ParamName, ValueType>\
{\
public:\
    [[nodiscard]] static auto value(cl_platform_id platform)\
    {\
        return get_value(platform);\
    }\
}

#define SPECIALIZE_PLATFORM_STRING_PROPERTY(ParamName) \
template<>\
struct platform_property_t<ParamName> : protected internal::string_platform_property_base<ParamName>\
{\
public:\
    template<typename CharTraits = std::char_traits<char>, typename Alloc = std::allocator<char>>\
    [[nodiscard]] static auto value(cl_platform_id platform, const Alloc& alloc = Alloc())\
    {\
        return get_value<CharTraits, Alloc>(platform, alloc);\
    }\
}

#define SPECIALIZE_PLATFORM_VECTOR_PROPERTY(ParamName, ValueType) \
template<>\
struct platform_property_t<ParamName> : protected internal::vector_platform_property_base<ParamName, ValueType>\
{\
public:\
    template<class Alloc = std::allocator<size_t>>\
    [[nodiscard]] static auto value(cl_platform_id platform, const Alloc alloc = Alloc())\
    {\
        return get_value<Alloc>(platform, alloc);\
    }\
}

SPECIALIZE_PLATFORM_STRING_PROPERTY(CL_PLATFORM_PROFILE);
SPECIALIZE_PLATFORM_STRING_PROPERTY(CL_PLATFORM_VERSION);
SPECIALIZE_PLATFORM_PROPERTY(CL_PLATFORM_NUMERIC_VERSION,                       cl_version);
SPECIALIZE_PLATFORM_STRING_PROPERTY(CL_PLATFORM_NAME);
SPECIALIZE_PLATFORM_STRING_PROPERTY(CL_PLATFORM_VENDOR);
SPECIALIZE_PLATFORM_STRING_PROPERTY(CL_PLATFORM_EXTENSIONS);
SPECIALIZE_PLATFORM_VECTOR_PROPERTY(CL_PLATFORM_EXTENSIONS_WITH_VERSION,        cl_name_version);
SPECIALIZE_PLATFORM_PROPERTY(CL_PLATFORM_HOST_TIMER_RESOLUTION,                 cl_ulong);
