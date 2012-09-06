#ifndef __scranen_mdd_node_cache_h
#define __scranen_mdd_node_cache_h

#include "node.h"

namespace mdd
{

enum cache_operation
{
    cache_set_union,
    cache_set_minus,
    cache_set_intersection,
    cache_rel_composition_i_i,
    cache_rel_composition_i_s,
    cache_rel_relabel
};

template <typename Node>
struct cacherecord
{
    typedef const Node* node_ptr;
    typedef cacherecord<Node> record_type;
    cache_operation m_operation;
    node_ptr m_arg1;
    node_ptr m_arg2;

    struct equal
    {
        bool operator()(const record_type& a, const record_type& b) const
        {
            return (a.m_operation == b.m_operation && a.m_arg1 == b.m_arg1 && a.m_arg2 == b.m_arg2);
        }
    };

    struct hash
    {
        long operator()(const record_type& r) const
        {
            uintptr_t a = r.m_operation,
                      b = reinterpret_cast<uintptr_t>(r.m_arg1),
                      c = reinterpret_cast<uintptr_t>(r.m_arg2);
            a -= b; a -= c; a ^= (c>>13);
            b -= c; b -= a; b ^= (a<<8);
            c -= a; c -= b; c ^= (b>>13);
            a -= b; a -= c; a ^= (c>>12);
            b -= c; b -= a; b ^= (a<<16);
            c -= a; c -= b; c ^= (b>>5);
            a -= b; a -= c; a ^= (c>>3);
            b -= c; b -= a; b ^= (a<<10);
            c -= a; c -= b; c ^= (b>>15);
            return c;
        }
    };

    cacherecord(cache_operation op, node_ptr arg1, node_ptr arg2)
        : m_operation(op), m_arg1(arg1), m_arg2(arg2)
    {
        m_arg1->use();
        m_arg2->use();
    }

    cacherecord(const cacherecord& other)
        : m_operation(other.m_operation), m_arg1(other.m_arg1), m_arg2(other.m_arg2)
    {
        m_arg1->use();
        m_arg2->use();
    }

    ~cacherecord()
    {
        m_arg1->unuse();
        m_arg2->unuse();
    }
};

template <typename Node>
class node_cache : public std::unordered_map<cacherecord<Node>,
                                             const Node*,
                                             typename cacherecord<Node>::hash,
                                             typename cacherecord<Node>::equal>
{
public:
    typedef std::unordered_map<cacherecord<Node>, const Node*, typename cacherecord<Node>::hash, typename cacherecord<Node>::equal> parent;
    typedef typename parent::size_type size_type;
    typedef cacherecord<Node> cacherecord_type;
    typedef const Node* node_ptr;

    node_cache()
        : parent(), m_hits(0), m_misses(0), m_stores(0)
    { }

    void clear()
    {
        auto it = parent::begin();
        while (it != parent::end())
        {
            it->second->unuse();
            it = parent::erase(it);
        }
    }

    inline
    bool lookup(cache_operation op, node_ptr a, node_ptr b, node_ptr& result)
    {
        auto it = parent::find(cacherecord_type(op, a, b));
        if (it != parent::end())
        {
            ++m_hits;
            result = it->second;
            return true;
        }
        ++m_misses;
        return false;
    }

    inline
    void store(cache_operation op, node_ptr a, node_ptr b, node_ptr result)
    {
        ++m_stores;
#ifndef NDEBUG
        auto it =
#endif
        parent::insert(std::make_pair(cacherecord_type(op, a, b), result->use()));
        assert(it.second);
    }

    size_type hits() const
    {
        return m_hits;
    }

    size_type misses() const
    {
        return m_misses;
    }

    size_type stores() const
    {
        return m_stores;
    }
private:
    size_type m_misses;
    size_type m_hits;
    size_type m_stores;
};

} // namespace mdd

#endif // __scranen_mdd_node_cache_h
