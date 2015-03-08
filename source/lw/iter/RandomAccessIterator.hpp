#pragma once

namespace lw {
namespace iter {

/// @brief A base class for random-access iterators.
///
/// Child classes must implement their own unary `*` operator (`)
/// and their own subscript operator ().
///
/// The following methods must be implemented by classes inheriting from
/// `RandomAccessIterator` in order to complete the STL requirements:
///     - `ValueType& operator*( void ) const`
///     - `ValueType& operator[]( const std::size_t ) const`
///     - `T& operator=( const T& )`
///     - `bool operator==( const T&) const`
///
/// @tparam T           The child iterator class which must inherit from
///                     `RandomAccessIterator`.
/// @tparam ValueType   The type being iterated over.
template< class T, typename ValueType >
class RandomAccessIterator {
private:
    /// @brief Our offset within whatever container is being iterated over.
    std::size_t m_offset;

    // ---------------------------------------------------------------------- //

    /// @brief Up casts `this` to the child class.
    ///
    /// @return A pointer to `T` for `this`.
    T* castThis( void ){
        return (T*)this;
    }

    // ---------------------------------------------------------------------- //

    /// @copydoc RandomAccessIterator::castThis()
    const T* castThis( void ) const {
        return (T*)this;
    }

    // ---------------------------------------------------------------------- //

protected:
    /// @brief Access to the current offset position for child classes.
    ///
    /// @return The current offset into whatever container is being iterated over.
    std::size_t _getOffset( void ) const {
        return m_offset;
    }

    // ---------------------------------------------------------------------- //

    /// @brief Default constructor.
    RandomAccessIterator( void ):
        m_offset( 0 )
    {}

    // ---------------------------------------------------------------------- //

    /// @brief Starting offset constructor.
    ///
    /// @param offset The offset in the container to start at.
    explicit RandomAccessIterator( const std::size_t offset ):
        m_offset( offset )
    {}

    // ---------------------------------------------------------------------- //

    /// @brief Copy constructor.
    ///
    /// @param other The iterator to copy.
    RandomAccessIterator( const RandomAccessIterator& other ):
        m_offset( other.m_offset )
    {}

    // ---------------------------------------------------------------------- //

    /// @brief Assignment operator only available to sub-classes.
    ///
    /// Copies the position of `other` into this one. It is up to the child
    /// class `T` to implement any aditional copying or moving of data as needed.
    ///
    /// @param other The iterator to copy the position of.
    ///
    /// @return A reference to this iterator as a `T`.
    T& operator=( const RandomAccessIterator& other ){
        m_offset = other.m_offset;
        return *castThis();
    }

    // ---------------------------------------------------------------------- //

    /// @brief Equality operator only available to sub-classes.
    ///
    /// Two `RandomAccessIterator`s are considered equal if both have the same
    /// offset. It is up to the child class `T` to implement any further
    /// equality comparisons as needed.
    ///
    /// @param other The iterator to check for equality with.
    bool operator==( const RandomAccessIterator& other ) const {
        return m_offset == other.m_offset;
    }

    // ---------------------------------------------------------------------- //

public:
    typedef ValueType       value_type;         ///< The type being iterated over.
    typedef value_type&     reference;          ///< For references to values.
    typedef value_type*     pointer;            ///< For pointers to a value.
    typedef std::ptrdiff_t  difference_type;    ///< For differences in position.

    /// @brief Marks this iterator as being random access.
    typedef std::random_access_iterator_tag iterator_category;

    // ---------------------------------------------------------------------- //

    /// @brief Calculates the difference in position between two iterators.
    ///
    /// @param other The iterator to calculate the difference from.
    ///
    /// @return The difference of position.
    difference_type operator-( const T& other ) const {
        return ((difference_type)m_offset) - other.m_offset;
    }

    // ---------------------------------------------------------------------- //

    /// @brief Moves the position of the iterator `rhs` positions.
    ///
    /// @param rhs The number of elements to move the iterator forward by.
    ///
    /// @return A reference to this iterator at the new position.
    T& operator+=( const std::size_t rhs ){
        m_offset += rhs;
        return *castThis();
    }

    // ---------------------------------------------------------------------- //

    /// @brief Moves the position of the iterator `rhs` positions.
    ///
    /// @param rhs The number of elements to move the iterator backwards by.
    ///
    /// @return A reference to this iterator at the new position.
    T& operator-=( const std::size_t rhs ){
        return (*castThis()) += (-rhs);
    }

    // ---------------------------------------------------------------------- //

    /// @brief Increments the position of the iterator by one.
    ///
    /// @return A reference to this iterator at the new position.
    T& operator++( void ){
        ++m_offset;
        return *castThis();
    }

    // ---------------------------------------------------------------------- //

    /// @brief Decrements the position of the iterator by one.
    ///
    /// @return A reference to this iterator at the new position.
    T& operator--( void ){
        --m_offset;
        return *castThis();
    }

    // ---------------------------------------------------------------------- //

    /// @brief  Increments the position of the iterator by one and returns the
    ///         old value.
    ///
    /// @return An iterator pointing to the old position.
    T operator++( int ){
        T iterator( *castThis() );
        ++m_offset;
        return std::move( iterator );
    }

    // ---------------------------------------------------------------------- //

    /// @brief  Decrements the position of the iterator by one and returns the
    ///         old value.
    ///
    /// @return An iterator pointing to the old position.
    T operator--( int ){
        T iterator( *castThis() );
        --m_offset;
        return std::move( iterator );
    }

    // ---------------------------------------------------------------------- //

    /// @brief Checks if this iterator is at a position before `other`.
    ///
    /// @return True if the position of this iterator is less than the position
    ///         of the other.
    bool operator<( const T& other ) const {
        return m_offset < other.m_offset;
    }

    // ---------------------------------------------------------------------- //

    /// @brief Checks if this iterator is at a position after `other`.
    ///
    /// @return True if the position of this iterator is greater than the
    ///         position of the other.
    bool operator>( const T& other ) const {
        return m_offset > other.m_offset;
    }

    // ---------------------------------------------------------------------- //

    /// @brief Checks if this iterator is at a position before `other` or equal to.
    ///
    /// @return True if the position of this iterator is less than the position
    ///         of the other or equal to it.
    bool operator<=( const T& other ) const {
        return (*castThis()) == other || (*castThis()) < other;
    }

    // ---------------------------------------------------------------------- //

    /// @brief Checks if this iterator is at a position after `other` or equal to.
    ///
    /// @return True if the position of this iterator is greater than the
    ///         position of the other or equal to it.
    bool operator>=( const T& other ) const {
        return (*castThis()) == other || (*castThis()) > other;
    }

    // ---------------------------------------------------------------------- //

    friend bool operator!=( const T& lhs, const T& rhs ){
        return !(lhs == rhs);
    }

    // ---------------------------------------------------------------------- //

    /// @brief Creates a new iterator `n` positions later than `it`.
    ///
    /// @param it   The iterator to base the new one off.
    /// @param n    The number of positions to adjust the new iterator by.
    ///
    /// @return A new iterator `n` positions after `it`.
    friend T operator+( const T& it, const difference_type n ){
        T iterator( it );
        iterator += n;
        return std::move( iterator );
    }

    // ---------------------------------------------------------------------- //

    /// @copydoc operator+(const T&, const difference_type)
    friend T operator+( const difference_type n, const T& it ){
        return it + n;
    }

    // ---------------------------------------------------------------------- //

    /// @brief Creates a new iterator `n` positions before `it`.
    ///
    /// @param it   The iterator to base the new one off.
    /// @param n    The number of positions to adjust the new iterator by.
    ///
    /// @return A new iterator `n` positions before `it`.
    friend T operator-( const T& it, const difference_type n ){
        return it + (-n);
    }

    // ---------------------------------------------------------------------- //

    /// @copydoc operator-(const T&, const difference_type)
    friend T operator-( const difference_type n, const T& it ){
        return it - n;
    }
};

}
}
