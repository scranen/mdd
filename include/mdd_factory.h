#ifndef __scranen_mdd_mdd_factory_h
#define __scranen_mdd_mdd_factory_h

#include "node_factory.h"

namespace mdd
{

template <typename Value>
class mdd;
template <typename Value>
class mdd_irel;
template <typename Value>
class mdd_srel;

template <typename Value>
class mdd_factory : public node_factory<Value>
{
public:
    typedef node_factory<Value> parent;
    typedef mdd<Value> set_type;
    typedef mdd_irel<Value> irel_type;
    typedef mdd_srel<Value> srel_type;
    typedef const node<Value>* node_ptr;

    friend class mdd<Value>;
    friend class mdd_irel<Value>;
    friend class mdd_srel<Value>;


    /**
     * @brief Returns an empty MDD.
     * @return An mdd::mdd representing the empty set.
     */
    irel_type empty_irel() { return irel_type(this, parent::empty()); }

    /**
     * @brief Returns an empty MDD.
     * @return An mdd::mdd representing the empty set.
     */
    srel_type empty_srel() { return srel_type(this, parent::empty()); }

    /**
     * @brief Returns an empty MDD.
     * @return An mdd::mdd representing the empty set.
     */
    set_type empty_set() { return set_type(this, parent::empty()); }

    /**
     * @brief Returns an MDD that only contains the empty list.
     * @return An mdd::mdd containing only the empty list.
     */
    set_type singleton_set() { return set_type(this, parent::emptylist()); }

    // For debugging purposes:
    void print_nodes(std::ostream& s)
    {
        parent::print_nodes(s);
    }
    void print_nodes(std::ostream& s, const set_type& hint1)
    {
        parent::print_nodes(s, hint1.m_node);
    }
    void print_nodes(std::ostream& s, const set_type& hint1, const set_type& hint2)
    {
        parent::print_nodes(s, hint1.m_node, hint2.m_node);
    }

    std::string print_nodes()
    {
        std::stringstream s;
        print_nodes(s);
        return s.str();
    }

    std::string print_nodes(const set_type& hint1)
    {
        std::stringstream s;
        print_nodes(s, hint1);
        return s.str();
    }

    std::string print_nodes(const set_type& hint1, const set_type& hint2)
    {
        std::stringstream s;
        print_nodes(s, hint1, hint2);
        return s.str();
    }
};

} // namespace mdd

#endif // __scranen_mdd_mdd_factory_h
