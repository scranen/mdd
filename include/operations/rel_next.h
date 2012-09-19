#ifndef __scranen_mdd_operations_rel_next_h
#define __scranen_mdd_operations_rel_next_h

#include <assert.h>
#include "mdd_factory.h"

namespace mdd
{

template <typename Value>
struct node_factory<Value>::mdd_rel_next
{
    typedef mdd_factory<Value> factory_type;
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
        return next<false,size_t*>(r, s, nullptr, nullptr, 0);
    }

    // Compute states reachable from s using one step of interleaved partial relation r
    template <typename iterator>
    node_ptr operator()(node_ptr r, node_ptr s, iterator proj_begin, iterator proj_end)
    {
        return next<true,iterator>(r, s, proj_begin, proj_end, 0);
    }
private:

    template <bool use_projection, typename iterator>
    node_ptr next(node_ptr r, node_ptr s, iterator pbegin, iterator pend, size_t level)
    {
        if (r == m_factory.empty())
            return r;
        if (r == m_factory.emptylist())
            return s->use();
        if (s->sentinel())
            return s;

        node_ptr result;
        if (m_factory.m_cache.lookup(cache_rel_next, r, s, result))
            return result->use();

        if (use_projection && (pbegin == pend || *pbegin != level))
            result = collect_wildcard<use_projection,iterator>(r, s, pbegin, pend, level);
        else
        if (s->value < r->value)
            result = next<use_projection,iterator>(r, s->right, pbegin, pend, level);
        else
        if (s->value > r->value)
            result = next<use_projection,iterator>(r->right, s, pbegin, pend, level);
        else
            result = collect<use_projection,iterator>(r->down, s, ++pbegin, pend, level);

        m_factory.m_cache.store(cache_rel_next, r, s, result);
        return result;
    }

    template <bool use_projection, typename iterator>
    node_ptr collect_wildcard(node_ptr r, node_ptr s, iterator pbegin, iterator pend, size_t level)
    {
        if (s->sentinel())
            return s;

        return m_factory.create(s->value,
                                collect_wildcard<use_projection,iterator>(r, s->right, pbegin, pend, level),
                                next<use_projection,iterator>(r, s->down, pbegin, pend, level + 1));
    }

    template <bool use_projection, typename iterator>
    node_ptr collect(node_ptr r, node_ptr s, iterator pbegin, iterator pend, size_t level)
    {
        assert(!s->sentinel());

        if (r->sentinel())
            return r;

        return m_factory.create(r->value,
                                collect<use_projection,iterator>(r->right, s, pbegin, pend, level),
                                next<use_projection,iterator>(r->down, s->down, pbegin, pend, level + 1));
    }

};

}

#endif // __scranen_mdd_operations_rel_next_h
