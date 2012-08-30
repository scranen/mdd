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
        if (a == m_factory.empty() || b == m_factory.empty())
            return a;

        if (m_factory.m_cache.lookup(cache_set_intersect, a, b, result))
            return result->use();

        /*
         * TODO: implement this.
         */

        m_factory.m_cache.store(cache_set_intersect, a, b, result);
        return result;
    }
};

}

#endif // __scranen_mdd_operations_set_intersect_h
