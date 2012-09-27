#ifndef __scranen_mdd_operations_set_project_h
#define __scranen_mdd_operations_set_project_h

#include "node_factory.h"
#include "set_union.h"
#include "projection.h"

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

    node_ptr operator()(node_ptr a, const projection& projection)
    {
        return project(a, projection.begin(), projection.end(), 0);
    }
private:
    template <typename iterator>
    node_ptr project(node_ptr p, iterator begin, const iterator& end, size_t level)
    {
        if (begin == end)
            return m_factory.emptylist();
        if (p->sentinel())
            return m_factory.empty();

        if (*begin == level)
        {
            iterator oldbegin = begin++;
            return m_factory.create(p->value, project(p->right, oldbegin, end, level), project(p->down, begin, end, level + 1));
        }
        return collect(p, begin, end, level);
    }

    template <typename iterator>
    node_ptr collect(node_ptr p, iterator& begin, const iterator& end, size_t level)
    {
        if (p->sentinel())
            return p;
        return typename factory_type::mdd_set_union(m_factory)(project(p->down, begin, end, level + 1), collect(p->right, begin, end, level));
    }
};

}

#endif // __scranen_mdd_operations_set_project_h
