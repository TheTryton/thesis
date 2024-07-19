#include <error.hpp>

std::error_code make_error_code(error error)
{
    return {static_cast<int>(error.error), ::error_category};
}

const char* error_category_t::name() const noexcept
{
    return "OpenCL Error";
}

std::string error_category_t::message(int ev) const
{
    switch (static_cast<cl_int>(ev))
    {
        case CL_SUCCESS:
            return "Success";
        case CL_DEVICE_NOT_FOUND:
            return "No OpenCL devices were found that matched query!";
        case CL_DEVICE_NOT_AVAILABLE:
            return "Device is currently not available!";
        case CL_COMPILER_NOT_AVAILABLE:
            return "OpenCL compiler is not available on the given device!";
        case CL_MEM_OBJECT_ALLOCATION_FAILURE:
            return "Failed to allocate buffer on the device!";
        case CL_OUT_OF_RESOURCES:
            return "Device ran out of memory!";
        case CL_OUT_OF_HOST_MEMORY:
            return "Host ran out of memory!";
        case CL_PROFILING_INFO_NOT_AVAILABLE:
            return "Profiling info is not available on given queue!";
        case CL_MEM_COPY_OVERLAP:
            return "Failed to copy buffer due to memory overlap!";
        case CL_IMAGE_FORMAT_MISMATCH:
            return "Failed to copy image due to format mismatch!";
        case CL_IMAGE_FORMAT_NOT_SUPPORTED:
            return "Given image format is not supported!";
        case CL_BUILD_PROGRAM_FAILURE:
            return "Failed to build program executable!";
        case CL_MAP_FAILURE:
            return "Failed to map requested region into host address space!";
        case CL_MISALIGNED_SUB_BUFFER_OFFSET:
            return "Sub-buffer object misaligned!";
        case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
            return "Invalid event status for one of the given events!";
        case CL_COMPILE_PROGRAM_FAILURE:
            return "Failed to compile program!";
        case CL_LINKER_NOT_AVAILABLE:
            return "Linker is not available on the given device!";
        case CL_LINK_PROGRAM_FAILURE:
            return "Failed to link compiled binaries and/or libraries!";
        case CL_DEVICE_PARTITION_FAILED:
            return "Failed to partition device!";
        case CL_KERNEL_ARG_INFO_NOT_AVAILABLE:
            return "Kernel argument info is not available!";
        default:
            return "Unknown error!";
    }
}
