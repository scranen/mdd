#ifndef __scranen_mdd_operations_set_count_h
#define __scranen_mdd_operations_set_count_h

#include "node_factory.h"
#include "add_element.h"

namespace mdd
{

template <typename Value>
struct node_factory<Value>::mdd_set_count
{
    typedef node_factory<Value> factory_type;
    typedef typename factory_type::node_ptr node_ptr;

    factory_type& m_factory;

    mdd_set_count(factory_type& factory)
        : m_factory(factory)
    { }

    double operator()(node_ptr p)
    {
        if (p == m_factory.empty())
            return 0;
        if (p == m_factory.emptylist())
            return 1;
        return operator()(p->down) + operator()(p->right);
    }
};

}

#endif // __scranen_mdd_operations_set_count_h
