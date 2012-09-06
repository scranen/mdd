#ifndef __scranen_mdd_operations_set_contains_h
#define __scranen_mdd_operations_set_contains_h

#include "node_factory.h"
#include "add_element.h"

namespace mdd
{

template <typename Value>
struct node_factory<Value>::mdd_set_contains
{
    typedef node_factory<Value> factory_type;
    typedef typename factory_type::node_ptr node_ptr;
    typedef typename factory_type::cache_type cache_type;
    typedef factory_type::mdd_add_element add_element;

    factory_type& m_factory;

    mdd_set_contains(factory_type& factory)
        : m_factory(factory)
    { }

    template <typename iterator>
    bool operator()(node_ptr p, iterator begin, iterator end) const
    {
        if (begin == end)
        {
            while (!p->sentinel())
                p = p->right;
            return p == m_factory.emptylist();
        }
        while (!p->sentinel() && p->value < *begin)
            p = p->right;
        if (p->value == *begin)
            return operator()(p->down, begin + 1, end);
        return false;
    }
};

}

#endif // __scranen_mdd_operations_set_contains_h

