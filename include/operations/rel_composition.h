#ifndef __scranen_mdd_operations_rel_composition_h
#define __scranen_mdd_operations_rel_composition_h

#include <assert.h>
#include "node_factory.h"
#include "set_union.h"

namespace mdd
{

template <typename Value>
struct node_factory<Value>::mdd_rel_composition
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

    mdd_rel_composition(factory_type& factory)
        : m_factory(factory)
    { }

    // Compose interleaved relation with non-interleaved relation
    node_ptr operator()(node_ptr a, node_ptr b, composition_type t)
    {
        switch (t)
        {
        case interleaved_interleaved:
            return compose_i_i(a, b);
        case interleaved_sequential:
            return compose_i_s(a, b);
        default:
            assert(false);
        }
    }
private:
    //
    // Implementation of interleaved-interleaved composition
    //

    node_ptr collect_i_i(node_ptr a, node_ptr b)
    {
        if (b->sentinel())
            return b;
        return m_factory.create(b->value, collect_i_i(a, b->right), compose_i_i(a, b->down));
    }

    node_ptr match_i_i(node_ptr a, node_ptr b)
    {
        if (a->sentinel() || b->sentinel())
            return m_factory.empty();
        if (a->value < b->value)
            return match_i_i(a->right, b);
        if (a->value > b->value)
            return match_i_i(a, b->right);

        node_ptr tmp1 = match_i_i(a->right, b->right);
        node_ptr tmp2 = collect_i_i(a->down, b->down);
        node_ptr result = factory_type::mdd_set_union(m_factory)(tmp1, tmp2);
        tmp1->unuse();
        tmp2->unuse();
        return result;
    }

    node_ptr compose_i_i(node_ptr a, node_ptr b)
    {
        node_ptr result;

        if (a->sentinel())
            return a;
        assert(b != m_factory.emptylist());
        if (b->sentinel())
            return match_i_i(a->down, b);

        if (m_factory.m_cache.lookup(cache_rel_composition_i_i, a, b, result))
            return result->use();

        node_ptr r_right = compose_i_i(a->right, b);
        node_ptr r_down = match_i_i(a->down, b);
        if (r_down != m_factory.empty())
        {
            result = m_factory.create(a->value, r_right, r_down);
        }
        else
            result = r_right;

        m_factory.m_cache.store(cache_rel_composition_i_i, a, b, result);
        return result;
    }

    //
    // Implementation of interleaved-sequential composition
    //
    node_ptr match_i_s(node_ptr a, node_ptr b)
    {
        assert(b != m_factory.emptylist());

        if (a == m_factory.empty())
            return a;
        if (b == m_factory.empty())
            return b;
        if (a->value < b->value)
            return match_i_s(a->right, b);
        if (a->value > b->value)
            return match_i_s(a, b->right);

        node_ptr tmp1 = match_i_s(a->right, b->right);
        node_ptr tmp2 = compose_i_s(a->down, b->down);
        node_ptr result = factory_type::mdd_set_union(m_factory)(tmp1, tmp2);
        tmp1->unuse();
        tmp2->unuse();
        return result;
    }

    node_ptr compose_i_s(node_ptr a, node_ptr b)
    {
        node_ptr result;

        if (a == m_factory.empty())
            return a;
        if (a == m_factory.emptylist())
            return b->use();

        assert(b != m_factory.emptylist());

        if (m_factory.m_cache.lookup(cache_rel_composition_i_s, a, b, result))
            return result->use();

        node_ptr r_right = compose_i_s(a->right, b);
        node_ptr r_down = match_i_s(a->down, b);
        if (r_down != m_factory.empty())
        {
            result = m_factory.create(a->value, r_right, r_down);
        }
        else
            result = r_right;

        m_factory.m_cache.store(cache_rel_composition_i_s, a, b, result);
        return result;
    }

    //
    // Implementation of interleaved-sequential composition with projection for a
    //
    node_ptr match_i_s(node_ptr a, node_ptr b, projection::iterator pbegin, projection::iterator pend, size_t level)
    {
        assert(b != m_factory.emptylist());

        if (a == m_factory.empty())
            return a;
        if (b == m_factory.empty())
            return b;
        if (a->value < b->value)
            return match_i_s(a->right, b);
        if (a->value > b->value)
            return match_i_s(a, b->right);

        node_ptr tmp1 = match_i_s(a->right, b->right, pbegin, pend, level);
        node_ptr tmp2 = compose_i_s(a->down, b->down, pbegin, pend, level);
        node_ptr result = factory_type::mdd_set_union(m_factory)(tmp1, tmp2);
        tmp1->unuse();
        tmp2->unuse();
        return result;
    }

    node_ptr collect_i_s(node_ptr a, node_ptr b, projection::iterator pbegin, projection::iterator pend, size_t level)
    {
        if (b == m_factory.empty())
            return b;

        node_ptr tmp1 = collect_i_s(a, b->right, pbegin, pend, level);
        node_ptr tmp2 = compose_i_s(a, b->down, pbegin, pend, level + 1);
        node_ptr result = factory_type::mdd_set_union(m_factory)(tmp1, tmp2);
        tmp1->unuse();
        tmp2->unuse();
        return result;
    }

    node_ptr compose_i_s(node_ptr a, node_ptr b, projection::iterator pbegin, projection::iterator pend, size_t level)
    {
        node_ptr result;

        if (a == m_factory.empty())
            return a;
        if (a == m_factory.emptylist() || pbegin == pend)
            return b->use();

        assert(b != m_factory.emptylist());

        if (m_factory.m_cache.lookup(cache_rel_composition_i_s, a, b, pbegin, result))
            return result->use();

        if (*pbegin == level)
        {
            projection::iterator newbegin = pbegin;
            ++newbegin;
            node_ptr r_right = compose_i_s(a->right, b, pbegin, pend, level);
            node_ptr r_down = match_i_s(a->down, b, newbegin, pend, level + 1);
            if (r_down != m_factory.empty())
            {
                result = m_factory.create(a->value, r_right, r_down);
            }
            else
                result = r_right;
        }
        else
        {
            result = collect_i_s(a, b, pbegin, pend, level);
        }

        m_factory.m_cache.store(cache_rel_composition_i_s, a, b, pbegin, result);
        return result;
    }
};

}

#endif // __scranen_mdd_operations_rel_composition_h
