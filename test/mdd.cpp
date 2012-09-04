// #define DEBUG_MDD_NODES

#include <gtest/gtest.h>
#include <mdd.h>
#include <algorithm>

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
    char R[4][2 * N] = { { 'a', 'b', 'a', 'b' },
                         { 'b', 'c', 'b', 'c' },
                         { 'b', 'd', 'b', 'd' },
                         { 'c', 'e', 'c', 'e' } };
    char E[3][2 * N] = { { 'a', 'c', 'a', 'c' },
                         { 'a', 'd', 'a', 'd' },
                         { 'b', 'e', 'b', 'e' } };
    char c[2 * N] = { 'a', 'e', 'a', 'e' };
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
            rel.add_in_place(v, v + 2 * N);
        for (auto v: E)
            result.add_in_place(v, v + 2 * N);

        EXPECT_EQ(result, rel.compose(rel));

        result |= rel;
        result.add_in_place(c, c + 2 * N);

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

TEST_F(MDDTest, Bisimulation)
{
    typedef mdd::mdd_factory<int> factory_t;
    factory_t factory;
    int AT[6][4]  = { { 0, 0, 0, 1 },   // 00 --> 01
                      { 0, 1, 0, 0 },   // 00 --> 10
                      { 0, 1, 1, 0 },   // 01 --> 10
                      { 1, 1, 0, 0 },   // 10 --> 10
                      { 1, 0, 1, 1 },   // 11 --> 01
                      { 1, 1, 1, 0 } }; // 11 --> 10
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
            T.add_in_place(v, v + 4);
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
    }
    factory.clear_cache();
    factory.clean();
    EXPECT_EQ(0, factory.size());
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
