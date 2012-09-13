#ifndef __scranen_mdd_operations_set_match_proj_h
#define __scranen_mdd_operations_set_match_proj_h

#include "node_factory.h"
#include "add_element.h"

namespace mdd
{

template <typename Value>
struct node_factory<Value>::mdd_set_match_proj
{
    typedef node_factory<Value> factory_type;
    typedef typename factory_type::node_ptr node_ptr;
    typedef typename factory_type::cache_type cache_type;
    typedef factory_type::mdd_add_element add_element;

    factory_type& m_factory;

    mdd_set_match_proj(factory_type& factory)
        : m_factory(factory)
    { }

    template <typename iterator>
    node_ptr operator()(node_ptr p, iterator begin, iterator end) const
    {
        return match(p, begin, end, 0);
    }
private:
    template <typename iterator>
    node_ptr match(node_ptr p, iterator begin, iterator end, size_t level) const
    {
        if (begin == end)
            return p->use();
        assert(level >= *begin);
        if (level == *begin)
        {
            iterator oldbegin(begin);
            if (p->value < *++begin)
                return match(p->right, oldbegin, end, level);
            if (p->value > *begin)
                return m_factory.empty();
            return m_factory.create(p->value, m_factory.empty(), match(p->down, begin + 1, end, level + 1));
        }
        else
        {
            // TODO: make more efficient.
            return m_factory.create(p->value, match(p->right, begin, end, level), match(p->down, begin + 1, end, level + 1));
        }
    }
};

}

#endif // __scranen_mdd_operations_set_match_proj_h

