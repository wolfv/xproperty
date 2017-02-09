/***************************************************************************
* Copyright (c) 2016, Johan Mabille and Sylvain Corlay                     *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XOBSERVED_HPP
#define XOBSERVED_HPP

#include <type_traits>
#include <cstddef>
#include <unordered_map>
#include <vector>
#include <functional>

#include "xproperty.hpp"
#include "any.hpp"

namespace xp
{

    // XOBSERVE(owner, Attribute, Callback)
    // Register a callback reacting to changes of the specified attribute of the owner.

    #define XOBSERVE(O, A, C) \
    O.observe<xoffsetof(decltype(O), A)>(C);

    // XUNOBSERVE(owner, Attribute)
    // Removes all callbacks reacting to changes of the specified attribute of the owner.

    #define XUNOBSERVE(O, A) \
    O.unobserve<xoffsetof(decltype(O), A)>();

    // XVALIDATE(owner, Attribute, Validator)
    // Register a validator for proposed values of the specified attribute.

    #define XVALIDATE(O, A, C) \
    O.validate<xoffsetof(decltype(O), A)>(std::function<typename decltype(O.A)::value_type(const decltype(O)&, typename decltype(O.A)::value_type)>(C));

    // XUNVALIDATE(owner, Attribute)
    // Removes all validators for proposed values of the specified attribute.

    #define XUNVALIDATE(O, A) \
    O.unvalidate<xoffsetof(decltype(O), A)>();

    // XDLINK(Source, AttributeName, Target, AttributeName)
    // Link the value of an attribute of a source xobserved object with the value of a target object.

    #define XDLINK(S, SA, T, TA) \
    T.TA = S.SA;\
    S.observe<xoffsetof(decltype(S), SA)>([&S, &T] (const auto&) { T.TA = S.SA; });

    // XLINK(Source, AttributeName, Target, AttributeName)
    // Bidirectional link between attributes of two xobserved objects.

    #define XLINK(S, SA, T, TA) \
    T.TA = S.SA;\
    S.observe<xoffsetof(decltype(S), SA)>([&S, &T] (const auto&) { T.TA = S.SA; });\
    T.observe<xoffsetof(decltype(T), TA)>([&S, &T] (const auto&) { S.SA = T.TA; });

    /*************************
     * xobserved declaration *
     *************************/

    template <class D>
    class xobserved
    {
    public:

        using derived_type = D;

        derived_type& derived_cast() noexcept;
        const derived_type& derived_cast() const noexcept;

        template <std::size_t I>
        void observe(std::function<void(const derived_type&)> cb);

        template <std::size_t I>
        void unobserve();

        template <std::size_t I, class V>
        void validate(std::function<V(const derived_type&, V)> cb);

        template <std::size_t I>
        void unvalidate();

    protected:

        xobserved() = default;
        ~xobserved() = default;

        xobserved(const xobserved&) = default;
        xobserved& operator=(const xobserved&) = default;

        xobserved(xobserved&&) = default;
        xobserved& operator=(xobserved&&) = default;

    private:

        std::unordered_map<std::size_t, std::vector<std::function<void(const derived_type&)>>> m_observers;
        std::unordered_map<std::size_t, std::vector<linb::any>> m_validators;
    
        template <class X, class Y, class Z>
        friend class xproperty;

        template <std::size_t I>
        void invoke_observers() const;
        
        template <std::size_t I, class V>
        auto invoke_validators(V&& r) const;
    };

    template <class E>
    using is_xobserved = std::is_base_of<xobserved<E>, E>;

    /****************************
     * xobserved implementation *
     ****************************/

    template <class D>
    inline auto xobserved<D>::derived_cast() noexcept -> derived_type&
    {
        return *static_cast<derived_type*>(this);
    }

    template <class D>
    inline auto xobserved<D>::derived_cast() const noexcept -> const derived_type&
    {
        return *static_cast<const derived_type*>(this);
    }

    template <class D>
    template <std::size_t I>
    inline void xobserved<D>::observe(std::function<void(const derived_type&)> cb)
    {
        auto position = m_observers.find(I);
        if(position == m_observers.end())
        {
            m_observers[I] = { std::move(cb) };
        }
        else
        {
            position->second.push_back(std::move(cb));
        }
    }

    template <class D>
    template <std::size_t I>
    inline void xobserved<D>::unobserve()
    {
        m_observers.erase(I);
    }

    template <class D>
    template <std::size_t I, class V>
    inline void xobserved<D>::validate(std::function<V(const derived_type&, V)> cb)
    {
        auto position = m_validators.find(I);
        if(position == m_validators.end())
        {
            m_validators[I] = { std::move(cb) };
        }
        else
        {
            position->second.push_back(std::move(cb));
        }
    }

    template <class D>
    template <std::size_t I>
    inline void xobserved<D>::unvalidate()
    {
        m_validators.erase(I);
    }

    template <class D>
    template <std::size_t I>
    inline void xobserved<D>::invoke_observers() const
    {
        auto position = m_observers.find(I);
        if(position != m_observers.end())
        {
            const auto& callbacks = position->second;
            for (auto it = callbacks.cbegin(); it != callbacks.cend(); ++it) 
            {
                it->operator()(derived_cast()); 
            }
        }
    }
    
    template <class D>
    template <std::size_t I, class V>
    inline auto xobserved<D>::invoke_validators(V&& v) const
    {
        auto position = m_validators.find(I);
        if(position != m_validators.end())
        {
            const auto& callbacks = position->second;
            for (auto it = callbacks.cbegin(); it != callbacks.cend(); ++it) 
            {
                v = linb::any_cast<std::function<V(const derived_type&, V)>>(*it)(derived_cast(), std::forward<V>(v));
            }
        }
        return v;
    }

}

#endif

