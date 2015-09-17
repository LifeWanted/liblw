#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iterator>

#include "lw/iter/Iterable.hpp"

namespace lw {
namespace memory {

typedef std::uint8_t byte;

// ---------------------------------------------------------------------------------------------- //

/// @brief A buffer that can store dynamically or statically allocated memory.
///
/// The buffers can be built around existing memory blocks (optionally taking ownership of them) or
/// can allocate their own blocks.
///
/// There are no protections in place for reading or writing outside the bounds of the buffer.
class Buffer : public iter::Iterable<Buffer, byte*, const byte*> {
public:
    typedef std::size_t size_type; ///< Type used for sizes.

    // ------------------------------------------------------------------------------------------ //

    /// @brief Creates an empty buffer object.
    Buffer(void):
        m_capacity( 0       ),
        m_data(     nullptr ),
        m_ownData(  false   )
    {}

    // ------------------------------------------------------------------------------------------ //

    /// @brief
    ///     Create a buffer wrapped around externally allocated memory, optionally taking ownership
    ///     of the memory.
    ///
    /// @param buffer   A pointer to the beginning of the buffer.
    /// @param capacity The size of the buffer in bytes.
    /// @param ownData  Flag indicating if this `Buffer` should take ownership of the memory.
    Buffer(byte* buffer, const size_type& capacity, const bool ownData = false):
        m_capacity( capacity        ),
        m_data(     (byte*)buffer   ),
        m_ownData(  ownData         )
    {}

    // ------------------------------------------------------------------------------------------ //

    /// @brief No copy constructor.
    Buffer(const Buffer& other) = delete;

    // ------------------------------------------------------------------------------------------ //

    /// @brief Move constructor.
    ///
    /// The ownership of the data is transfered to this buffer, but the `other` buffer remains
    /// pointing to the same block with the same capacity so it can continue being used. You are not
    /// guaranteed that the data in `other` will remain valid as this buffer may free the memory if
    /// it is destroyed.
    ///
    /// @param other The buffer to move.
    Buffer(Buffer&& other):
        m_capacity( other.m_capacity    ),
        m_data(     other.m_data        ),
        m_ownData(  other.m_ownData     )
    {
        other.m_ownData = false;
    }

    // ------------------------------------------------------------------------------------------ //

    /// @brief Move and resize constructor.
    ///
    /// Like `Buffer::Buffer(Buffer&&)`, ownership is transfered to this buffer, but the reported
    /// size will be the one given, regardless of the size of `other`.
    ///
    /// @param other    The buffer to move the ownership from.
    /// @param size     The new size to report with.
    Buffer(Buffer&& other, const std::size_t size):
        m_capacity( size            ),
        m_data(     other.m_data    ),
        m_ownData(  other.m_ownData )
    {
        other.m_ownData = false;
    }

    // ------------------------------------------------------------------------------------------ //

    /// @brief Create a buffer which allocates its own memory.
    ///
    /// @param size The number of bytes to allocate.
    explicit Buffer(const size_type& size):
        m_capacity( size                ),
        m_data(     new byte[ size ]    ),
        m_ownData(  true                )
    {}

    // ------------------------------------------------------------------------------------------ //

    /// @brief Generic data-copying constructor.
    ///
    /// The input iterator must support `operator-( const InputIterator&, const InputIterator&)` as
    /// a way of determining the distance between two iterators. On top of that it must support
    /// dereferencing (`operator*()`) and prefix increment (`operator++()`).
    ///
    /// @tparam InputIterator
    ///     A generic forward-iterator whose dereferenced value must be convertible to `byte`.
    ///
    /// @param begin    The iterator to begin the copy from.
    /// @param end      An iterator just past the end of the data.
    template<typename InputIterator>
    Buffer(InputIterator begin, const InputIterator& end):
        Buffer((size_type)(end - begin))
    {
        std::copy(begin, end, m_data);
    }

    // ------------------------------------------------------------------------------------------ //

    /// @brief ~Destructor will free the memory if it is owned by this `Buffer`.
    virtual ~Buffer(void){
        if (m_ownData && m_data) {
            delete[] m_data;
        }
    }

    // ------------------------------------------------------------------------------------------ //

    /// @brief Accesses the capacity of the buffer.
    ///
    /// @return The size in bytes of the buffer.
    size_type capacity(void) const {
        return m_capacity;
    }

    // ------------------------------------------------------------------------------------------ //

    /// @copydoc pl::memory::Buffer::capacity
    size_type size( void ) const {
        return capacity();
    }

    // ------------------------------------------------------------------------------------------ //

    /// @brief Gets the beginning of the buffer's data.
    ///
    /// @return A pointer to the first byte of the buffer.
    pointer data( void ){
        return m_data;
    }

    // ------------------------------------------------------------------------------------------ //

    /// @copydoc pl::memory::Buffer::data()
    const_pointer data( void ) const {
        return m_data;
    }

    // ------------------------------------------------------------------------------------------ //

    /// @brief Index into the buffer.
    ///
    /// @param i The number of bytes into the buffer to look.
    ///
    /// @return A reference to the byte at index `i`.
    byte& operator[]( const size_type i ){
        return m_data[ i ];
    }

    // ------------------------------------------------------------------------------------------ //

    /// @copydoc pl::memory::Buffer::operator[](const size_type&)
    const byte& operator[]( const size_type i ) const {
        return m_data[ i ];
    }

    // ------------------------------------------------------------------------------------------ //

    /// @brief Sets all the bytes in the buffer to `val`.
    ///
    /// @param val The byte value to set for all bytes in the buffer.
    void setMemory( const byte val ){
        std::memset( m_data, val, m_capacity );
    }

    // ------------------------------------------------------------------------------------------ //

    /// @brief Generic copy method.
    ///
    /// Will copy at most `size()` bytes of data.
    ///
    /// @param begin    The iterator to start from.
    /// @param end      The iterator one past the end.
    template< typename InputIterator >
    void copy( InputIterator begin, const InputIterator& end ){
        for( size_type i = 0; i < size() && begin != end; ++i, ++begin ){
            m_data[ i ] = *begin;
        }
    }

    // ------------------------------------------------------------------------------------------ //

    /// @brief Generic copy-n method.
    ///
    /// Will copy `std::min( size(), count )` bytes of data.
    ///
    /// @param begin The iterator to start from.
    /// @param count The number of bytes to copy.
    template< typename InputIterator >
    void copy( InputIterator begin, const size_type count ){
        std::copy_n( begin, std::min( count, size() ), m_data );
    }

    // ------------------------------------------------------------------------------------------ //

    /// @brief No copy operator.
    Buffer& operator=( const Buffer& other ) = delete;

    // ------------------------------------------------------------------------------------------ //

    /// @brief Moves control of a buffer over to this one.
    ///
    /// The ownership of the data is transfered to this buffer, but the `other` buffer remains
    /// pointing to the same block with the same capacity so it can continue being used. You are not
    /// guaranteed that the data in `other` will remain valid as this buffer may free the memory if
    /// it is destroyed.
    ///
    /// @param other The buffer to move the control from.
    ///
    /// @return A reference to this buffer.
    Buffer& operator=( Buffer&& other );

    // ------------------------------------------------------------------------------------------ //

    /// @brief Compares two buffers for equality.
    ///
    /// Two buffers are equal if they are the same size and contain exactly the same data, byte for
    /// byte.
    ///
    /// @param other The other `Buffer` to compare this one to.
    ///
    /// @return True if the two buffers are the same size and contain the same data.
    bool operator==( const Buffer& other ) const;

    // ------------------------------------------------------------------------------------------ //

    /// @brief Compares two buffers for inequality.
    ///
    /// Two buffers are equal if they are the same size and contain exactly the same data, byte for
    /// byte.
    ///
    /// @param other The other `Buffer` to compare this one to.
    ///
    /// @return False if the two buffers are the same size and contain the same data.
    bool operator!=( const Buffer& other ) const {
        return !(*this == other);
    }

    // ------------------------------------------------------------------------------------------ //

    /// @brief Combines two memory buffers using XOR.
    ///
    /// Only `std::min( this->size(), other.size() )` bytes are combined.
    ///
    /// @param other The buffer to combine with this one.
    ///
    /// @return A reference to this buffer.
    Buffer& operator^=( const Buffer& other );

    // ------------------------------------------------------------------------------------------ //

    /// @brief Combines two memory buffers using XOR.
    ///
    /// Only `std::min( this->size(), other.size() )` bytes are combined.
    ///
    /// @param other The buffer to combine with this one.
    ///
    /// @return A new buffer containing the combined data.
    Buffer operator^( const Buffer& other ) const;

    // ------------------------------------------------------------------------------------------ //

private:
    size_type   m_capacity; ///< The capacity of the buffer in bytes.
    byte*       m_data;     ///< The data wrapped by the buffer.
    bool        m_ownData;  ///< Flag indicating if the buffer owns the memory.

    // ------------------------------------------------------------------------------------------ //

    /// @brief Performs XOR between two buffers into a third.
    ///
    /// @param lhs Buffer on the left-hand-side of the XOR.
    /// @param rhs Buffer ont he right-hand-side of the XOR.
    /// @param out Buffer to assign the results of the XOR to.
    static void _xor( const Buffer& lhs, const Buffer& rhs, Buffer& out );
};

// ---------------------------------------------------------------------------------------------- //

/// @brief Creates a memory buffer on the stack.
///
/// @tparam Capacity The size, in bytes, of the buffer.
template< std::size_t Capacity >
class StackBuffer : public Buffer {
public:
    /// @brief Constructor has no options.
    StackBuffer( void ): Buffer( m_data, Capacity, false ){}

    // ------------------------------------------------------------------------------------------ //

    /// @brief Value-initialized buffer.
    ///
    /// @param val The byte value to initialize the buffer with.
    StackBuffer( const byte val ):
        Buffer( m_data, Capacity, false )
    {
        setMemory( val );
    }

    // ------------------------------------------------------------------------------------------ //

    /// @brief Generic data-copying constructor.
    ///
    /// The input iterator must support `operator-( const InputIterator&, const InputIterator&)` as
    /// a way of determining the distance between two iterators. On top of that it must support
    /// dereferencing (`operator*()`) and prefix increment (`operator++()`).
    ///
    /// @tparam InputIterator
    ///     A generic forward-iterator whose dereferenced value must be convertible to `byte`.
    ///
    /// @param begin    The iterator to begin the copy from.
    /// @param end      An iterator just past the end of the data.
    template< typename InputIterator >
    StackBuffer( InputIterator begin, const InputIterator& end ):
        StackBuffer()
    {
        const size_type copySize = std::min( Capacity, (size_type)(end - begin) );
        std::copy_n( begin, copySize, m_data );
    }

    // ------------------------------------------------------------------------------------------ //

    /// @brief ~Destructor.
    ~StackBuffer( void ){}

    // ------------------------------------------------------------------------------------------ //

private:
    byte m_data[ Capacity ]; ///< Internal buffer, allocated on the stack.
};

}
}
