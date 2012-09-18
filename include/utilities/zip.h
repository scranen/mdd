#ifndef __scranen_mdd_utilities_zip_h
#define __scranen_mdd_utilities_zip_h

#include <iterator>

namespace mdd
{
namespace utilities
{

template <typename data_iterator>
class zip
{
public:
    /**
     * @brief Zip iterator.
     *
     * This is an input iterator that takes two begin/end pairs of a certain type
     * of iterator, and behaves like that type of iterator again, only interleaving
     * the values of both pairs.
     */
    class iterator : public std::iterator<std::input_iterator_tag, typename std::iterator_traits<data_iterator>::value_type>
    {
        data_iterator m_a, m_b;
        bool m_at_a;
    public:
        iterator(data_iterator&& a, data_iterator&& b)
            : m_a(std::move(a)), m_b(std::move(b)), m_at_a(true)
        { }
        iterator(data_iterator& a, data_iterator& b)
            : m_a(a), m_b(b), m_at_a(true)
        { }
        iterator(const iterator& other)
            : m_a(other.m_a), m_b(other.m_b), m_at_a(other.m_at_a)
        { }
        iterator& operator=(const iterator& other)
        {
            m_a = other.m_a;
            m_b = other.m_b;
            m_at_a = other.m_at_a;
            return *this;
        }
        const typename iterator::value_type operator*() const { return m_at_a ? *m_a : *m_b; }
        const typename iterator::value_type* operator->() { return m_at_a ? m_a.operator->() : m_b.operator->(); }
        bool operator==(const iterator& other) const { return other.m_a == m_a && other.m_b == m_b && other.m_at_a == m_at_a; }
        bool operator!=(const iterator& other) const { return !operator==(other); }
        iterator operator++(int) { iterator result(*this); operator++(); return result; }
        iterator& operator++()
        {
            if (m_at_a)
                ++m_a;
            else
                ++m_b;
            m_at_a = !m_at_a;
            return *this;
        }
    };

    zip(data_iterator&& a_begin, data_iterator&& a_end, data_iterator&& b_begin, data_iterator&& b_end)
        : m_begin(a_begin, b_begin), m_end(a_end, b_end)
    { }

    zip(data_iterator& a_begin, data_iterator& a_end, data_iterator& b_begin, data_iterator& b_end)
        : m_begin(a_begin, b_begin), m_end(a_end, b_end)
    { }

    iterator& begin()
    {
        return m_begin;
    }

    iterator& end()
    {
        return m_end;
    }
private:
    iterator m_begin, m_end;
};

} // namespace utilities
} // namespace mdd

#endif // __scranen_mdd_utilities_zip_h
