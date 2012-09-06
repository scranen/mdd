#ifndef __scranen_mdd_operations_rel_prev_h
#define __scranen_mdd_operations_rel_prev_h

#include <assert.h>
#include "node_factory.h"

namespace mdd
{

template <typename Value>
struct node_factory<Value>::mdd_rel_prev
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

    mdd_rel_prev(factory_type& factory)
        : m_factory(factory)
    { }

    node_ptr operator()(node_ptr r, node_ptr s)
    {
        if (r->sentinel())
            return r;
        if (s->sentinel())
            return s;

        node_ptr result;
        if (m_factory.m_cache.lookup(cache_rel_prev, r, s, result))
            return result->use();

        node_ptr down = collect(r->down, s);
        if (down != m_factory.empty())
            result = m_factory.create(r->value, operator()(r->right, s), down);
        else
            result = operator()(r->right, s);

        m_factory.m_cache.store(cache_rel_prev, r, s, result);
        return result;
    }

    node_ptr collect(node_ptr r, node_ptr s)
    {
        assert(r != m_factory.emptylist());
        assert(s != m_factory.emptylist());
        if (r == m_factory.empty())
            return r;
        if (s == m_factory.empty())
            return s;
        if (r->value < s->value)
            return collect(r->right, s);
        if (r->value > s->value)
            return collect(r, s->right);

        node_ptr down = operator()(r->down, s->down);
        node_ptr right = collect(r->right, s->right);
        node_ptr result = typename factory_type::mdd_set_union(m_factory)(down, right);
        down->unuse();
        right->unuse();
        return result;
    }
};

}

#endif // __scranen_mdd_operations_rel_prev_h
