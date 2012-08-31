#ifndef __scranen_mdd_mdd_h
#define __scranen_mdd_mdd_h

#include <mdd_iterator.h>
#include <mdd_factory.h>

namespace mdd
{

/**
 * @brief Generic MDD class.
 *
 * This class represents a set of vectors of elements of type Value. Instances are
 * created using an mdd::mdd_factory, or by copy-constructing other instances.
 * An mdd::mdd is actually a combination of a pointer to a factory, and a pointer
 * to a data structure created by that factory. Therefore, operations that use two
 * MDDs (e.g. set union) will fail if MDDs from different factories are used.
 *
 * Usage example:
\code
#include <string>
#include <vector>
#include <iostream>
#include "mdd.h"

int main(int argc, char** argv)
{
    mdd::mdd_factory<std::string> factory;
    mdd::mdd<std::string> m = factory.empty();
    std::vector<std::string> vec;

    vec.push_back("a");
    m += vec;
    vec.push_back("b");
    m += vec;

    for (auto v: m)
    {
      for (auto e: v)
      {
        std::cout << e << " ";
      }
      std::cout << std::endl;
    }
    return 0;
}
\endcode
 *
 * The code above would output the following:
\verbatim
a
a b
\endverbatim
 */
template <typename Value>
class mdd
{
public:
    friend class mdd_factory<Value>;

    typedef mdd_iterator<Value> iterator;
    typedef mdd_iterator<Value> const_iterator;
    typedef Value& reference;
    typedef const Value& const_reference;
    typedef mdd<Value> mdd_type;
    typedef mdd_factory<Value> factory_type;
    typedef factory_type* factory_ptr;
    typedef typename mdd_factory<Value>::node_ptr node_ptr;
protected:
    factory_ptr m_factory;
    node_ptr m_node;

    mdd(factory_ptr factory, node_ptr node)
        : m_factory(factory), m_node(node)
    {}

public:
    template<typename Functor, typename... Args>
    inline
    mdd_type apply(Args... args) const
    {
        return mdd_type(m_factory, Functor(*m_factory)(m_node, args...));
    }

    template<typename Functor, typename... Args>
    inline
    mdd_type& apply_in_place(Args... args)
    {
        node_ptr newnode = Functor(*m_factory)(m_node, args...);
        m_node->unuse();
        m_node = newnode;
        return *this;
    }

    ~mdd()
    {
        m_node->unuse();
    }

    /**
     * @brief Copy constructor.
     * @param other The mdd to copy.
     */
    mdd(const mdd_type& other)
        : m_factory(other.m_factory)
    {
        m_node = other.m_node->use();
    }

    /**
     * @brief Assignment
     * @param other The mdd to copy.
     * @return The updated mdd.
     */
    mdd_type& operator=(const mdd_type& other)
    {
        assert(m_factory == other.m_factory);
        m_node->unuse();
        m_node = other.m_node->use();
        return *this;
    }

    /**
     * @brief Set intersection.
     * @param other The mdd to intersect with.
     * @return The intersection of this mdd and other.
     */
    mdd_type operator&(const mdd_type& other) const
    { return apply<typename factory_type::mdd_set_intersect>(other.m_node); }

    /**
     * @brief Efficient intersection-assignment.
     * @see operator&()
     */
    mdd_type& operator&=(const mdd_type& other) const
    { return apply_in_place<typename factory_type::mdd_set_intersect>(other.m_node); }

    /**
     * @brief Set union.
     * @param other The mdd to merge with.
     * @return The union of this mdd and \p other.
     */
    mdd_type operator|(const mdd_type& other) const
    { return apply<typename factory_type::mdd_set_union>(other.m_node); }

    /**
     * @brief Efficient union-assignment.
     * @see operator|()
     */
    mdd_type& operator|=(const mdd_type& other) const
    { return apply_in_place<typename factory_type::mdd_set_union>(other.m_node); }


    /**
     * @brief Add an iterable to the MDD.
     *
     * Any STL-conforming iterable is acceptable. Example:
\code
std::vector<int> v = { 10, 11, 12 };
mdd::mdd_factory<int> f;
mdd::mdd<int> m = f.empty();

m = m + v; // inefficient
m += v;    // efficient
\endcode
     *
     * @param list The iterable to add.
     * @return The mdd with \p list added to it.
     */
    template <typename iterable>
    inline
    mdd_type operator+(iterable list) const
    { return add(list.begin(), list.end()); }

    /**
     * @brief Efficient addition-assignment.
     * @see operator+()
     */
    template <typename iterable>
    inline
    mdd_type& operator+=(iterable list)
    { return add_in_place(list.begin(), list.end()); }

    template <typename iterator>
    mdd_type add(iterator begin, iterator end) const
    { return apply<typename factory_type::mdd_add_element>(begin, end); }

    template <typename iterator>
    mdd_type& add_in_place(iterator begin, iterator end)
    { return apply_in_place<typename factory_type::mdd_add_element>(begin, end); }

    bool operator==(const mdd_type& other) const
    {
        assert(m_factory == other.m_factory);
        return other.m_node == m_node;
    }

    iterator begin() const
    { return iterator(m_factory, m_node); }

    iterator end() const
    { return iterator(m_factory); }
};

/**
 * @brief MDD class implementing interleaved relations.
 */
template <typename Value>
class mdd_irel : public mdd<Value>
{
public:
    friend class mdd_factory<Value>;

    typedef mdd<Value> parent;
    typedef typename parent::mdd_type mdd_type;
    typedef typename parent::factory_type factory_type;
    typedef typename parent::factory_ptr factory_ptr;
    typedef typename parent::node_ptr node_ptr;

    mdd_type compose(const mdd_irel& other)
    {
        return parent::template apply<typename factory_type::mdd_rel_composition>(other.m_node, factory_type::mdd_rel_composition::interleaved_interleaved);
    }
protected:
    mdd_irel(factory_ptr factory, node_ptr node)
        : parent(factory, node)
    {}
};

} // namespace mdd

#endif // __scranen_mdd_mdd_h
