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

}
}
