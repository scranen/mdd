#ifndef __scranen_mdd_operations_rel_next_h
#define __scranen_mdd_operations_rel_next_h

#include <assert.h>
#include "node_factory.h"

namespace mdd
{

template <typename Value>
struct node_factory<Value>::mdd_rel_next
{
    typedef node_factory<Value> factory_type;
    typedef typename factory_type::node_ptr node_ptr;
    typedef typename factory_type::cache_type cache_type;

    enum composition_type
    {
        interleaved_interleaved,
        interleaved_sequential
    };

    factory_type& m_factory;

    mdd_rel_next(factory_type& factory)
        : m_factory(factory)
    { }

    // Compute states reachable from s using one step of interleaved relation r
    node_ptr operator()(node_ptr r, node_ptr s)
    {
        return next(r, s);
    }
private:

    node_ptr next(node_ptr r, node_ptr s)
    {
        assert(r != m_factory.empty());

        if (r->sentinel())
            return r;
        if (s->sentinel())
            return s;

        if (s->value < r->value)
            return operator()(r, s->right);
        if (s->value > r->value)
            return operator()(r->right, s);
        return collect(r->down, s);

    }

    node_ptr collect(node_ptr r, node_ptr s)
    {
        assert(!s->sentinel());

        if (r->sentinel())
            return r;

        return m_factory.create(r->value, collect(r->right, s), next(r->down, s->down));
    }

};

}

#endif // __scranen_mdd_operations_rel_next_h