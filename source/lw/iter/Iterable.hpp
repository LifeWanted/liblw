#pragma once

#include <iterator>

namespace lw {
namespace iter {

/// @brief Defines all the basic constant iterator methods needed by the STL.
///
/// @tparam T               The type to add iterators to. It must inerherit from
///                         `ConstIterable`.
/// @tparam ConstIterator   The iterator type to use.
template< typename T, typename ConstIterator >
class ConstIterable {
private:
    /// @brief Cast this instance to the templated type.
    ///
    /// @return A pointer to this as type `T`.
    const T* castThis( void ) const {
        return (const T*)this;
    }

    // ---------------------------------------------------------------------- //

protected:
    /// @brief Default all the constructors and the destructor.
    ConstIterable( void )                   = default;
    ConstIterable( const ConstIterable& )   = default;
    ~ConstIterable( void )                  = default;

    // ---------------------------------------------------------------------- //

public:
    /// @brief Const iterator type.
    typedef ConstIterator const_iterator;

    /// @brief Const reverse iterator type.
    typedef std::reverse_iterator< ConstIterator > const_reverse_iterator;

    /// @brief Iterator traits for this type.
    typedef std::iterator_traits< ConstIterator > traits;

    /// @brief Const reference to value type.
    typedef typename traits::reference const_reference;

    /// @brief Const pointer to value type.
    typedef typename traits::pointer const_pointer;

    // ---------------------------------------------------------------------- //

    /// @brief Gets a reference to the first element in the container.
    ///
    /// @return A const reference to the first element.
    const_reference front( void ) const {
        return const_reference( *(castThis()->data()) );
    }

    // ---------------------------------------------------------------------- //

    /// @brief Gets a reference to the last element in the container.
    ///
    /// @return A const reference to the last element.
    const_reference back( void ) const {
        return const_reference( *(castThis()->data() + castThis()->size() - 1) );
    }

    // ---------------------------------------------------------------------- //

    /// @brief Gets an iterator pointing to the first element of the container.
    ///
    /// @return A const iterator pointing at the first element.
    const_iterator begin( void ) const {
        return const_iterator( castThis()->data() );
    }

    // ---------------------------------------------------------------------- //

    /// @copydoc pl::util::ConstIterable::begin()
    const_iterator cbegin( void ) const {
        return castThis()->begin();
    }

    // ---------------------------------------------------------------------- //

    /// @brief  Gets a const reverse iterator pointing to the last element of
    ///         the container.
    ///
    /// @return A const reverse iterator pointing at the last element.
    const_reverse_iterator rbegin( void ) const {
        return const_reverse_iterator( castThis()->end() );
    }

    // ---------------------------------------------------------------------- //

    /// @copydoc pl::util::ConstIterable::rbegin()
    const_reverse_iterator crbegin( void ) const {
        return castThis()->rbegin();
    }

    // ---------------------------------------------------------------------- //

    /// @brief  Gets a const iterator pointing past the last element of the
    ///         container.
    ///
    /// @return A const iterator pointing at the last element.
    const_iterator end( void ) const {
        return const_iterator( castThis()->data() + castThis()->size() );
    }

    // ---------------------------------------------------------------------- //

    /// @copydoc pl::util::ConstIterable::end()
    const_iterator cend( void ) const {
        return castThis()->end();
    }

    // ---------------------------------------------------------------------- //

    /// @brief  Gets a const reverse iterator pointing to before the first
    ///         element of the container.
    ///
    /// @return A const reverse iterator pointing at before the first element.
    const_reverse_iterator rend( void ) const {
        return const_reverse_iterator( castThis()->begin() );
    }

    // ---------------------------------------------------------------------- //

    /// @copydoc pl::util::ConstIterable::rend()
    const_reverse_iterator crend( void ) const {
        return castThis()->rend();
    }
};

// -------------------------------------------------------------------------- //

/// @brief Defines all the basic iterators, const or otherwise, needed by the STL.
///
/// @tparam T               The type having the methods added to. It must inherit
///                         from `Iterable`.
/// @tparam Iterator        The basic iterator type.
/// @tparam ConstIterator   Iterator type for const methods.
template< typename T, typename Iterator, typename ConstIterator >
class Iterable : public ConstIterable< T, ConstIterator >{
private:
    /// @brief Cast this instance to the templated type.
    ///
    /// @return A pointer to this as type `T`.
    T* castThis( void ){
        return (T*)this;
    }

    // ---------------------------------------------------------------------- //

protected:
    /// @brief Default all the constructors and the destructor.
    Iterable( void )            = default;
    Iterable( const Iterable& ) = default;
    ~Iterable( void )           = default;

    // ---------------------------------------------------------------------- //

public:
    /// @brief The iterator type.
    typedef Iterator iterator;

    /// @brief Reversed iteration type.
    typedef std::reverse_iterator< Iterator > reverse_iterator;

    /// @brief Iterator traits for this type.
    typedef std::iterator_traits< Iterator > traits;

    /// @brief Reference to a value type.
    typedef typename traits::reference reference;

    /// @brief Value type.
    typedef typename traits::value_type value_type;

    /// @brief Pointer to a value type.
    typedef typename traits::pointer pointer;

    /// @brief Difference between two `iterator`s type.
    typedef typename traits::difference_type difference_type;

    // ---------------------------------------------------------------------- //

    // Import all the types and methods from ConstIterable.
    using typename ConstIterable< T, ConstIterator >::const_iterator;
    using typename ConstIterable< T, ConstIterator >::const_reverse_iterator;
    using typename ConstIterable< T, ConstIterator >::const_reference;
    using typename ConstIterable< T, ConstIterator >::const_pointer;

    using ConstIterable< T, ConstIterator >::front;
    using ConstIterable< T, ConstIterator >::back;
    using ConstIterable< T, ConstIterator >::begin;
    using ConstIterable< T, ConstIterator >::cbegin;
    using ConstIterable< T, ConstIterator >::rbegin;
    using ConstIterable< T, ConstIterator >::crbegin;
    using ConstIterable< T, ConstIterator >::end;
    using ConstIterable< T, ConstIterator >::cend;
    using ConstIterable< T, ConstIterator >::rend;
    using ConstIterable< T, ConstIterator >::crend;

    // ---------------------------------------------------------------------- //

    /// @brief Gets a reference to the first element in the container.
    ///
    /// @return A reference to the first element.
    reference front( void ){
        return reference( *(castThis()->data()) );
    }

    // ---------------------------------------------------------------------- //

    /// @brief Gets a reference to the last element in the container.
    ///
    /// @return A reference to the last element.
    reference back( void ){
        return reference( *(castThis()->data() + castThis()->size() - 1) );
    }

    // ---------------------------------------------------------------------- //

    /// @brief Gets an iterator pointing to the first element of the container.
    ///
    /// @return An iterator pointing at the first element.
    iterator begin( void ){
        return iterator( castThis()->data() );
    }

    // ---------------------------------------------------------------------- //

    /// @brief Gets a reverse iterator pointing to the last element in the container.
    ///
    /// @return A reverse iterator pointing at the last element.
    reverse_iterator rbegin( void ){
        return reverse_iterator( castThis()->end() );
    }

    // ---------------------------------------------------------------------- //

    /// @brief Gets an iterator pointing just after the last element of the container.
    ///
    /// @return An iterator pointing past the end of the container.
    iterator end( void ){
        return iterator( castThis()->data() + castThis()->size() );
    }

    // ---------------------------------------------------------------------- //

    /// @brief  Get a reverse iterator pointing in front of the first element in
    ///         the container.
    ///
    /// @return A reverse iterator pointing before the first element of the
    ///         container.
    reverse_iterator rend( void ){
        return reverse_iterator( castThis()->begin() );
    }
};

// -------------------------------------------------------------------------- //

/// @headerfile pl/util.hpp
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
