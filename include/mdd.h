#ifndef __scranen_mdd_mdd_h
#define __scranen_mdd_mdd_h

#include <mdd_iterator.h>
#include <mdd_factory.h>

namespace mdd
{

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
    typedef mdd_factory<Value>* factory_ptr;
    typedef typename mdd_factory<Value>::node_ptr node_ptr;
private:
    factory_ptr m_factory;
    node_ptr m_node;

    mdd(factory_ptr factory, node_ptr node)
        : m_factory(factory), m_node(node)
    {}

public:
    ~mdd()
    {
        m_node->unuse();
    }

    mdd(const mdd_type& other)
        : m_factory(other.m_factory)
    {
        m_node = other.m_node->use();
    }

    mdd_type& operator=(const mdd_type& other)
    {
        assert(m_factory == other.m_factory);
        m_node->unuse();
        m_node = other.m_node->use();
        return *this;
    }

    mdd_type operator&(const mdd_type& other) const
    {
        assert(m_factory == other.m_factory);
        return mdd_type(m_factory, m_factory->intersect(m_node, other.m_node));
    }

    mdd_type operator|(const mdd_type& other) const
    {
        assert(m_factory == other.m_factory);
        return mdd_type(m_factory, m_factory->set_union(m_node, other.m_node));
    }

    template <typename iterable>
    mdd_type operator+(iterable list) const
    {
        return mdd_type(m_factory, m_factory->add(m_node, list.begin(), list.end()));
    }

    bool operator==(const mdd_type& other) const
    {
        assert(m_factory == other.m_factory);
        return other.m_node == m_node;
    }

    iterator begin() const
    {
        return iterator(m_factory, m_node);
    }

    iterator end() const
    {
        return iterator(m_factory);
    }
};

} // namespace mdd

#endif // __scranen_mdd_mdd_h
