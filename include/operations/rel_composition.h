#ifndef __scranen_mdd_operations_rel_composition_h
#define __scranen_mdd_operations_rel_composition_h

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

    node_ptr collect_i_i(node_ptr left, node_ptr right)
    {
        if (right->sentinel())
            return right;
        return m_factory.create(right->value, collect_i_i(left, right->right), apply_i_i(left, right->down));
    }

    node_ptr match_i_i(node_ptr left, node_ptr right)
    {
        if (left->sentinel() || right->sentinel())
            return m_factory.empty();
        if (left->value < right->value)
            return match_i_i(left->right, right);
        if (left->value > right->value)
            return match_i_i(left, right->right);

        node_ptr tmp1 = match_i_i(left->right, right->right);
        node_ptr tmp2 = collect_i_i(left->down, right->down);
        node_ptr result = factory_type::mdd_set_union(m_factory)(tmp1, tmp2);
        tmp1->unuse();
        tmp2->unuse();
        return result;
    }

    // Compose interleaved relation with interleaved relation
    node_ptr apply_i_i(node_ptr left, node_ptr right)
    {
        node_ptr result;
        order(left, right);

        if (left->sentinel())
            return left;

        assert(right != m_factory.emptylist());

        if (m_factory.m_cache.lookup(cache_rel_composition, left, right, result))
            return result->use();

        node_ptr r_right = apply_i_i(left->right, right);
        node_ptr r_down = match_i_i(left->down, right);
        if (r_down != m_factory.empty())
        {
            result = m_factory.create(left->value, r_right, r_down);
        }
        else
            result = r_right;

        m_factory.m_cache.store(cache_rel_composition, left, right, result);
        return result;
    }

    // Compose interleaved relation with non-interleaved relation
    node_ptr operator()(node_ptr a, node_ptr b, composition_type t)
    {
        switch (t)
        {
        case interleaved_interleaved:
            return apply_i_i(a, b);
        default:
            assert(false);
        }
    }
};

}

#endif // __scranen_mdd_operations_rel_composition_h
