#ifndef __scranen_mdd_operations_set_union_h
#define __scranen_mdd_operations_set_union_h

#include "node_factory.h"
#include "add_element.h"

namespace mdd
{

template <typename Value>
struct node_factory<Value>::mdd_set_union
{
    typedef node_factory<Value> factory_type;
    typedef typename factory_type::node_ptr node_ptr;
    typedef typename factory_type::cache_type cache_type;
    typedef factory_type::mdd_add_element add_element;

    factory_type& m_factory;

    mdd_set_union(factory_type& factory)
        : m_factory(factory)
    { }

    node_ptr operator()(node_ptr a, node_ptr b)
    {
        node_ptr result;
        order(a, b);

        if (a == b || b == m_factory.empty())
            return a->use();
        if (a == m_factory.empty())
            return b->use();
        if (a == m_factory.emptylist())
            return add_element(m_factory)(b);
        if (b == m_factory.emptylist())
            return add_element(m_factory)(a);

        if (m_factory.m_cache.lookup(cache_set_union, a, b, result))
            return result->use();

        if (a->value < b->value)
            result = m_factory.create(a->value, operator()(a->right, b), a->down->use());
        else
        if (a->value > b->value)
            result = m_factory.create(b->value, operator()(a, b->right), b->down->use());
        else
            result = m_factory.create(a->value, operator()(a->right, b->right), operator()(a->down, b->down));

        m_factory.m_cache.store(cache_set_union, a, b, result);
        return result;
    }
};

}

#endif // __scranen_mdd_operations_set_union_h
