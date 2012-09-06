#ifndef __scranen_mdd_operations_rel_prev_h
#define __scranen_mdd_operations_rel_prev_h

#include <assert.h>
#include "node_factory.h"

namespace mdd
{

template <typename Value>
struct node_factory<Value>::mdd_rel_prev
{
    typedef node_factory<Value> factory_type;
    typedef typename factory_type::node_ptr node_ptr;
    typedef typename factory_type::cache_type cache_type;

    enum composition_type
    {
        interleaved_interleaved,
        interleaved_sequential
    };

    factory_type& m_factory;

    mdd_rel_prev(factory_type& factory)
        : m_factory(factory)
    { }

    // Compose interleaved relation with non-interleaved relation
    node_ptr operator()(node_ptr r, node_ptr s)
    {
        // TODO
    }
};

}

#endif // __scranen_mdd_operations_rel_prev_h
