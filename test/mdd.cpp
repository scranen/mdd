// #define DEBUG_MDD_NODES

#include <gtest/gtest.h>
#include <mdd.h>
#include <algorithm>

namespace std
{
    template<typename value_type>
    void PrintTo(const vector<value_type>& vec, ostream* os) {
        *os << "[";
        for (auto element: vec)
        {
            *os << element << " ";
        }
        *os << "]";
    }

    template<typename value_type>
    void PrintTo(const mdd::mdd<value_type>& m, ostream* os) {
        for (auto vec: m)
        {
            *os << ::testing::PrintToString(vec) << "; ";
        }
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
        mdd::mdd<std::string> m = strfactory.empty();
        m = m + strvec1; // [a]
        m = m + strvec2; // [a, b]
        m = m + strvec3; // [b, c]
        m = m + strvec3; // [b, c]

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
        mdd::mdd<std::string> m1 = strfactory.empty(),
                              m2 = strfactory.empty();
        m1 = m1 + strvec1; // [a]
        m2 = m2 + strvec2; // [a, b]

        EXPECT_EQ(m1 | m2, m1 | m2 | m2) << strfactory.print_nodes(m1, m2);

        m2 = m2 + strvec3; // [b, c]
        m1 = m2 | m1;
        m2 = m1 | m2;

        EXPECT_EQ(m1, m2) << strfactory.print_nodes(m1, m2);

        m1 = strfactory.empty();
        m1 = m1 + strvec1; // [a]
        m2 = strfactory.empty();
        m2 = m2 + strvec2; // [a, b]

        size_t hits = strfactory.cache_hits(),
               misses = strfactory.cache_misses();

        m1 | m2; // Seen before: cache hit
        m2 | m1; // Seen before in different order: cache hit
        m1 | m1; // Union of equal elements: trivial case
        m2 | strfactory.empty(); // Union with empty set: trivial case
        strfactory.empty() | m2; // Union with empty set: trivial case

        EXPECT_EQ(hits + 2, strfactory.cache_hits()) << "m1: " << ::testing::PrintToString(m1)
                                                     << "\nm2: " << ::testing::PrintToString(m2)
                                                     << "\n" << strfactory.print_nodes(m1, m2);
        EXPECT_EQ(misses, strfactory.cache_misses()) << "m1: " << ::testing::PrintToString(m1)
                                                     << "\nm2: " << ::testing::PrintToString(m2)
                                                     << "\n" << strfactory.print_nodes(m1, m2);
    }
    strfactory.clear_cache();
    strfactory.clean();
    EXPECT_EQ(0, strfactory.size());
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
