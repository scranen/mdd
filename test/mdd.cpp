// #define DEBUG_MDD_NODES

#include <algorithm>
#include <vector>
#include <list>

#include <gtest/gtest.h>

#include "mdd.h"
#include "utilities/zip.h"
#include "projection.h"

#include <fstream>

namespace std
{
void PrintTo(const std::vector<char>& vec, std::ostream* os) {
    *os << "[";
    for (auto c: vec)
        *os << c << " ";
    *os << "]";
}
}

class MDDTest : public testing::Test {
protected:
    virtual void SetUp()
    {
        strvec1.push_back("a");
        strvec2.push_back("a");
        strvec2.push_back("b");
        strvec3.push_back("b");
        strvec3.push_back("c");
        floatvec1.push_back(1.0);
        floatvec1.push_back(2.0);
        floatvec2.push_back(1.0);
        floatvec2.push_back(3.0);
    }

    std::vector<std::string> strvec0;
    std::vector<std::string> strvec1;
    std::vector<std::string> strvec2;
    std::vector<std::string> strvec3;
    std::vector<float> floatvec1;
    std::vector<float> floatvec2;
    std::vector<float> floatvec3;
};

TEST_F(MDDTest, CreateStringMDD)
{
    mdd::mdd_factory<std::string> strfactory;
    EXPECT_EQ(0, strfactory.size()) << strfactory.print_nodes();
    {
        mdd::mdd<std::string> m = strfactory.empty_set();
        m = m + strvec1; // [a]
        m += strvec2;    // [a, b]
        m = m + strvec3; // [b, c]
        m += strvec3;    // [b, c]

        EXPECT_EQ(strfactory.singleton_set(), strfactory.empty_set() + strvec0);

        auto result = std::find(m.begin(), m.end(), strvec1);
        EXPECT_NE(m.end(), result);
        result = std::find(m.begin(), m.end(), strvec2);
        EXPECT_NE(m.end(), result);
        result = std::find(m.begin(), m.end(), strvec3);
        EXPECT_NE(m.end(), result);

        if (HasFailure())
             ADD_FAILURE() << "contents was:\n" << ::testing::PrintToString(m);

        strfactory.clean();
        EXPECT_EQ(4, strfactory.size()) << strfactory.print_nodes();
    }
    strfactory.clean();
    EXPECT_EQ(0, strfactory.size()) << strfactory.print_nodes();
}

TEST_F(MDDTest, SetUnion)
{
    mdd::mdd_factory<std::string> strfactory;
    EXPECT_EQ(0, strfactory.size());
    {
        mdd::mdd<std::string> m1 = strfactory.empty_set(),
                              m2 = strfactory.empty_set();
        m1 = m1 + strvec1; // [a]
        m2 = m2 + strvec2; // [a, b]

        EXPECT_EQ(m1 | m2, m1 | m2 | m2) << strfactory.print_nodes(m1, m2);

        m2 = m2 + strvec3; // [b, c]
        m1 = m2 | m1;
        m2 = m1 | m2;

        EXPECT_EQ(m1, m2) << strfactory.print_nodes(m1, m2);

        m1 = strfactory.empty_set();
        m1 = m1 + strvec1; // [a]
        m2 = strfactory.empty_set();
        m2 = m2 + strvec2; // [a, b]

        size_t hits = strfactory.cache_hits(),
               misses = strfactory.cache_misses();

        m1 | m2; // Seen before: cache hit
        m2 | m1; // Seen before in different order: cache hit
        m1 | m1; // Union of equal elements: trivial case
        m2 | strfactory.empty_set(); // Union with empty set: trivial case
        strfactory.empty_set() | m2; // Union with empty set: trivial case

        EXPECT_EQ(hits + 2, strfactory.cache_hits()) << "m1: " << ::testing::PrintToString(m1)
                                                     << "\nm2: " << ::testing::PrintToString(m2)
                                                     << "\n" << strfactory.print_nodes(m1, m2);
        EXPECT_EQ(misses, strfactory.cache_misses()) << "m1: " << ::testing::PrintToString(m1)
                                                     << "\nm2: " << ::testing::PrintToString(m2)
                                                     << "\n" << strfactory.print_nodes(m1, m2);
    }
    strfactory.clear_cache();
    strfactory.clean();
    EXPECT_EQ(0, strfactory.size()) << strfactory.print_nodes();
}

TEST_F(MDDTest, SetDiff)
{
    mdd::mdd_factory<std::string> strfactory;
    EXPECT_EQ(0, strfactory.size());
    {
        mdd::mdd<std::string> m1 = strfactory.empty_set(),
                              m2 = strfactory.empty_set(),
                              m3 = strfactory.empty_set();
        m1 += strvec1; // [a]
        m2 = m1 + strvec2; // [a, b]
        m3 += strvec2;
        m1 = m2 - m1;
        EXPECT_EQ(m3, m1);
    }
    strfactory.clear_cache();
    strfactory.clean();
    EXPECT_EQ(0, strfactory.size()) << strfactory.print_nodes();
}

TEST_F(MDDTest, RelNext)
{
    mdd::mdd_factory<int> strfactory;
    EXPECT_EQ(0, strfactory.size());
    {
        int R[4][2][2] = { { {0, 0}, {1, 1} },
                           { {1, 1}, {2, 2} },
                           { {0, 0}, {2, 2} },
                           { {2, 2}, {3, 3} } };
        int S[2][2] = { {0, 0},
                        {1, 1} };
        int N[2][2] = { {1, 1},
                        {2, 2} };
        mdd::mdd_irel<int> r = strfactory.empty_irel();
        mdd::mdd<int> s1 = strfactory.empty_set(),
                      s2 = strfactory.empty_set();

        for (auto v: R)
            r.add_in_place(v[0], v[0] + 2, v[1], v[1] + 2);
        for (auto v: S)
            s1.add_in_place(v, v + 2);
        for (auto v: N)
            s2.add_in_place(v, v + 2);

        EXPECT_EQ(s2, r(s1)) << testing::PrintToString(r);
    }
    strfactory.clear_cache();
    strfactory.clean();
    EXPECT_EQ(0, strfactory.size()) << strfactory.print_nodes();
}

TEST_F(MDDTest, PartialRelNext)
{
    mdd::mdd_factory<int> strfactory;
    mdd::projection_factory projfactory;
    EXPECT_EQ(0, strfactory.size());
    {
        int R[2][2][4] = { { {9, 0, 0, 0}, {4, 1, 0, 0} },
                           { {9, 0, 0, 1}, {4, 1, 1, 1} } };
        int S[10][13] = {{ 3, 2, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
                         { 3, 2, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1 },
                         { 3, 2, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0 },
                         { 3, 2, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1 },
                         { 5, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                         { 5, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
                         { 5, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                         { 5, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
                         { 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                         { 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } };

        int N[2][13] = { {4, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                         {4, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1} };
        int P[4] = {0, 1, 2, 12};
        mdd::projection proj = projfactory.create(P, P + 4, 13);
        mdd::mdd_irel<int> r = strfactory.empty_irel();
        mdd::mdd<int> s1 = strfactory.empty_set(),
                      s2 = strfactory.empty_set();

        for (auto v: R)
            r.add_in_place(v[0], v[0] + 4, v[1], v[1] + 4);
        for (auto v: S)
            s1.add_in_place(v, v + 13);
        for (auto v: N)
            s2.add_in_place(v, v + 13);

        std::ofstream lvl("/tmp/lvl.dot");
        std::ofstream nxt("/tmp/nxt.dot");
        std::ofstream res("/tmp/res.dot");
        lvl << s1.dot();
        lvl.close();
        nxt << r.dot();
        nxt.close();
        res << r(s1, proj).dot();
        res.close();

        EXPECT_EQ(s2, r(s1, proj)) << testing::PrintToString(r);
    }
    strfactory.clear_cache();
    strfactory.clean();
    EXPECT_EQ(0, strfactory.size()) << strfactory.print_nodes();
}

TEST_F(MDDTest, RelPrev)
{
    mdd::mdd_factory<int> strfactory;
    EXPECT_EQ(0, strfactory.size());
    {
        int R[4][2][2] = { { {0, 0}, {1, 1} },
                           { {1, 1}, {2, 2} },
                           { {0, 0}, {2, 2} },
                           { {2, 2}, {3, 3} } };
        int S[2][2] = { {1, 1},
                        {2, 2} };
        int N[2][2] = { {0, 0},
                        {1, 1} };
        mdd::mdd_irel<int> r = strfactory.empty_irel();
        mdd::mdd<int> s1 = strfactory.empty_set(),
                      s2 = strfactory.empty_set();

        for (auto v: R)
            r.add_in_place(v[0], v[0] + 2, v[1], v[1] + 2);
        for (auto v: S)
            s1.add_in_place(v, v + 2);
        for (auto v: N)
            s2.add_in_place(v, v + 2);

        EXPECT_EQ(s2, r.pre(s1));
    }
    strfactory.clear_cache();
    strfactory.clean();
    EXPECT_EQ(0, strfactory.size()) << strfactory.print_nodes();
}

TEST_F(MDDTest, SetIntersect)
{
    mdd::mdd_factory<std::string> strfactory;
    EXPECT_EQ(0, strfactory.size());
    {
        mdd::mdd<std::string> m1 = strfactory.empty_set(),
                              m2 = strfactory.empty_set(),
                              el = strfactory.singleton_set();

        m1 += strvec1; // [a]
        m2 += strvec2; // [a, b]
        m2 += strvec3; // [b, c]

        EXPECT_EQ(m1 & m2, m1 & m2 & m2);

        m1 += strvec2;
        m1 += strvec3;

        EXPECT_EQ(m2, m1 & m2);
        EXPECT_EQ(strfactory.empty_set(), m1 & strfactory.empty_set());
        EXPECT_EQ(strfactory.empty_set(), strfactory.empty_set() & m2);
        EXPECT_EQ(strfactory.empty_set(), el & m2);
        EXPECT_EQ(strfactory.empty_set(), m1 & el);
        EXPECT_EQ(el, el & el);

        m1 += strvec0;

        EXPECT_EQ(el, el & m1);
    }
    strfactory.clear_cache();
    strfactory.clean();
    EXPECT_EQ(0, strfactory.size());
}

TEST_F(MDDTest, RelComposition)
{
    typedef mdd::mdd_factory<char> factory_t;
    factory_t factory;
    const int N = 2;
    char R[4][N][N]  = { { {'a', 'a'}, {'b', 'b'} },
                         { {'b', 'b'}, {'c', 'c'} },
                         { {'b', 'b'}, {'d', 'd'} },
                         { {'c', 'c'}, {'e', 'e'} } };
    char E[3][N][N] =  { { {'a', 'a'}, {'c', 'c'} },
                         { {'a', 'a'}, {'d', 'd'} },
                         { {'b', 'b'}, {'e', 'e'} } };
    char c[N][N]     = { {'a', 'a'}, {'e', 'e'} };
    char S[3][N + 1] = { { 'd', 'd', '1' },
                         { 'e', 'e', '2' },
                         { 'f', 'f', '3' } };
    char C[5][N + 1] = { { 'a', 'a', '1' },
                         { 'a', 'a', '2' },
                         { 'b', 'b', '1' },
                         { 'b', 'b', '2' },
                         { 'c', 'c', '2' } };

    EXPECT_EQ(0, factory.size());
    {
        mdd::mdd_irel<char> rel = factory.empty_irel(),
                            result = factory.empty_irel();
        mdd::mdd_srel<char> seq = factory.empty_srel(),
                            sresult = factory.empty_srel();

        for (auto v: R)
            rel.add_in_place(v[0], v[0] + N, v[1], v[1] + N);
        for (auto v: E)
            result.add_in_place(v[0], v[0] + N, v[1], v[1] + N);

        EXPECT_EQ(result, rel.compose(rel));

        result |= rel;
        result.add_in_place(c[0], c[0] + N, c[1], c[1] + N);

        EXPECT_EQ(result, rel.closure());

        for (auto v: S)
            seq.add_in_place(v, v + N + 1);

        seq = result.compose(seq);

        for (auto v: C)
            sresult.add_in_place(v, v + N + 1);

        EXPECT_EQ(sresult, seq);
    }
    factory.clear_cache();
    factory.clean();
    EXPECT_EQ(0, factory.size());
}

class Relabeler
{
    mdd::mdd_factory<int>& m_factory;
    int m_last;
    int m_start;
public:
    Relabeler(mdd::mdd_factory<int>& factory)
        : m_factory(factory)
    {
        reset();
    }
    bool match(size_t level, const mdd::mdd<int>& node)
    {
        return level == 2;
    }
    mdd::mdd<int> replace(size_t, const mdd::mdd<int>& m)
    {
        mdd::mdd<int> result = m_factory.empty_set();
        result.add_in_place(&m_last, reinterpret_cast<int*>(&m_last) + 1);
        ++m_last;
        return result;
    }
    int num() const
    {
        return m_last - m_start;
    }
    void reset(int n=0)
    {
        m_last = n;
        m_start = n;
    }
};

TEST_F(MDDTest, Iterator)
{
    typedef mdd::mdd_factory<int> factory_t;
    factory_t factory;
    mdd::mdd<int> m = factory.empty_set();
    for (auto x: m)
    {
        throw std::runtime_error(testing::PrintToString(x));
    }
}

TEST_F(MDDTest, RelRelabel)
{
    typedef mdd::mdd_factory<int> factory_t;
    factory_t factory;
    int R[8][4]  = { { 0, 0, 1, 1 },
                     { 0, 0, 1, 2 },
                     { 0, 1, 1, 1 },
                     { 0, 1, 1, 2 },
                     { 0, 2, 0, 0 },
                     { 0, 2, 0, 1 },
                     { 0, 3, 0, 0 },
                     { 0, 3, 0, 2 } };

    EXPECT_EQ(0, factory.size());
    {
        mdd::mdd_srel<int> r = factory.empty_srel();

        for (auto v: R)
            r.add_in_place(v, v + 4);

        Relabeler l(factory);
        r = r.relabel(l);
    }
    factory.clear_cache();
    factory.clean();
    EXPECT_EQ(0, factory.size());
}

TEST_F(MDDTest, BisimSignatures)
{
    typedef mdd::mdd_factory<int> factory_t;
    factory_t factory;
    int AT[6][2][2]  = { { {0, 0}, {0, 1} },
                         { {0, 0}, {1, 0} },
                         { {0, 1}, {1, 0} },
                         { {1, 0}, {1, 0} },
                         { {1, 1}, {0, 1} },
                         { {1, 1}, {1, 0} } };
    int AL[4][3]  = { { 0, 0, 12 },
                      { 0, 1, 13 },
                      { 1, 0, 12 },
                      { 1, 1, 12 } };

    EXPECT_EQ(0, factory.size());
    {
        mdd::mdd_srel<int> L = factory.empty_srel();
        mdd::mdd_srel<int> P = factory.empty_srel();
        mdd::mdd_srel<int> P0 = factory.empty_srel();
        mdd::mdd_srel<int> sig = factory.empty_srel();
        mdd::mdd_irel<int> T = factory.empty_irel();
        int oldn, n0;
        for (auto v: AL)
            L.add_in_place(v, v + 3);
        for (auto v: AT)
            T.add_in_place(v[0], v[0] + 2, v[1], v[1] + 2);
        Relabeler l(factory);

        P0 = L.relabel(l);
        n0 = l.num();
        l.reset(n0);
        P = P0.relabel(l);
        oldn = 1;
        while (l.num() != oldn)
        {
            oldn = l.num();
            sig = T.compose(P) | P0;
            l.reset(n0);
            P = sig.relabel(l);
        }

        EXPECT_EQ(P(0)(0), P(1)(1));
        EXPECT_NE(P(0)(0), P(0)(1));
        EXPECT_NE(P(0)(0), P(1)(0));
        EXPECT_NE(P(0)(1), P(1)(0));
        EXPECT_NE(P(0)(1), P(1)(1));
        EXPECT_NE(P(1)(0), P(1)(1));
    }
    factory.clear_cache();
    factory.clean();
    EXPECT_EQ(0, factory.size());
}

TEST_F(MDDTest, BisimKanellakisSmolka)
{
    typedef mdd::mdd_factory<int> factory_t;
    factory_t factory;
    int AT[6][2][2]  = { { {0, 0}, {0, 1} },
                         { {0, 0}, {1, 0} },
                         { {0, 1}, {1, 0} },
                         { {1, 0}, {1, 0} },
                         { {1, 1}, {0, 1} },
                         { {1, 1}, {1, 0} } };
    int P0_0[3][3]  = { { 0, 0 },
                        { 1, 0 },
                        { 1, 1 } };
    int P0_1[1][3]  = { { 0, 1 } };
    int PN_2[1][3]  = { { 1, 0 } };

    EXPECT_EQ(0, factory.size());
    {
        mdd::mdd_irel<int> T = factory.empty_irel();
        std::vector<mdd::mdd<int> > partitions;
        std::vector<mdd::mdd<int> > expected;
        mdd::mdd<int> p[3] = { factory.empty_set(), factory.empty_set(), factory.empty_set() };

        for (auto v: AT)
            T.add_in_place(v[0], v[0] + 2, v[1], v[1] + 2);

        for (auto v: P0_0)
          p[0].add_in_place(v, v + 2);
        for (auto v: P0_1)
          p[1].add_in_place(v, v + 2);
        for (auto v: PN_2)
          p[2].add_in_place(v, v + 2);
        partitions.push_back(p[0]);
        partitions.push_back(p[1]);
        expected.push_back(p[0] - p[2]);
        expected.push_back(p[1]);
        expected.push_back(p[2]);

        auto splitter = partitions.begin();
        while (splitter != partitions.end())
        {
            mdd::mdd<int> p = T.pre(*splitter);
            for (auto& b: partitions)
            {
                mdd::mdd<int> pb = p & b;
                if (!pb.empty() && pb != b)
                {
                    mdd::mdd<int> diff = b - pb;
                    b = pb;
                    partitions.push_back(diff);
                    splitter = partitions.end();
                    break;
                }
            }
            if (splitter == partitions.end())
                splitter = partitions.begin();
            else
                ++splitter;
        }

        std::sort(expected.begin(), expected.end(), [](const mdd::mdd<int>& x, const mdd::mdd<int>& y){ return x.id() < y.id(); });
        std::sort(partitions.begin(), partitions.end(), [](const mdd::mdd<int>& x, const mdd::mdd<int>& y){ return x.id() < y.id(); });
        EXPECT_EQ(expected, partitions);
    }
    factory.clear_cache();
    factory.clean();
    EXPECT_EQ(0, factory.size());
}

TEST(UtilityTest, ZipIterator)
{
    std::vector<int> a { 0, 2, 4 }, b { 1, 3, 5 }, c { 0, 1, 2, 3, 4, 5 };
    mdd::utilities::zip<std::vector<int>::iterator> z(a.begin(), a.end(), b.begin(), b.end());
    std::vector<int> d(z.begin(), z.end());
    EXPECT_EQ(c, d);
}

TEST_F(MDDTest, Projection)
{
    size_t list[3] = { 0, 1, 3 };
    mdd::projection_factory f;
    mdd::projection p = f.create(list, list + 3, 4);
    std::vector<bool> v(p.begin(), p.end());
    EXPECT_EQ(true,  v[0]);
    EXPECT_EQ(true,  v[1]);
    EXPECT_EQ(false, v[2]);
    EXPECT_EQ(true,  v[3]);
    EXPECT_EQ(4, v.size());
}

TEST(Randoms, CacheRecord)
{
    typedef mdd::cacherecord<mdd::node<int> > rectype;
    mdd::node_factory<int> f;
    rectype rec1(mdd::cache_set_union, f.empty(), f.empty(), nullptr);
    rectype rec2(rec1);
    rectype rec3(mdd::cache_set_union, f.emptylist(), f.emptylist(), nullptr);
    rec2.make_clear_operation();
    EXPECT_EQ(rectype::hash()(rec1), rectype::hash()(rec2));
    EXPECT_EQ(true, rectype::equal()(rec2, rec3));
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
