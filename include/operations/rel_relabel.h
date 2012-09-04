#ifndef __scranen_mdd_operations_rel_relabel_h
#define __scranen_mdd_operations_rel_relabel_h

#include <unordered_map>
#include "mdd.h"

namespace mdd
{

template <typename Value>
struct node_factory<Value>::mdd_rel_relabel
{
    typedef node_factory<Value> factory_type;
    typedef mdd<Value> mdd_type;
    typedef typename factory_type::node_ptr node_ptr;
    typedef typename factory_type::node_type node_type;
    typedef typename factory_type::cache_type cache_type;    

    factory_type& m_factory;
    std::unordered_map<node_ptr, node_ptr> m_cache;

    mdd_rel_relabel(factory_type& factory)
        : m_factory(factory)
    { }

    template <typename generator>
    node_ptr operator()(node_ptr a, generator& g)
    {
        return replace(a, g, 0);
    }
private:

    template <typename generator>
    node_ptr replace(node_ptr a, generator& g, size_t level)
    {
        assert(a != m_factory.emptylist());
        mdd_type m(&m_factory, a->use());
        if (g.match(level, m))
        {
            auto it = m_cache.find(a);
            if (it != m_cache.end())
                return it->second->use();
            node_ptr result = g.replace(level, m).m_node;
            m_cache[a] = result;
            return result->use();
        }
        if (a == m_factory.empty())
            return a;
        return m_factory.create(a->value, replace(a->right, g, level), replace(a->down, g, level + 1));
    }

};

}

#endif // __scranen_mdd_operations_rel_relabel_h
