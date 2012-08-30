#ifndef __scranen_mdd_operations_add_element_h
#define __scranen_mdd_operations_add_element_h

#include "node_factory.h"

namespace mdd
{

template <typename Value>
struct node_factory<Value>::mdd_add_element
{
    typedef node_factory<Value> factory_type;
    typedef typename factory_type::node_ptr node_ptr;
    typedef typename factory_type::cache_type cache_type;

    factory_type& m_factory;

    mdd_add_element(factory_type& factory)
        : m_factory(factory)
    { }

    template <typename iterator>
    node_ptr operator()(node_ptr a, iterator begin, iterator end)
    {
        node_ptr result;
        node_ptr temp;
        if (begin == end)
        {
            if (a->sentinel())
                return m_factory.emptylist();
            temp = operator()(a->right, begin, end);
            result = m_factory.create(a->value, temp, a->down);
        }
        else
        if (a->sentinel() || a->value > *begin)
        {
          temp = operator()(m_factory.empty(), begin + 1, end);
          result = m_factory.create(*begin, a, temp);
        }
        else
        {
            if (a->value == *begin)
            {
                temp = operator()(a->down, begin + 1, end);
                result = m_factory.create(a->value, a->right, temp);
            }
            else
            if (a->value < *begin)
            {
                temp = operator()(a->right, begin, end);
                result = m_factory.create(a->value, temp, a->down);
            }
        }
        temp->unuse();
        return result;
    }
};

}

#endif // __scranen_mdd_operations_add_element_h
