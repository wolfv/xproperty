/***************************************************************************
* Copyright (c) 2016, Johan Mabille and Sylvain Corlay                     *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XPROPERTY_HPP
#define XPROPERTY_HPP

#include <type_traits>
#include <cstddef>

#define xoffsetof(st, m) offsetof(st, m)

namespace xp
{

    /*************************
     * xproperty declaration *
     *************************/

    // Type, Owner Type, Derived Type

    template <class T, class O, class D>
    class xproperty
    {
    public:

        using owner_type = O;
        using derived_type = D;

        using value_type = T;
        using reference = T&;
        using const_reference = const T&;

        xproperty() noexcept(noexcept(std::is_nothrow_constructible<value_type>::value));
        xproperty(const_reference value) noexcept(noexcept(std::is_nothrow_constructible<value_type>::value));
        xproperty(value_type&& value) noexcept(noexcept(std::is_nothrow_move_constructible<value_type>::value));

        operator reference() noexcept;
        operator const_reference() const noexcept;

        template <class V>
        reference operator=(V&& value);

    private:

        owner_type* owner() noexcept;

        value_type m_value;
    }; 

    /*******************
     * XPROPERTY macro *
     *******************/

    // XPROPERTY(Type, Owner, Name)
    //
    // Defines a property of the specified type and name, for the specified owner type.
    //
    // The owner type must have two template methods
    //
    //  - invoke_validators<std::size_t Offset, typename const_ref>( const_ref value);
    //  - invoke_observers<std::size_t Offset>();
    //
    // Tthe `Offset` integral parameter is the offset of the observed member in the owner class.
    // The `const_ref` typename is a constant reference type on the proposed value.

    #define XPROPERTY(T, O, D) \
    class D ## _property  : public ::xp::xproperty<T, O, D ## _property> {\
    public:\
        template <class V>\
        inline typename ::xp::xproperty<T, O, D ## _property>::reference operator=(V&& value)\
        { return ::xp::xproperty<T, O, D ## _property>::operator=(std::forward<V>(value)); }\
        static inline constexpr std::size_t offset() noexcept { return xoffsetof(O, D); }\
    } D;

    /***********************
     * MAKE_OBSERVED macro *
     ***********************/

    // MAKE_OBSERVED()
    //
    // Adds the required boilerplate for an obsered structure.

    #define MAKE_OBSERVED() \
    template <std::size_t I> \
    inline void invoke_observers() const {} \
    template <std::size_t I, class V> \
    inline auto invoke_validators(V&& r) const { return r; }

    /*************************
     * XOBSERVE_STATIC macro *
     *************************/

    // XOBSERVE_STATIC(Type, Owner, Name)
    //
    // Set up the static notifier for the specified property

    #define XOBSERVE_STATIC(T, O, D) \
    template <> \
    inline void O::invoke_observers<xoffsetof(O, D)>() const

    /**************************
     * XVALIDATE_STATIC macro *
     **************************/

    // XVALIDATE_STATIC(Type, Owner, Name, Proposal Argument Name)
    //
    // Set up the static validator for the specified property

    #define XVALIDATE_STATIC(T, O, D, A) \
    template <> \
    inline auto O::invoke_validators<xoffsetof(O, D), T>(T&& A) const

    /****************************
     * xproperty implementation *
     ****************************/

    template <class T, class O, class D>
    inline xproperty<T, O, D>::xproperty() noexcept(noexcept(std::is_nothrow_constructible<value_type>::value))
        : m_value()
    {
    }
    
    template <class T, class O, class D>
    inline xproperty<T, O, D>::xproperty(value_type&& value) noexcept(noexcept(std::is_nothrow_move_constructible<value_type>::value))
        : m_value(value)
    {
    }
    
    template <class T, class O, class D>
    inline xproperty<T, O, D>::xproperty(const_reference value) noexcept(noexcept(std::is_nothrow_constructible<value_type>::value))
        : m_value(value)
    {
    }

    template <class T, class O, class D>
    inline xproperty<T, O, D>::operator reference() noexcept
    {
        return m_value;
    }

    template <class T, class O, class D>
    inline xproperty<T, O, D>::operator const_reference() const noexcept
    {
        return m_value;
    }

    template <class T, class O, class D>
    template <class V>
    inline auto xproperty<T, O, D>::operator=(V&& value) -> reference
    {
        m_value = owner()->template invoke_validators<derived_type::offset()>(std::forward<V>(value));
        owner()->template invoke_observers<derived_type::offset()>();
        return m_value;
    }

    template <class T, class O, class D>
    inline auto xproperty<T, O, D>::owner() noexcept -> owner_type*
    {
        return reinterpret_cast<owner_type*>(reinterpret_cast<char*>(this) - derived_type::offset());
    }
}

#endif

