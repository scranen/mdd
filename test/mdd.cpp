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
    char v1[2 * N] = { 'a', 'b', 'a', 'b' };
    char v2[2 * N] = { 'b', 'c', 'b', 'c' };
    char v3[2 * N] = { 'b', 'd', 'b', 'd' };
    char v4[2 * N] = { 'c', 'e', 'c', 'e' };
    char r1[2 * N] = { 'a', 'c', 'a', 'c' };
    char r2[2 * N] = { 'a', 'd', 'a', 'd' };
    char r3[2 * N] = { 'b', 'e', 'b', 'e' };
    char c1[2 * N] = { 'a', 'e', 'a', 'e' };
    char s1[N + 1] = { 'e', 'e', '1' };
    char s2[N + 1] = { 'f', 'f', '2' };
    char sc1[N + 1] = { 'a', 'a', '1' };
    char sc2[N + 1] = { 'b', 'b', '1' };
    char sc3[N + 1] = { 'c', 'c', '1' };

    EXPECT_EQ(0, factory.size());
    {
        mdd::mdd_irel<char> rel = factory.empty_irel(),
                            result = factory.empty_irel();
        mdd::mdd_srel<char> seq = factory.empty_srel(),
                            sresult = factory.empty_srel();

        rel.add_in_place(v1, v1 + 2 * N);
        rel.add_in_place(v2, v2 + 2 * N);
        rel.add_in_place(v3, v3 + 2 * N);
        rel.add_in_place(v4, v4 + 2 * N);

        result.add_in_place(r1, r1 + 2 * N);
        result.add_in_place(r2, r2 + 2 * N);
        result.add_in_place(r3, r3 + 2 * N);

        EXPECT_EQ(result, rel.compose(rel));

        result |= rel;
        result.add_in_place(c1, c1 + 2 * N);

        EXPECT_EQ(result, rel.closure());

        seq.add_in_place(s1, s1 + N + 1);
        seq.add_in_place(s2, s2 + N + 1);

        seq = result.compose(seq);

        sresult.add_in_place(sc1, sc1 + N + 1);
        sresult.add_in_place(sc2, sc2 + N + 1);
        sresult.add_in_place(sc3, sc3 + N + 1);

        EXPECT_EQ(sresult, seq);
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
