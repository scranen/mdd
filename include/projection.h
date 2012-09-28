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

    class iterator : public std::iterator<std::input_iterator_tag, size_t>
    {
        friend class projection;
    private:
        node_ptr m_node;

        iterator(node_ptr node)
            : m_node(node)
        { }
    public:
        iterator& operator++() { m_node = m_node->down; return *this; }
        iterator operator++(int) { iterator it(m_node); operator++(); return it; }
        size_t operator*() const { return m_node->value; }
        const size_t* operator->() { return &m_node->value; }
        bool operator==(const iterator& other) const { return m_node == other.m_node; }
        bool operator!=(const iterator& other) const { return m_node != other.m_node; }
        node_ptr node() const { return m_node; }
    };

    iterator begin() const
    {
        return iterator(m_node);
    }

    iterator end() const
    {
        return iterator(m_factory.emptylist());
    }

    bool full() const
    {
        return m_node == m_factory.empty();
    }

    size_t size() const
    {
        return m_size;
    }

    projection(const projection& other)
        : m_factory(other.m_factory), m_node(other.m_node)
    { }
private:
    factory_type& m_factory;
    node_ptr m_node;
    size_t m_size;

    template<typename listit>
    projection(factory_type& factory, listit begin, listit end)
        : m_factory(factory), m_node(factory_type::mdd_add_element(factory)(factory.empty(), begin, end)), m_size(0)
    {
        while (begin != end)
        {
            ++m_size;
            ++begin;
        }
    }

    projection(factory_type& factory)
        : m_factory(factory), m_node(factory.empty()), m_size(0)
    { }
};

class projection_factory : private node_factory<size_t>
{
public:
    template <typename iterator>
    projection create(iterator begin, iterator end)
    {
        return projection(*this, begin, end);
    }

    projection create()
    {
        return projection(*this);
    }
};

} // namespace mdd

#endif // __scranen_mdd_projection_h
