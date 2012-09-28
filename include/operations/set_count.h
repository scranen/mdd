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
        size_t nodes = 0;
        return operator()(p, nodes);
    }

    double operator()(node_ptr p, size_t& nodes)
    {
        double result = 0;
        const_cast<typename factory_type::node_type*>(m_factory.empty())->right = nullptr;
        const_cast<typename factory_type::node_type*>(m_factory.emptylist())->right = nullptr;
        count(p, result, nodes);
        clean(p);
        return result;
    }
private:
    void count(node_ptr p, double& paths, size_t& nodes)
    {
        if (p == m_factory.emptylist())
        {
            paths += 1;
        }
        if (!((size_t)p->right & 1))
        {
            node_ptr right = p->right;
            const_cast<typename factory_type::node_type*&>(p)->right = (node_ptr)((uintptr_t)right | 1);
            ++nodes;
            if (!p->sentinel())
            {
                count(right, paths, nodes);
                count(p->down, paths, nodes);
            }
        }
        else
        {
            if (!p->sentinel())
            {
                count((node_ptr)((uintptr_t)p->right ^ 1), paths, nodes);
                count(p->down, paths, nodes);
            }
        }
    }

    void clean(node_ptr p)
    {
        if (!((uintptr_t)p->right & 1))
            return;
        const_cast<typename factory_type::node_type*&>(p)->right = (node_ptr)((uintptr_t)p->right ^ 1);
        if (p->sentinel())
            return;
        clean(p->right);
        clean(p->down);
    }
};

}

#endif // __scranen_mdd_operations_set_count_h
