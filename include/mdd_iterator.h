#ifndef __scranen_mdd_iterator_h
#define __scranen_mdd_iterator_h

#include <stack>
#include <stdexcept>
#include <iterator>

#include "node_factory.h"

namespace mdd
{

template <typename Value>
class mdd_iterator : public std::iterator<std::input_iterator_tag, Value>
{
public:
    typedef std::vector<Value> vector_type;
    typedef node_factory<Value>* factory_ptr;
    typedef const node<Value>* node_ptr;

    mdd_iterator(factory_ptr factory, node_ptr node)
        : m_factory(factory)
    {
        m_stack.push(node);
        saturate();
    }

    mdd_iterator(factory_ptr factory)
        : m_factory(factory)
    { }

    mdd_iterator(const mdd_iterator& other)
        : m_stack(other.m_stack), m_vector(other.m_vector), m_factory(other.m_factory)
    { }

    const vector_type& operator*() const
    {
        if (m_stack.empty())
            throw std::runtime_error("Trying to dereference MDD end() iterator.");
        return m_vector;
    }

    mdd_iterator<Value>& operator++()
    {
        next();
        saturate();
        return *this;
    }

    mdd_iterator<Value> operator++(int)
    {
        mdd_iterator<Value> result(*this);
        next();
        saturate();
        return result;
    }

    bool operator==(const mdd_iterator<Value>& other) const
    {
        return m_stack == other.m_stack && m_factory == other.m_factory && m_vector == other.m_vector;
    }

    bool operator!=(const mdd_iterator<Value>& other) const
    {
        return !operator==(other);
    }

private:
    void saturate()
    {
        bool done = m_stack.empty();
        while (!done)
        {
            if (m_stack.top()->sentinel())
            {
                if (m_stack.top() == m_factory->empty())
                {
                    m_stack.pop();
                    if (m_stack.empty())
                        done = true;
                    else
                        next();
                }
                else // m_stack.top() == m_factory.emptylist()
                {
                    m_stack.pop();
                    done = true;
                }
            }
            else
            {
                m_vector.push_back(m_stack.top()->value);
                m_stack.push(m_stack.top()->down);
            }
        }
    }

    void next()
    {
        if (m_stack.empty())
        {
            if (m_vector.empty())
                throw std::runtime_error("Trying to increase MDD end() iterator.");
            m_vector.clear();
        }
        else
        {
            node_ptr newtop = m_stack.top()->right;
            m_stack.pop();
            m_vector.pop_back();
            m_stack.push(newtop);
        }
    }

    std::stack<node_ptr> m_stack;
    vector_type m_vector;
    factory_ptr m_factory;
};

} // namespace mdd

#endif // __scranen_mdd_iterator_h
