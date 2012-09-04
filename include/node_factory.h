#ifndef __scranen_mdd_factory_h
#define __scranen_mdd_factory_h

#include <unordered_set>
#include <unordered_map>

#include "node.h"
#include "node_cache.h"

#ifdef DEBUG_MDD_NODES
#include <iostream>
#endif // DEBUG_MDD_NODES
#include <sstream>

namespace mdd
{

template <typename Value>
class mdd_iterator;

template <typename Value>
class node_factory
{
public:
    friend class mdd_iterator<Value>;

    struct mdd_add_element;
    struct mdd_set_union;
    struct mdd_set_intersect;
    struct mdd_rel_composition;
    struct mdd_rel_relabel;

    typedef Value value_type;
    typedef const value_type& const_reference;
    typedef node<value_type> node_type;
    typedef const node_type* node_ptr;
    typedef cacherecord<node_type> cacherecord_type;
    typedef node_cache<node_type> cache_type;
    typedef typename std::unordered_set<node_ptr, typename node_type::hash, typename node_type::equal> hashtable;
    typedef typename hashtable::size_type size_type;
private:
    hashtable m_nodes;
    cache_type m_cache;
    node_type m_sentinels[2];
protected:

    /*************************************************************************************************
     * Memory management operations and node creation.
     *************************************************************************************************/

    node_ptr empty() { return &m_sentinels[0]; }
    node_ptr emptylist() { return &m_sentinels[1]; }

    /**
     * @brief Creates a new node (\p val, \p right, \p down).
     * @warning Ownership of \p right and \p down is transferred to the newly created node.
     * @param val The value of the MDD node.
     * @param right The MDD node that continues this level.
     * @param down The MDD node that represents the next level.
     * @return A new MDD node with the specified values.
     */
    node_ptr create(const_reference val, node_ptr right, node_ptr down)
    {
        node_ptr newnode = new node_type(val, right, down, 1);
        auto result = m_nodes.insert(newnode);
        if (!result.second)
        {
            delete newnode;
            newnode = *result.first;
            if (newnode->usecount != 0)
            {
                right->unuse();
                down->unuse();
            }
            newnode->use();
        }
#ifdef DEBUG_MDD_NODES
        else
        {
        std::cout << "Created " << newnode << "(" << newnode->value << ", "
                  << newnode->right << ", " << newnode->down << ")@"
                  << newnode->usecount << std::endl;
        }
#endif
        return newnode;
    }

public:
    /**
     * @brief Constructor.
     */
    node_factory()
    {}

    /**
     * @brief Returns the amount of MDD nodes that reside in memory. This includes unused
     *        nodes (use clean() to remove these).
     * @return The number of MDD nodes in memory.
     */
    size_type size() { return m_nodes.size(); }

    /**
     * @brief Removes all unused nodes from the storage. Note that it is necessary to
     *        either remove *all* unused nodes, or remove none: if an unused node is
     *        removed that is still used by another unused node that is not removed,
     *        then undeleting the latter will cause problems.
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

    /**
     * @brief Clears the cache. This does not remove any nodes from the storage; to free
     *        memory after a cache clear, use clean().
     */
    void clear_cache()
    {
        m_cache.clear();
    }

    /**
     * @brief Returns the total number of cache hits since the factory was created. Together
     *        with cache_misses(), this provides information about the amount of non-trivial
     *        MDD operations requested from this object.
     */
    size_type cache_hits() const
    {
        return m_cache.hits();
    }

    /**
     * @brief Returns the total number of cache misses since the factory was created. This is
     *        a good measure for the actual amount of work done, as trivial cases in MDD
     *        operations are not cached.
     */
    size_type cache_misses() const
    {
        return m_cache.misses();
    }

    // For debugging purposes
    /**
     * @brief Dumps the contents of the node buffer to stream s.
     * @param s The stream to dump the information to.
     * @param hint1 If given (and not equal to empty()), this pointer is labelled "*"
     * @param hint2 If given (and not equal to empty()), this pointer is labelled "+"
     */
    void print_nodes(std::ostream& s, node_ptr hint1 = nullptr, node_ptr hint2 = nullptr)
    {
        std::unordered_map<node_ptr, uintptr_t> nummap;
        uintptr_t last = 1;
        for (auto it = m_nodes.begin(); it != m_nodes.end(); ++it)
        {
            node_ptr node = *it;
            uintptr_t& num = nummap[node];
            if (num == 0) num = last++;
            uintptr_t& numr = nummap[node->right];
            if (!node->right->sentinel() && numr == 0) numr = last++;
            uintptr_t& numd = nummap[node->down];
            if (!node->down->sentinel() && numd == 0) numd = last++;

            s << num;
            if (node == hint1)
                s << "*";
            if (node == hint2)
                s << "+";
            s << "(" << node->value << ", ";
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
