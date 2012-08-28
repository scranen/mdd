#ifndef __scranen_mdd_factory_h
#define __scranen_mdd_factory_h

#include <stdint.h>
#include <unordered_set>
#include <unordered_map>

// #define DEBUG_MDD_NODES
// #include <iostream>

namespace mdd
{

template <typename Value, typename Hash=std::hash<Value> >
struct node
{
    typedef node<Value> node_type;
    typedef const node_type* node_ptr;

    struct equal
    {
        bool operator()(const node_ptr& a, const node_ptr& b) const
        {
            return (a->usecount == 0) || (b->usecount == 0) ||
                   (a->value == b->value && a->right == b->right && a->down == b->down);
        }
    };

    struct hash
    {
        long operator()(const node_ptr& r) const
        {
            uintptr_t a = Hash()(r->value),
                      b = (uintptr_t)r->right,
                      c = (uintptr_t)r->down;
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

    mutable Value value;
    mutable node_ptr right;
    mutable node_ptr down;
    mutable uintptr_t usecount;
    node(const Value& value, node_ptr right, node_ptr down, uintptr_t usecount)
        : value(value), right(right), down(down), usecount(usecount)
    {}
};

template <typename Node>
struct cacherecord
{
    enum operation
    {
        set_union,
        set_intersection,
        num_operations
    };

    typedef const Node* node_ptr;
    typedef cacherecord<Node> record_type;
    operation m_operation;
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

    cacherecord(operation op, node_ptr arg1, node_ptr arg2)
        : m_operation(op), m_arg1(arg1), m_arg2(arg2)
    { }
};

template <typename Value>
class node_factory
{
public:
    typedef Value value_type;
    typedef const value_type& const_reference;
    typedef node<value_type> node_type;
    typedef const node_type* node_ptr;
    typedef cacherecord<node_type> cacherecord_type;
    typedef typename std::unordered_map<cacherecord_type, node_ptr, typename cacherecord_type::hash, typename cacherecord_type::equal> cachemap;
    typedef typename std::unordered_set<node_ptr, typename node_type::hash, typename node_type::equal> hashtable;
    typedef typename hashtable::size_type size_type;
private:
    hashtable m_nodes;
    cachemap m_cache;
protected:

    /*************************************************************************************************
     * Memory management operations and node creation.
     *************************************************************************************************/

    static node_ptr empty() { return static_cast<node_ptr>(0); }
    static node_ptr emptylist() { return reinterpret_cast<node_ptr>(1); }
    static bool is_sentinel(node_ptr a) { return reinterpret_cast<uintptr_t>(a) < 2; }

    node_ptr use(node_ptr node)
    {
        if (!is_sentinel(node))
        {
            assert(node->usecount > 0);
            if (++node->usecount == 1)
            {
                use(node->right);
                use(node->down);
            }
#ifdef DEBUG_MDD_NODES
            std::cout << "Reused " << node << "(" << node->value << ", "
                      << node->right << ", " << node->down << ")@"
                      << node->usecount << std::endl;
#endif
        }
        return node;
    }
    void unuse(node_ptr node)
    {
        if (!is_sentinel(node))
        {
            assert(node->usecount > 0);
            if (--node->usecount == 0)
            {
                unuse(node->down);
                unuse(node->right);
            }
#ifdef DEBUG_MDD_NODES
            std::cout << "Deleted " << node << "(" << node->value << ", "
                      << node->right << ", " << node->down << ")@"
                      << node->usecount << std::endl;
#endif
        }
    }

    node_ptr create(const_reference val, node_ptr right=empty(), node_ptr down=emptylist())
    {
        node_ptr newnode = new node_type(val, right, down, 1);
        auto result = m_nodes.insert(newnode);
        if (!result.second)
        {
            delete newnode;
            newnode = use(*result.first);
        }
        else
        {
            use(right);
            use(down);
#ifdef DEBUG_MDD_NODES
        std::cout << "Created " << newnode << "(" << newnode->value << ", "
                  << newnode->right << ", " << newnode->down << ")@"
                  << newnode->usecount << std::endl;
#endif
        }
        if (newnode->usecount == 0)
        {
            newnode->value = val;
            newnode->right = use(right);
            newnode->down = use(down);
            newnode->usecount = 1;
#ifdef DEBUG_MDD_NODES
        std::cout << "Undeleted " << newnode << "(" << newnode->value << ", "
                  << newnode->right << ", " << newnode->down << ")@"
                  << newnode->usecount << std::endl;
#endif
        }
        return newnode;
    }

    /*************************************************************************************************
     * Operations used for caching and optimisation
     *************************************************************************************************/

    inline
    bool cache_lookup(typename cacherecord_type::operation op, node_ptr a, node_ptr b, node_ptr& result) const
    {
        auto it = m_cache.find(cacherecord_type(op, a, b));
        if (it != m_cache.end())
        {
            result = it->second;
            return true;
        }
        return false;
    }

    inline
    void cache_store(typename cacherecord_type::operation op, node_ptr a, node_ptr b, node_ptr result)
    {
        m_cache.insert(std::make_pair(cacherecord_type(op, a, b), result));
    }

    inline
    void order(node_ptr& a, node_ptr& b) const
    {
        if (b < a)
        {
            node_ptr tmp = a;
            a = b;
            b = tmp;
        }
    }

    /*************************************************************************************************
     * Operations on sets
     *************************************************************************************************/

    /**
     * @brief Computes the union of two sets.
     * @param a
     * @param b
     * @return
     */
    node_ptr set_union(node_ptr a, node_ptr b)
    {
        node_ptr result;
        order(a, b);

        if (a == b || b == empty())
            return use(a);
        if (a == empty())
            return use(b);
        if (a == emptylist())
            return add_emptylist(b);
        if (b == emptylist())
            return add_emptylist(a);

        if (cache_lookup(cacherecord_type::set_union, a, b, result))
            return result;

        if (a->value < b->value)
        {
            node_ptr temp = set_union(a->right, b);
            result = create(a->value, temp, a->down);
            unuse(temp);
        }
        else
        if (a->value == b->value)
        {
            node_ptr temp1 = set_union(a->right, b->right),
                     temp2 = set_union(a->down, b->down);
            result = create(a->value, temp1, temp2);
            unuse(temp1);
            unuse(temp2);
        }
        else
        {
            node_ptr temp = set_union(a, b->right);
            result = create(b->value, temp, b->down);
            unuse(temp);
        }

        cache_store(cacherecord_type::set_union, a, b, result);
        return result;
    }

    /**
     * @brief Shortcut for adding an empty list to MDD a. This method simply calls add(a, b, e)
     *        with (b == e).
     * @param a
     * @return
     */
    inline
    node_ptr add_emptylist(node_ptr a)
    {
        return add(a, static_cast<value_type*>(nullptr), static_cast<value_type*>(nullptr));
    }

    /**
     * @brief Adds a list of elements to the MDD.
     * @param a
     * @param begin
     * @param end
     * @return
     */
    template <typename iterator>
    node_ptr add(node_ptr a, iterator begin, iterator end)
    {
        node_ptr result;
        node_ptr temp;
        if (begin == end)
        {
            if (is_sentinel(a))
                return emptylist();
            temp = add(a->right, begin, end);
            result = create(a->value, temp, a->down);
        }
        else
        if (is_sentinel(a) || a->value > *begin)
        {
          temp = add(empty(), begin + 1, end);
          result = create(*begin, a, temp);
        }
        else
        {
            if (a->value == *begin)
            {
                temp = add(a->down, begin + 1, end);
                result = create(a->value, a->right, temp);
            }
            else
            if (a->value < *begin)
            {
                temp = add(a->right, begin, end);
                result = create(a->value, temp, a->down);
            }
        }
        unuse(temp);
        return result;
    }

public:
    size_type size() { return m_nodes.size(); }

    /**
     * @brief Removes all unused nodes from the storage.
     */
    void clean()
    {
        for (auto it = m_nodes.begin(); it != m_nodes.end();)
        {
            if ((*it)->usecount == 0)
                it = m_nodes.erase(it);
            else
                ++it;
        }
    }

    // For debugging purposes
    /**
     * @brief Dumps the contents of the node buffer to stream s.
     * @param s The stream to dump the information to.
     * @param hint1 If given (and not equal to empty()), this pointer is labelled "1"
     * @param hint2 If given (and not equal to empty()), this pointer is labelled "2"
     */
    void print_nodes(std::ostream& s, node_ptr hint1 = empty(), node_ptr hint2 = empty())
    {
        std::unordered_map<node_ptr, uintptr_t> nummap;
        uintptr_t last = 1;
        if (hint2 != empty())
        {
            nummap[hint1] = last++;
            nummap[hint2] = last++;
        }
        else
        if (hint1 != empty())
            nummap[hint1] = last++;
        for (auto it = m_nodes.begin(); it != m_nodes.end(); ++it)
        {
            node_ptr node = *it;
            uintptr_t& num = nummap[node];
            if (num == 0) num = last++;
            uintptr_t& numr = nummap[node->right];
            if (!is_sentinel(node->right) && numr == 0) numr = last++;
            uintptr_t& numd = nummap[node->down];
            if (!is_sentinel(node->down) && numd == 0) numd = last++;

            s << num << "(" << node->value << ", ";
            if (node->right == empty()) s << "FALSE, "; else
            if (node->right == emptylist()) s << "TRUE, "; else
                s << numr << ", ";
            if (node->down == empty()) s << "FALSE)@"; else
            if (node->down == emptylist()) s << "TRUE)@"; else
                s << numd << ")@";
            s << node->usecount << std::endl;
        }
    }
};

}

#endif // __scranen_mdd_factory_h
