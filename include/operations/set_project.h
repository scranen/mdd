#ifndef __scranen_mdd_operations_set_project_h
#define __scranen_mdd_operations_set_project_h

#include "node_factory.h"
#include "set_union.h"

namespace mdd
{

template <typename Value>
struct node_factory<Value>::mdd_set_project
{
    typedef node_factory<Value> factory_type;
    typedef typename factory_type::node_ptr node_ptr;

    factory_type& m_factory;

    mdd_set_project(factory_type& factory)
        : m_factory(factory)
    { }

    template <typename iterator>
    node_ptr operator()(node_ptr p, iterator begin, const iterator& end)
    {
        return project(p, begin, end, 0);
    }
private:
    template <typename iterator>
    node_ptr project(node_ptr p, iterator& begin, const iterator& end, int level)
    {
        if (p->sentinel())
            return p;
        if (begin == end)
            return m_factory.emptylist();
        if (*begin == level)
            return m_factory.create(p->value, project(p->right, begin, end, level), project(p->down, ++begin, end, level + 1));
        return collect(p, begin, end, level);
    }

    template <typename iterator>
    node_ptr collect(node_ptr p, iterator& begin, const iterator& end, int level)
    {
        if (p->sentinel())
            return p;
        return typename factory_type::mdd_set_union(m_factory)(project(p->down, begin, end, level + 1), collect(p->right, begin, end, level));
    }
};

}

#endif // __scranen_mdd_operations_set_project_h
