#ifndef __scranen_mdd_mdd_h
#define __scranen_mdd_mdd_h

#include <mdd_iterator.h>
#include <mdd_factory.h>

namespace mdd
{

/**
 * @brief The main MDD class.
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
    typedef const mdd_iterator<Value> const_iterator;
    typedef Value& reference;
    typedef const Value& const_reference;
    typedef mdd<Value> mdd_type;
    typedef mdd_factory<Value> factory_type;
    typedef factory_type* factory_ptr;
    typedef typename mdd_factory<Value>::node_ptr node_ptr;
private:
    factory_ptr m_factory;
    node_ptr m_node;

    mdd(factory_ptr factory, node_ptr node)
        : m_factory(factory), m_node(node)
    {}

    template<typename Functor>
    inline
    mdd_type apply(const mdd_type& other) const
    {
        assert(m_factory == other.m_factory);
        return mdd_type(m_factory, Functor(*m_factory)(m_node, other.m_node));
    }

    template<typename Functor>
    inline
    mdd_type apply_in_place(const mdd_type& other) const
    {
        assert(m_factory == other.m_factory);
        node_ptr newnode = Functor(*m_factory)(m_node, other.m_node);
        m_node->unuse();
        m_node = newnode;
        return *this;
    }

    template<typename Functor, typename... Args>
    inline
    mdd_type apply(Args... args) const
    {
        return mdd_type(m_factory, Functor(*m_factory)(m_node, args...));
    }

    template<typename Functor, typename... Args>
    inline
    mdd_type apply_in_place(Args... args) const
    {
        node_ptr newnode = Functor(*m_factory)(m_node, args...);
        m_node->unuse();
        m_node = newnode;
        return *this;
    }

public:
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
    { return apply<typename factory_type::mdd_set_intersect>(other); }

    /**
     * @brief Efficient intersection-assignment.
     * @see operator&()
     */
    mdd_type& operator&=(const mdd_type& other) const
    { return apply_in_place<typename factory_type::mdd_set_intersect>(other); }

    /**
     * @brief Set union.
     * @param other The mdd to merge with.
     * @return The union of this mdd and \p other.
     */
    mdd_type operator|(const mdd_type& other) const
    { return apply<typename factory_type::mdd_set_union>(other); }

    /**
     * @brief Efficient union-assignment.
     * @see operator|()
     */
    mdd_type& operator|=(const mdd_type& other) const
    { return apply_in_place<typename factory_type::mdd_set_union>(other); }


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
    mdd_type operator+(iterable list) const
    { return apply<typename factory_type::mdd_add_element>(list.begin(), list.end()); }

    /**
     * @brief Efficient addition-assignment.
     * @see operator+()
     */
    template <typename iterable>
    mdd_type& operator+=(iterable list) const
    { return apply_in_place<typename factory_type::mdd_add_element>(list.begin(), list.end()); }

    template <typename iterator>
    mdd_type add(iterator begin, iterator end)
    { return apply<typename factory_type::mdd_add_element>(begin, end); }

    template <typename iterator>
    mdd_type add_in_place(iterator begin, iterator end)
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

} // namespace mdd

#endif // __scranen_mdd_mdd_h
