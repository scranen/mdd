#ifndef __scranen_mdd_mdd_h
#define __scranen_mdd_mdd_h

#include <factory.h>
#include <iterator.h>

namespace mdd
{

template <typename Value>
class mdd_factory;

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
        m_factory->unuse(m_node);
    }

    mdd(const mdd_type& other)
        : m_factory(other.m_factory)
    {
        m_node = m_factory->use(other.m_node);
    }

    mdd_type& operator=(const mdd_type& other)
    {
        assert(m_factory == other.m_factory);
        m_factory->unuse(m_node);
        m_node = m_factory->use(other.m_node);
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

template <typename Value>
class mdd_factory : public node_factory<Value>
{
public:
    friend class mdd<Value>;

    typedef node_factory<Value> parent;
    typedef mdd<Value> mdd_type;
    typedef const node<Value>* node_ptr;

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

}

#endif // __scranen_mdd_mdd_h
