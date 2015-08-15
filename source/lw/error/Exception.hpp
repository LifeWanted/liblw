#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>

namespace lw {
namespace error {

class Exception : public std::runtime_error {
public:
    Exception(const std::int64_t code, const std::string& message):
        std::runtime_error(message),
        m_error_code(code)
    {}

    // ---------------------------------------------------------------------- //

    std::int64_t error_code(void) const {
        return m_error_code;
    }

    // ---------------------------------------------------------------------- //

private:
    std::int64_t m_error_code;
};

// -------------------------------------------------------------------------- //

#define LW_DEFINE_EXCEPTION_EX(_Type, _ParentNamespace, _Parent)    \
    class _Type : public _ParentNamespace::_Parent {                \
    public:                                                         \
        using _ParentNamespace::_Parent::_Parent;                   \
    };

#define LW_DEFINE_EXCEPTION(_Type) \
    LW_DEFINE_EXCEPTION_EX( _Type, ::lw::error, Exception )

// -------------------------------------------------------------------------- //

#define LW_UV_ERROR(_Type, _code) \
    _Type(_code, (std::string)uv_err_name(_code) + ": " + uv_strerror(_code))

}
}
