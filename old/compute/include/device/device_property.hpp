#pragma once

#include <common.hpp>
#include <error.hpp>

template<cl_device_info ParamName>
struct device_property_t
{
public:
    [[nodiscard]] static std::expected<void, error_t> get_value(cl_device_id device)
    {
        static_assert(ParamName != ParamName, "Unsupported param name value! Please specialize platform_property_t struct!");
    }
};

namespace internal
{
    template<cl_device_info ParamName, typename ValueType>
    requires std::is_trivial_v<ValueType>
    struct device_property_base
    {
        [[nodiscard]] static std::expected<ValueType, error_t> get_value(cl_device_id device)
        {
            ValueType property_value;

            cl_int result = clGetDeviceInfo(device, ParamName, sizeof(property_value), &property_value, nullptr);
            if(result != CL_SUCCESS)
                return std::unexpected(error_t{result});

            return property_value;
        }
    };

    template<cl_device_info ParamName>
    struct string_device_property_base
    {
        template<typename CharTraits = std::char_traits<char>, typename Alloc = std::allocator<char>>
        [[nodiscard]] static std::expected<std::basic_string<char, CharTraits, Alloc>, error_t> get_value(cl_device_id device, const Alloc& alloc = Alloc())
        {
            using string_type = std::basic_string<char, CharTraits, Alloc>;
            using size_type = typename string_type::size_type;
            static_assert(std::is_convertible_v<typename string_type::pointer, char*>);

            size_t param_size;
            cl_int result;

            result = clGetDeviceInfo(device, ParamName, static_cast<size_t>(0), nullptr, &param_size);
            if(result != CL_SUCCESS)
                return std::unexpected(error_t{result});

            string_type property_value(static_cast<size_type>(param_size), {}, alloc);

            result = clGetDeviceInfo(device, ParamName, param_size, static_cast<char*>(property_value.data()), nullptr);
            if(result != CL_SUCCESS)
                return std::unexpected(error_t{result});

            return property_value;
        }
    };

    template<cl_device_info ParamName, typename Elem>
    requires std::is_trivial_v<Elem>
    struct vector_device_property_base
    {
        template<typename Alloc = std::allocator<Elem>>
        [[nodiscard]] static std::expected<std::vector<Elem, Alloc>, error_t> get_value(cl_device_id device, const Alloc& alloc = Alloc())
        {
            using vector_type = std::vector<Elem, Alloc>;
            using size_type = typename vector_type::size_type;
            static_assert(std::is_convertible_v<typename vector_type::pointer, Elem*>);

            size_t param_size;
            cl_int result;

            result = clGetDeviceInfo(device, ParamName, static_cast<size_t>(0), nullptr, &param_size);
            if(result != CL_SUCCESS)
                return std::unexpected(result);

            auto elements_count = static_cast<size_type>(param_size / sizeof(Elem));
            vector_type property_value(elements_count, alloc);

            result = clGetDeviceInfo(device, ParamName, param_size, static_cast<cl_name_version*>(property_value.data()), nullptr);
            if(result != CL_SUCCESS)
                return std::unexpected(result);

            return property_value;
        }
    };
}

#define SPECIALIZE_DEVICE_PROPERTY(ParamName, ValueType) \
template<>\
struct device_property_t<ParamName> : protected internal::device_property_base<ParamName, ValueType>\
{\
public:\
    [[nodiscard]] static auto value(cl_device_id device)\
    {\
        return get_value(device);\
    }\
}

#define SPECIALIZE_DEVICE_STRING_PROPERTY(ParamName) \
template<>\
struct device_property_t<ParamName> : protected internal::string_device_property_base<ParamName>\
{\
public:\
    template<typename CharTraits = std::char_traits<char>, typename Alloc = std::allocator<char>>\
    [[nodiscard]] static auto value(cl_device_id device, const Alloc& alloc = Alloc())\
    {\
        return get_value<CharTraits, Alloc>(device, alloc);\
    }\
}

#define SPECIALIZE_DEVICE_VECTOR_PROPERTY(ParamName, ValueType) \
template<>\
struct device_property_t<ParamName> : protected internal::vector_device_property_base<ParamName, ValueType>\
{\
public:\
    template<class Alloc = std::allocator<size_t>>\
    [[nodiscard]] static auto value(cl_device_id device, const Alloc alloc = Alloc())\
    {\
        return get_value<Alloc>(device, alloc);\
    }\
}

SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_TYPE,                                      cl_device_type);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_VENDOR_ID,                                 cl_uint);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_MAX_COMPUTE_UNITS,                         cl_uint);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS,                  cl_uint);
SPECIALIZE_DEVICE_VECTOR_PROPERTY(CL_DEVICE_MAX_WORK_ITEM_SIZES,                size_t);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_MAX_WORK_GROUP_SIZE,                       size_t);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR,               cl_uint);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT,              cl_uint);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT,                cl_uint);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG,               cl_uint);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT,              cl_uint);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE,             cl_uint);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF,               cl_uint);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR,                  cl_uint);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT,                 cl_uint);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_NATIVE_VECTOR_WIDTH_INT,                   cl_uint);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG,                  cl_uint);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT,                 cl_uint);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE,                cl_uint);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF,                  cl_uint);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_MAX_CLOCK_FREQUENCY,                       cl_uint);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_ADDRESS_BITS,                              cl_uint);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_MAX_MEM_ALLOC_SIZE,                        cl_ulong);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_IMAGE_SUPPORT,                             cl_bool);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_MAX_READ_IMAGE_ARGS,                       cl_uint);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_MAX_WRITE_IMAGE_ARGS,                      cl_uint);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_MAX_READ_WRITE_IMAGE_ARGS,                 cl_uint);
SPECIALIZE_DEVICE_STRING_PROPERTY(CL_DEVICE_IL_VERSION);
SPECIALIZE_DEVICE_VECTOR_PROPERTY(CL_DEVICE_ILS_WITH_VERSION,                   cl_name_version);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_IMAGE2D_MAX_WIDTH,                         size_t);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_IMAGE2D_MAX_HEIGHT,                        size_t);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_IMAGE3D_MAX_WIDTH,                         size_t);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_IMAGE3D_MAX_HEIGHT,                        size_t);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_IMAGE3D_MAX_DEPTH,                         size_t);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_IMAGE_MAX_BUFFER_SIZE,                     size_t);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_IMAGE_MAX_ARRAY_SIZE,                      size_t);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_MAX_SAMPLERS,                              cl_uint);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_IMAGE_PITCH_ALIGNMENT,                     cl_uint);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_IMAGE_BASE_ADDRESS_ALIGNMENT,              cl_uint);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_MAX_PIPE_ARGS,                             cl_uint);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_PIPE_MAX_ACTIVE_RESERVATIONS,              cl_uint);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_PIPE_MAX_PACKET_SIZE,                      cl_uint);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_MAX_PARAMETER_SIZE,                        size_t);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_MEM_BASE_ADDR_ALIGN,                       cl_uint);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE,                  cl_uint);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_SINGLE_FP_CONFIG,                          cl_device_fp_config);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_DOUBLE_FP_CONFIG,                          cl_device_fp_config);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_GLOBAL_MEM_CACHE_TYPE,                     cl_device_mem_cache_type);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE,                 cl_uint);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_GLOBAL_MEM_CACHE_SIZE,                     cl_ulong);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_GLOBAL_MEM_SIZE,                           cl_ulong);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE,                  cl_ulong);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_MAX_CONSTANT_ARGS,                         cl_uint);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_MAX_GLOBAL_VARIABLE_SIZE,                  size_t);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_GLOBAL_VARIABLE_PREFERRED_TOTAL_SIZE,      size_t);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_LOCAL_MEM_TYPE,                            cl_device_local_mem_type);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_LOCAL_MEM_SIZE,                            cl_ulong);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_ERROR_CORRECTION_SUPPORT,                  cl_bool);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_HOST_UNIFIED_MEMORY,                       cl_bool);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_PROFILING_TIMER_RESOLUTION,                size_t);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_ENDIAN_LITTLE,                             cl_bool);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_AVAILABLE,                                 cl_bool);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_COMPILER_AVAILABLE,                        cl_bool);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_LINKER_AVAILABLE,                          cl_bool);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_EXECUTION_CAPABILITIES,                    cl_device_exec_capabilities);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_QUEUE_PROPERTIES,                          cl_command_queue_properties);
#if CL_DEVICE_QUEUE_PROPERTIES != CL_DEVICE_QUEUE_ON_HOST_PROPERTIES
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_QUEUE_ON_HOST_PROPERTIES,                  cl_command_queue_properties);
#endif
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_QUEUE_ON_DEVICE_PROPERTIES,                cl_command_queue_properties);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_QUEUE_ON_DEVICE_PREFERRED_SIZE,            cl_uint);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_QUEUE_ON_DEVICE_MAX_SIZE,                  cl_uint);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_MAX_ON_DEVICE_QUEUES,                      cl_uint);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_MAX_ON_DEVICE_EVENTS,                      cl_uint);
SPECIALIZE_DEVICE_STRING_PROPERTY(CL_DEVICE_BUILT_IN_KERNELS);
SPECIALIZE_DEVICE_VECTOR_PROPERTY(CL_DEVICE_BUILT_IN_KERNELS_WITH_VERSION,      cl_name_version);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_PLATFORM,                                  cl_platform_id);
SPECIALIZE_DEVICE_STRING_PROPERTY(CL_DEVICE_NAME);
SPECIALIZE_DEVICE_STRING_PROPERTY(CL_DEVICE_VENDOR);
SPECIALIZE_DEVICE_STRING_PROPERTY(CL_DRIVER_VERSION);
SPECIALIZE_DEVICE_STRING_PROPERTY(CL_DEVICE_PROFILE);
SPECIALIZE_DEVICE_STRING_PROPERTY(CL_DEVICE_VERSION);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_NUMERIC_VERSION,                           cl_version);
SPECIALIZE_DEVICE_STRING_PROPERTY(CL_DEVICE_OPENCL_C_VERSION);
SPECIALIZE_DEVICE_VECTOR_PROPERTY(CL_DEVICE_OPENCL_C_ALL_VERSIONS,              cl_name_version);
SPECIALIZE_DEVICE_VECTOR_PROPERTY(CL_DEVICE_OPENCL_C_FEATURES,                  cl_name_version);
SPECIALIZE_DEVICE_STRING_PROPERTY(CL_DEVICE_EXTENSIONS);
SPECIALIZE_DEVICE_VECTOR_PROPERTY(CL_DEVICE_EXTENSIONS_WITH_VERSION,            cl_name_version);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_PRINTF_BUFFER_SIZE,                        size_t);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_PREFERRED_INTEROP_USER_SYNC,               cl_bool);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_PARENT_DEVICE,                             cl_device_id);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_PARTITION_MAX_SUB_DEVICES,                 cl_uint);
SPECIALIZE_DEVICE_VECTOR_PROPERTY(CL_DEVICE_PARTITION_PROPERTIES,               cl_device_partition_property);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_PARTITION_AFFINITY_DOMAIN,                 cl_device_affinity_domain);
SPECIALIZE_DEVICE_VECTOR_PROPERTY(CL_DEVICE_PARTITION_TYPE,                     cl_device_partition_property);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_REFERENCE_COUNT,                           cl_uint);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_SVM_CAPABILITIES,                          cl_device_svm_capabilities);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_PREFERRED_PLATFORM_ATOMIC_ALIGNMENT,       cl_uint);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_PREFERRED_GLOBAL_ATOMIC_ALIGNMENT,         cl_uint);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_PREFERRED_LOCAL_ATOMIC_ALIGNMENT,          cl_uint);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_MAX_NUM_SUB_GROUPS,                        cl_uint);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_SUB_GROUP_INDEPENDENT_FORWARD_PROGRESS,    cl_bool);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_ATOMIC_MEMORY_CAPABILITIES,                cl_device_atomic_capabilities);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_ATOMIC_FENCE_CAPABILITIES,                 cl_device_atomic_capabilities);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_NON_UNIFORM_WORK_GROUP_SUPPORT,            cl_bool);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_WORK_GROUP_COLLECTIVE_FUNCTIONS_SUPPORT,   cl_bool);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_GENERIC_ADDRESS_SPACE_SUPPORT,             cl_bool);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_DEVICE_ENQUEUE_CAPABILITIES,               cl_device_device_enqueue_capabilities);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_PIPE_SUPPORT,                              cl_bool);
SPECIALIZE_DEVICE_PROPERTY(CL_DEVICE_PREFERRED_WORK_GROUP_SIZE_MULTIPLE,        size_t);
SPECIALIZE_DEVICE_STRING_PROPERTY(CL_DEVICE_LATEST_CONFORMANCE_VERSION_PASSED);
