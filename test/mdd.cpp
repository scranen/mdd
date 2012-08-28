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

TEST_F(MDDTest, CreateIntMDD)
{
    EXPECT_EQ(0, strfactory.size());
    {
        mdd::mdd<std::string> m = strfactory.empty();
        m = m + strvec1; // [10]

        // strfactory.print_nodes(std::cout, m);
        // std::cout << "[----------]\n";

        m = m + strvec2; // [10, 11]

        // strfactory.print_nodes(std::cout, m);
        // std::cout << "[----------]\n";

        m = m + strvec3; // [3, 4]
        m = m + strvec3; // [3, 4]

        // strfactory.print_nodes(std::cout, m);
        // std::cout << "[----------]\n";

        mdd::mdd<std::string> m2 = m | m;
        EXPECT_EQ(m, m2);

        // strfactory.print_nodes(std::cout, m, m2);
        // std::cout << "[----------]\n";

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
        EXPECT_EQ(4, strfactory.size());
    }
    strfactory.clean();
    strfactory.print_nodes(std::cout);
    EXPECT_EQ(0, strfactory.size());
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
