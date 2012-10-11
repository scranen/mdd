#ifndef __scranen_mdd_operations_set_match_proj_h
#define __scranen_mdd_operations_set_match_proj_h

#include "node_factory.h"
#include "add_element.h"
#include "projection.h"

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
    node_ptr operator()(node_ptr p, projection::iterator pbegin, projection::iterator pend, iterator vbegin) const
    {
        return match(p, pbegin, pend, vbegin);
    }
private:
    template <typename iterator>
    node_ptr match(node_ptr p, projection::iterator pbegin, projection::iterator pend, iterator vbegin) const
    {
        if (pbegin == pend)
            return p->use();
        if (*pbegin)
        {
            if (p->value < *vbegin)
                return match(p->right, pbegin, pend, vbegin);
            if (p->value > *vbegin)
                return m_factory.empty();
            ++pbegin;
            ++vbegin;
            return m_factory.create(p->value, m_factory.empty(), match(p->down, pbegin, pend, vbegin));
        }
        else
        {
            node_ptr right = match(p->right, pbegin, pend, vbegin);
            ++pbegin;
            node_ptr down = match(p->down, pbegin, pend, vbegin);
            return m_factory.create(p->value, right, down);
        }
    }
};

}

#endif // __scranen_mdd_operations_set_match_proj_h

