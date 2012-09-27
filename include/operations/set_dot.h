#ifndef __scranen_mdd_operations_set_dot_h
#define __scranen_mdd_operations_set_dot_h

#include "node_factory.h"
#include "add_element.h"
#include <sstream>
#include <set>

namespace mdd
{

template <typename Value>
struct node_factory<Value>::mdd_set_dot
{
    typedef node_factory<Value> factory_type;
    typedef typename factory_type::node_ptr node_ptr;

    factory_type& m_factory;

    mdd_set_dot(factory_type& factory)
        : m_factory(factory)
    { }

    std::string operator()(node_ptr p)
    {
        std::stringstream s;
        std::set<node_ptr> seen;
        int sentinels = 0;
        s << "digraph {\n";
        s << "  rankdir=TB;\n";
        s << "  node [label=\"\",shape=record];\n";
        if (p->sentinel())
            s << "  " << (p == m_factory.empty() ? "F [label=\"F\"];\n" : "T [label=\"T\"];\n");
        else
            dump(s, p, seen, sentinels);
        s << "}\n";
        return s.str();
    }
private:
    void dump(std::ostream& s, node_ptr p, std::set<node_ptr>& seen, int& sentinels)
    {
        if (!p->sentinel() && seen.insert(p).second)
        {
            s << "  n" << p << " [label=\"";
            if (p->down->sentinel())
                s << "{" << p->value << "|" << (p->down == m_factory.empty() ? "F}" : "T}");
            else
                s << p->value;
            if (p->right->sentinel())
                s << "|" << (p->right == m_factory.empty() ? "F" : "T");
            s << "\"];\n";
            dump(s, p->right, seen, sentinels);
            dump(s, p->down, seen, sentinels);
            if (!p->down->sentinel())
                s << "  n" << p << " -> n" << p->down << ";\n";
            if (!p->right->sentinel())
                s << "  { rank=same; n" << p << " -> n" << p->right << "; }\n";
        }
    }
};

}

#endif // __scranen_mdd_operations_set_dot_h
