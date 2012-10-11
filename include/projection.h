#ifndef __scranen_mdd_projection_h
#define __scranen_mdd_projection_h

#include "mdd.h"

namespace mdd
{

class projection_factory;

class projection
{
public:
    friend class projection_factory;
    typedef mdd<size_t> parent;
    typedef const node<size_t>* node_ptr;
    typedef node_factory<size_t> factory_type;
    typedef factory_type* factory_ptr;

    class iterator : public std::iterator<std::input_iterator_tag, bool>
    {
        friend class projection;
    private:
        node_ptr m_node;
        size_t m_level;

        iterator(node_ptr node, size_t level)
            : m_node(node), m_level(level)
        { }
    public:
        iterator& operator++()
        {
            if (m_level == m_node->value)
                m_node = m_node->down;
            ++m_level;
            return *this;
        }
        iterator operator++(int) { iterator it(m_node, m_level); operator++(); return it; }
        bool operator*() const { return !m_node->sentinel() && m_level == m_node->value; }
        bool operator==(const iterator& other) const { return m_node == other.m_node && m_level == other.m_level; }
        bool operator!=(const iterator& other) const { return m_node != other.m_node || m_level != other.m_level; }
        node_ptr node() const { return m_node; }
    };

    iterator begin() const
    {
        return iterator(m_node, 0);
    }

    iterator end() const
    {
        return iterator(m_factory.emptylist(), m_domain_size);
    }

    bool full() const
    {
        return m_node == m_factory.empty();
    }

    size_t size() const
    {
        return m_size;
    }

    size_t domain_size() const
    {
        return m_domain_size;
    }

    projection(const projection& other)
        : m_factory(other.m_factory), m_node(other.m_node)
    { }
private:
    factory_type& m_factory;
    node_ptr m_node;
    size_t m_size;
    size_t m_domain_size;

    template<typename listit>
    projection(factory_type& factory, listit begin, listit end, size_t domain_size)
        : m_factory(factory), m_node(factory_type::mdd_add_element(factory)(factory.empty(), begin, end)), m_size(0), m_domain_size(domain_size)
    {
        while (begin != end)
        {
            ++m_size;
            ++begin;
        }
    }

    projection(factory_type& factory, size_t domain_size)
        : m_factory(factory), m_node(factory.empty()), m_size(0), m_domain_size(domain_size)
    { }
};

class projection_factory : private node_factory<size_t>
{
public:
    template <typename iterator>
    projection create(iterator begin, iterator end, size_t domain_size)
    {
        return projection(*this, begin, end, domain_size);
    }

    projection create(size_t domain_size)
    {
        return projection(*this, domain_size);
    }
};

} // namespace mdd

#endif // __scranen_mdd_projection_h
