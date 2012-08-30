#ifndef __scranen_mdd_mdd_factory_h
#define __scranen_mdd_mdd_factory_h

#include "node_factory.h"
#include "operations/add_element.h"
#include "operations/set_union.h"
#include "operations/set_intersect.h"

namespace mdd
{

template <typename Value>
class mdd;

template <typename Value>
class mdd_factory : public node_factory<Value>
{
public:
    friend class mdd<Value>;

    typedef node_factory<Value> parent;
    typedef mdd<Value> mdd_type;
    typedef const node<Value>* node_ptr;

    /**
     * @brief Returns an empty MDD.
     * @return An mdd::mdd representing the empty set.
     */
    mdd_type empty() { return mdd_type(this, parent::empty()); }

    // For debugging purposes:
    void print_nodes(std::ostream& s)
    {
        parent::print_nodes(s);
    }
    void print_nodes(std::ostream& s, const mdd_type& hint1)
    {
        parent::print_nodes(s, hint1.m_node);
    }
    void print_nodes(std::ostream& s, const mdd_type& hint1, const mdd_type& hint2)
    {
        parent::print_nodes(s, hint1.m_node, hint2.m_node);
    }

    std::string print_nodes()
    {
        std::stringstream s;
        print_nodes(s);
        return s.str();
    }

    std::string print_nodes(const mdd_type& hint1)
    {
        std::stringstream s;
        print_nodes(s, hint1);
        return s.str();
    }

    std::string print_nodes(const mdd_type& hint1, const mdd_type& hint2)
    {
        std::stringstream s;
        print_nodes(s, hint1, hint2);
        return s.str();
    }
};

} // namespace mdd

#endif // __scranen_mdd_mdd_factory_h
