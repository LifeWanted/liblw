#pragma once

#include <stdexcept>

namespace lw {
namespace error {

class Exception : public std::runtime_error {
public:
    Exception( const std::int64_t code, const std::string& message ):
        std::runtime_error( message ),
        m_error_code( code )
    {}

    // ---------------------------------------------------------------------- //

    std::int64_t error_code( void ) const {
        return m_error_code;
    }

    // ---------------------------------------------------------------------- //

private:
    std::int64_t m_error_code;
};

// -------------------------------------------------------------------------- //

#define LW_DEFINE_EXECEPTION( _Type, _Parent )  \
    class _Type : public _Parent {              \
    public:                                     \
        using _Parent::_Parent;                 \
    };

}
}
