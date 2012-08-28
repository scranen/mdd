// #define DEBUG_MDD_NODES

#include <gtest/gtest.h>
#include <mdd.h>

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

    mdd::mdd_factory<std::string> strfactory;
    std::vector<std::string> strvec1;
    std::vector<std::string> strvec2;
    std::vector<std::string> strvec3;
    std::vector<float> floatvec1;
    std::vector<float> floatvec2;
    std::vector<float> floatvec3;
};

TEST_F(MDDTest, CreateStringMDD)
{
    EXPECT_EQ(0, strfactory.size()) << strfactory.print_nodes();
    {
        mdd::mdd<std::string> m = strfactory.empty();
        m = m + strvec1; // [a]
        m = m + strvec2; // [a, b]
        m = m + strvec3; // [b, c]
        m = m + strvec3; // [b, c]

        /*
        std::list<std::vector<int> > contents;
        mdd.dump(contents);

        EXPECT_EQ(3, contents.size());

        auto result = std::find(contents.begin(), contents.end(), intvec1);
        EXPECT_NE(contents.end(), result);
        result = std::find(contents.begin(), contents.end(), intvec2);
        EXPECT_NE(contents.end(), result);
        result = std::find(contents.begin(), contents.end(), intvec3);
        EXPECT_NE(contents.end(), result);

        if (HasFailure())
             ADD_FAILURE() << "contents was:\n" << ::testing::PrintToString(contents);
        */
        strfactory.clean();
        EXPECT_EQ(4, strfactory.size()) << strfactory.print_nodes();
    }
    strfactory.clean();
    EXPECT_EQ(0, strfactory.size()) << strfactory.print_nodes();
}

TEST_F(MDDTest, SetUnion)
{
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

        size_t hits = strfactory.cache_hits(),
               misses = strfactory.cache_misses();

        m1 = strfactory.empty();
        m1 = m1 + strvec1; // [a]
        m2 = strfactory.empty();
        m2 = m2 + strvec2; // [a, b]

        m1 | m2; // Seen before: cache hit
        m2 | m1; // Seen before in different order: cache hit
        m1 | m1; // Union of equal elements: trivial case
        m2 | strfactory.empty(); // Union with empty set: trivial case
        strfactory.empty() | m2; // Union with empty set: trivial case

        EXPECT_EQ(hits + 2, strfactory.cache_hits());
        EXPECT_EQ(misses, strfactory.cache_misses());
    }
    strfactory.clean();
    EXPECT_EQ(0, strfactory.size());
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
