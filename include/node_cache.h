#ifndef __scranen_mdd_node_cache_h
#define __scranen_mdd_node_cache_h

#include "node.h"

#include <unordered_map>
#include <tuple>

namespace mdd
{

enum cache_operation
{
    cache_set_union            = 0,
    cache_set_minus            = 1,
    cache_set_intersection     = 2,
    cache_rel_composition_i_i  = 3,
    cache_rel_composition_i_s  = 4,
    cache_rel_relabel          = 5,
    cache_rel_next             = 6,
    cache_rel_prev             = 7,
    cache_set_project          = 8,
    cache_clear                = 16 // <-- used as bit mask, must be next power of 2
};

template <class T>
inline void hash_combine(size_t& seed, T const& v)
{
    seed ^= std::hash<T>()(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

template <typename Node>
struct cacherecord : public std::tuple<uintptr_t, const Node*, const Node*, const node<size_t>*>
{
    typedef std::tuple<uintptr_t, const Node*, const Node*, const node<size_t>*> parent;
    typedef const Node* node_ptr;
    typedef const node<size_t>* proj_ptr;
    typedef cacherecord<Node> record_type;

    void make_clear_operation()
    {
        std::get<0>(*this) |= cache_clear;
    }

    struct hash{
        unsigned int operator()(const record_type& r) const
        {
            size_t result = 0;
            hash_combine(result, std::get<0>(r) & ~cache_clear);
            hash_combine(result, std::get<1>(r));
            hash_combine(result, std::get<2>(r));
            hash_combine(result, std::get<3>(r));
            return result;
        }
    };

    struct equal
    {
        bool operator()(const record_type& a, const record_type& b) const
        {
            return (std::get<0>(a) & cache_clear)  || (std::get<0>(b) & cache_clear) || (a == b);
        }
    };

    cacherecord(cache_operation op, node_ptr arg1, node_ptr arg2, proj_ptr arg3)
        :  parent(op, arg1, arg2, arg3)
    {
        std::get<1>(*this)->use();
        if (std::get<2>(*this))
            std::get<2>(*this)->use();
        if (std::get<3>(*this))
            std::get<3>(*this)->use();
    }

    cacherecord(const cacherecord& other)
        : parent(other)
    {
        std::get<1>(*this)->use();
        if (std::get<2>(*this))
            std::get<2>(*this)->use();
        if (std::get<3>(*this))
            std::get<3>(*this)->use();
    }

    ~cacherecord()
    {
        std::get<1>(*this)->unuse();
        if (std::get<2>(*this))
            std::get<2>(*this)->unuse();
        if (std::get<3>(*this))
            std::get<3>(*this)->unuse();
    }
};

template <typename Node>
class node_cache : private std::unordered_map<cacherecord<Node>,
                                             const Node*,
                                             typename cacherecord<Node>::hash,
                                             typename cacherecord<Node>::equal>
{
public:
    typedef std::unordered_map<cacherecord<Node>, const Node*, typename cacherecord<Node>::hash, typename cacherecord<Node>::equal> parent;
    typedef typename parent::size_type size_type;
    typedef cacherecord<Node> cacherecord_type;
    typedef const Node* node_ptr;
    typedef const node<size_t>* proj_ptr;

    node_cache(size_type size=100000)
        : parent(size), m_hits(0), m_misses(0), m_stores(0)
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
        return lookup(op, a, b, nullptr, result);
    }

    inline
    bool lookup(cache_operation op, node_ptr a, node_ptr b, proj_ptr c, node_ptr& result)
    {
        cacherecord_type rec(op, a, b, c);
        auto it = parent::find(rec);
        if (it != parent::end())
        {
            ++m_hits;
            result = it->second;
            return true;
        }
        rec.make_clear_operation();
        parent::erase(rec);
        ++m_misses;
        return false;
    }

    inline
    void store(cache_operation op, node_ptr a, node_ptr b, node_ptr result)
    {
        return store(op, a, b, nullptr, result);
    }

    inline
    void store(cache_operation op, node_ptr a, node_ptr b, proj_ptr c, node_ptr result)
    {
        parent::insert(std::move(std::make_pair(cacherecord_type(op, a, b, c), result->use())));
        ++m_stores;
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
    size_type m_hits;
    size_type m_misses;
    size_type m_stores;
};

} // namespace mdd

#endif // __scranen_mdd_node_cache_h
