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
    node_ptr project(node_ptr p, const projection::iterator& begin, const projection::iterator& end, size_t level)
    {
        if (begin == end)
            return m_factory.emptylist();
        if (p->sentinel())
            return m_factory.empty();

        node_ptr result;
        if (m_factory.m_cache.lookup(cache_set_project, p, nullptr, begin.node(), result))
            return result->use();

        if (*begin == level)
        {
            projection::iterator newbegin = begin;
            ++newbegin;
            result = m_factory.create(p->value, project(p->right, begin, end, level), project(p->down, newbegin, end, level + 1));
        }
        else
            result = collect(p, begin, end, level);

        m_factory.m_cache.store(cache_set_project, p, nullptr, begin.node(), result);

        return result;
    }

    node_ptr collect(node_ptr p, const projection::iterator& begin, const projection::iterator& end, size_t level)
    {
        if (p->sentinel())
            return p;
        return typename factory_type::mdd_set_union(m_factory)(project(p->down, begin, end, level + 1), collect(p->right, begin, end, level));
    }
};

}

#endif // __scranen_mdd_operations_set_project_h
