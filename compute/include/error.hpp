#pragma once

#include <common.hpp>

struct error
{
public:
    cl_int error;
};

namespace std
{
    template <>
    struct is_error_code_enum<error> : true_type {};
}

struct error_category_t : std::error_category
{
    const char* name() const noexcept override;
    std::string message(int ev) const override;
};

constexpr static error_category_t error_category {};

std::error_code make_error_code(error error);
