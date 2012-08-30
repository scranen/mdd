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

    node_ptr create(const_reference val, node_ptr right, node_ptr down)
    {
        node_ptr newnode = new node_type(val, right, down, 1);
        auto result = m_nodes.insert(newnode);
        if (!result.second)
        {
            delete newnode;
            newnode = *result.first;
            if (newnode->usecount == 0)
            {
                right->use();
                down->use();
                newnode->usecount = 1;
#ifdef DEBUG_MDD_NODES
        std::cout << "Undeleted " << newnode << "(" << newnode->value << ", "
                  << newnode->right << ", " << newnode->down << ")@"
                  << newnode->usecount << std::endl;
#endif
            }
            else
                newnode->use();
        }
        else
        {
            right->use();
            down->use();
#ifdef DEBUG_MDD_NODES
        std::cout << "Created " << newnode << "(" << newnode->value << ", "
                  << newnode->right << ", " << newnode->down << ")@"
                  << newnode->usecount << std::endl;
#endif
        }

        return newnode;
    }

    /*************************************************************************************************
     * Operations on sets
     *************************************************************************************************/

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
     * @param a The MDD to add a vector to.
     * @param begin The start of the vector (any iterator).
     * @param end The end of the vector (any iterator comparable to begin).
     * @return An MDD that represents a with the provided vector added to it.
     */
    template <typename iterator>
    node_ptr add(node_ptr a, iterator begin, iterator end)
    {
        node_ptr result;
        node_ptr temp;
        if (begin == end)
        {
            if (a->sentinel())
                return emptylist();
            temp = add(a->right, begin, end);
            result = create(a->value, temp, a->down);
        }
        else
        if (a->sentinel() || a->value > *begin)
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
        temp->unuse();
        return result;
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
