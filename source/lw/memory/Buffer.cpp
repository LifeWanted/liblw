
#include "lw/memory/Buffer.hpp"

namespace lw {
namespace memory {

void Buffer::_xor( const Buffer& lhs, const Buffer& rhs, Buffer& out ){
    for( size_type i = 0; i < out.size(); ++i ){
        out.m_data[ i ] = lhs.m_data[ i ] ^ rhs.m_data[ i ];
    }
}

// -------------------------------------------------------------------------- //

Buffer& Buffer::operator=( Buffer&& other ){
    // If own our current data, delete it first.
    if( m_ownData && m_data ){
        delete[] m_data;
    }

    // Copy the information over.
    m_data      = other.m_data;
    m_capacity  = other.m_capacity;
    m_ownData   = other.m_ownData;

    // Remove ownership of the buffer from the other one.
    other.m_ownData = false;

    // Return self.
    return *this;
}

// ------------------------------------------------------------------------- //

bool Buffer::operator==( const Buffer& other ) const {
    if( size() != other.size() ){
        return false;
    }
    if( data() == other.data() ){
        return true;
    }

    return std::memcmp( data(), other.data(), size() ) == 0;
}

// -------------------------------------------------------------------------- //

Buffer& Buffer::operator^=( const Buffer& other ){
    // Stupid Windows defines a "min" macro that conflicts with std::min.
    using namespace std;
    Buffer tmp( m_data, min( this->size(), other.size() ), false );
    _xor( *this, other, tmp );
    return *this;
}

// -------------------------------------------------------------------------- //

Buffer Buffer::operator^( const Buffer& other ) const {
    // Prepare our buffers.
    // Stupid Windows defines a "min" macro that conflicts with std::min.
    using namespace std;
    Buffer output( this->size() );
    Buffer tmp( output.m_data, min( this->size(), other.size() ), false );

    // Perform the xor operation.
    _xor( *this, other, tmp );

    // Copy the remaining data from this buffer over and return the output buffer.
    std::copy_n(
        m_data + tmp.size(),
        this->size() - tmp.size(),
        output.m_data + tmp.size()
    );
    return std::move( output );
}

}
}
