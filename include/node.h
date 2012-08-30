#ifndef __scranen_mdd_node_h
#define __scranen_mdd_node_h

#include <stdint.h>

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
            return (a->value == b->value && a->right == b->right && a->down == b->down);
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

    inline
    bool sentinel() const
    {
        return !down;
    }

    inline
    node_ptr use() const
    {
        if (!sentinel())
        {
            assert(usecount > 0);
            ++usecount;
#ifdef DEBUG_MDD_NODES
            std::cout << "Reused " << this << "(" << value << ", "
                      << right << ", " << down << ")@"
                      << usecount << std::endl;
#endif
        }
        return this;
    }

    inline
    void unuse() const
    {
        if (!sentinel())
        {
            assert(usecount > 0);
            if (--usecount == 0)
            {
                down->unuse();
                right->unuse();
            }
#ifdef DEBUG_MDD_NODES
            std::cout << "Deleted " << this << "(" << value << ", "
                      << right << ", " << down << ")@"
                      << usecount << std::endl;
#endif
        }
    }

    Value value;
    node_ptr right;
    node_ptr down;
    mutable uintptr_t usecount;

    node()
        : down(nullptr)
    { }

    node(const Value& value, node_ptr right, node_ptr down, uintptr_t usecount)
        : value(value), right(right), down(down), usecount(usecount)
    {}
};

template<typename Value>
inline
void order(const node<Value>*& a, const node<Value>*& b)
{
    if (b < a)
    {
        const node<Value>* tmp = a;
        a = b;
        b = tmp;
    }
}


} // namespace mdd

#endif // __scranen_mdd_node_h
