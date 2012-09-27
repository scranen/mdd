#ifndef __scranen_mdd_operations_set_minus_h
#define __scranen_mdd_operations_set_minus_h

#include "node_factory.h"
#include "add_element.h"

namespace mdd
{

template <typename Value>
struct node_factory<Value>::mdd_set_minus
{
    typedef node_factory<Value> factory_type;
    typedef typename factory_type::node_ptr node_ptr;
    typedef typename factory_type::cache_type cache_type;
    typedef factory_type::mdd_add_element add_element;

    factory_type& m_factory;

    mdd_set_minus(factory_type& factory)
        : m_factory(factory)
    { }

    node_ptr operator()(node_ptr a, node_ptr b)
    {
        if (a == b)
            return m_factory.empty();
        if (a == m_factory.empty())
            return a;
        if (b == m_factory.empty())
            return a->use();
        if (a == m_factory.emptylist())
            return operator()(a, b->right);
        if (b == m_factory.emptylist())
            return m_factory.create(a->value, operator()(a->right, b), a->down->use());

        node_ptr result;
        if (m_factory.m_cache.lookup(cache_set_minus, a, b, result))
            return result->use();

        if (a->sentinel() || a->value > b->value)
            result = operator()(a, b->right);
        else
        if (a->value < b->value)
            result = m_factory.create(a->value, operator()(a->right, b), a->down->use());
        else // a->value == b->value
        {
            result = operator()(a->down, b->down);
            if (result != m_factory.empty())
                result = m_factory.create(a->value, operator()(a->right, b->right), result);
            else
                result = operator()(a->right, b->right);
        }

        m_factory.m_cache.store(cache_set_minus, a, b, result);
        return result;
    }
};

}

#endif // __scranen_mdd_operations_set_minus_h
