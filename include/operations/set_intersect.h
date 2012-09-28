#ifndef __scranen_mdd_operations_set_intersect_h
#define __scranen_mdd_operations_set_intersect_h

#include "node_factory.h"

namespace mdd
{

template <typename Value>
struct node_factory<Value>::mdd_set_intersect
{
    typedef node_factory<Value> factory_type;
    typedef typename factory_type::node_ptr node_ptr;
    typedef typename factory_type::cache_type cache_type;

    factory_type& m_factory;

    mdd_set_intersect(factory_type& factory)
        : m_factory(factory)
    { }

    node_ptr operator()(node_ptr a, node_ptr b)
    {
        node_ptr result;
        order(a, b);

        if (a == b)
            return a->use();
        if (a == m_factory.empty())
            return a;
        if (b == m_factory.empty())
            return b;
        if (a == m_factory.emptylist())
            return operator()(a, b->right);
        if (b == m_factory.emptylist())
            return operator()(a->right, b);

        if (m_factory.m_cache.lookup(cache_set_intersection, a, b, result))
            return result->use();

        if (a->value == b->value)
            result = m_factory.create(a->value, operator()(a->right, b->right), operator()(a->down, b->down));
        else
        if (a->value < b->value)
            result = operator()(a->right, b);
        else // (a->value > b->value)
            result = operator()(a, b->right);

        m_factory.m_cache.store(cache_set_intersection, a, b, result);
        return result;
    }
};

}

#endif // __scranen_mdd_operations_set_intersect_h
