#ifndef __scranen_mdd_operations_rel_next_h
#define __scranen_mdd_operations_rel_next_h

#include <assert.h>
#include "node_factory.h"
#include "projection.h"

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

    // Compute states reachable from s using one step of interleaved partial relation r
    node_ptr operator()(node_ptr r, node_ptr s, const projection& proj)
    {
        if (proj.full())
            return next(r, s);
        return next(r, s, proj.begin(), proj.end());
    }
private:

    /*
     * Version with projection
     */

    node_ptr next(node_ptr r, node_ptr s, projection::iterator pbegin, projection::iterator pend)
    {
        if (r == m_factory.empty())
            return r;
        if (r == m_factory.emptylist())
            return s->use();
        if (s->sentinel())
            return s;

        node_ptr result;
        projection::iterator oldbegin = pbegin;
        if (m_factory.m_cache.lookup(cache_rel_next, r, s, oldbegin.node(), result))
            return result->use();

        if (pbegin == pend || !*pbegin)
            result = collect_wildcard(r, s, ++pbegin, pend);
        else
        if (s->value < r->value)
            result = next(r, s->right, pbegin, pend);
        else
        if (s->value > r->value)
            result = next(r->right, s, pbegin, pend);
        else
        {
            node_ptr right = next(r->right, s->right, pbegin, pend);
            node_ptr down = collect(r->down, s, ++pbegin, pend);
            result = typename factory_type::mdd_set_union(m_factory)(right, down);
            right->unuse();
            down->unuse();
        }

        m_factory.m_cache.store(cache_rel_next, r, s, oldbegin.node(), result);
        return result;
    }

    node_ptr collect_wildcard(node_ptr r, node_ptr s, projection::iterator pbegin, projection::iterator pend)
    {
        if (s->sentinel())
            return s;

        return m_factory.create(s->value, collect_wildcard(r, s->right, pbegin, pend), next(r, s->down, pbegin, pend));
    }

    node_ptr collect(node_ptr r, node_ptr s, projection::iterator pbegin, projection::iterator pend)
    {
        assert(!s->sentinel());

        if (r->sentinel())
            return r;

        return m_factory.create(r->value, collect(r->right, s, pbegin, pend), next(r->down, s->down, pbegin, pend));
    }

    /*
     * Version without projection
     */

    node_ptr next(node_ptr r, node_ptr s)
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

        if (s->value < r->value)
            result = next(r, s->right);
        else
        if (s->value > r->value)
            result = next(r->right, s);
        else
        {
            node_ptr right = next(r->right, s->right);
            node_ptr down = collect(r->down, s);
            result = typename factory_type::mdd_set_union(m_factory)(right, down);
            right->unuse();
            down->unuse();
        }

        m_factory.m_cache.store(cache_rel_next, r, s, result);
        return result;
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
