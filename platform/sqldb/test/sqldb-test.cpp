
#include <iostream>
#include <string>
#include <gtest/gtest.h>
#include <sqldb/parameter.hpp>
#include <sqldb/parameters-list.hpp>

#include <boost/regex.hpp>

//==============================================================================
TEST(ParameterTest, SqldbTest)
{
    sqldb::Parameter param001;
    EXPECT_EQ(param001.is_null(), true);
    EXPECT_EQ(param001.c_str(), nullptr);

    sqldb::Parameter param002(sqldb::null);
    EXPECT_EQ(param001.is_null(), true);
    EXPECT_EQ(param002.c_str(), nullptr);

    int i = 100500;
    sqldb::Parameter param003(i);
    EXPECT_EQ(param003.is_null(), false);
    EXPECT_EQ(std::string(param003.c_str()) == std::string("100500"), true);

    double d = 3.141592;
    sqldb::Parameter param004(d);
    EXPECT_EQ(param004.is_null(), false);
    EXPECT_EQ(std::string(param004.c_str()) == std::string("3.141592"), true);

    sqldb::Parameter param005(std::move(param004));
    EXPECT_EQ(param005.is_null(), false);
    EXPECT_EQ(std::string(param005.c_str()) == std::string("3.141592"), true);

    sqldb::Parameter param006(param005);
    EXPECT_EQ(param006.is_null(), false);
    EXPECT_EQ(std::string(param006.c_str()) == std::string("3.141592"), true);

    sqldb::Parameter param007 = std::move(param006);
    EXPECT_EQ(param007.is_null(), false);
    EXPECT_EQ(std::string(param007.c_str()) == std::string("3.141592"), true);

    sqldb::Parameter param008 = param007;
    EXPECT_EQ(param008.is_null(), false);
    EXPECT_EQ(std::string(param008.c_str()) == std::string("3.141592"), true);
}

//==============================================================================
TEST(ParametersListTest, SqldbTest)
{
    sqldb::ParametersList pl;
    EXPECT_EQ(pl.size(), 0UL);

    pl << 100500;
    EXPECT_EQ(pl.size(), 1UL);
    EXPECT_EQ(std::string(pl[0].c_str()) == std::to_string(100500), true);

    pl << 123.456;
    EXPECT_EQ(pl.size(), 2UL);
    EXPECT_EQ(std::string(pl[1].c_str()) == std::to_string(123.456), true);

    pl << "Hello World!";
    EXPECT_EQ(pl.size(), 3UL);
    EXPECT_EQ(std::string(pl[2].c_str()) == std::string("'Hello World!'"), true);

    pl << std::string("qwerty");
    EXPECT_EQ(pl.size(), 4UL);
    EXPECT_EQ(std::string(pl[3].c_str()) == std::string("'qwerty'"), true);

    for(auto& elt : pl)
    {
        elt = std::move(sqldb::Parameter(42));
    }
    for(auto elt : pl)
    {
        EXPECT_EQ(std::string(elt.c_str()) == std::to_string(42), true);
    }

    sqldb::ParametersList pl2 = sqldb::make_params(3, "test", 3.141592, sqldb::null, 42, std::string("Hello World!"), sqldb::null);
    EXPECT_EQ(pl2.size(), 7UL);
    EXPECT_EQ(std::string(pl2[0].c_str()) == sqldb::to_string(3), true);
    EXPECT_EQ(std::string(pl2[1].c_str()) == sqldb::to_string("test"), true);
    EXPECT_EQ(std::string(pl2[2].c_str()) == sqldb::to_string(3.141592), true);
    EXPECT_EQ(pl2[3].c_str() == nullptr, true);
    EXPECT_EQ(std::string(pl2[4].c_str()) == sqldb::to_string(42), true);
    EXPECT_EQ(std::string(pl2[5].c_str()) == sqldb::to_string(std::string("Hello World!")), true);
    EXPECT_EQ(pl2[6].c_str() == nullptr, true);
}

//==============================================================================
TEST(RegexTest, SqldbTest)
{
    std::string s("insert into table1 as t1 inner join table2 as t2 on t1.name = t2.name; select something in another query;");
    boost::regex e("SELECT|INSERT|UPDATE|DELETE|VALUES", boost::regex_constants::ECMAScript | boost::regex_constants::icase | boost::regex_constants::optimize);
    std::cout << "1: search: " << boost::regex_search(s, e, boost::regex_constants::match_any) << std::endl;
    std::cout << "2: match: " << boost::regex_match(s, e, boost::regex_constants::match_any) << std::endl;

    std::string s2("insert into table1 as t1 inner join table2 as t2 on t1.name = t2.name; select something in another query;");
    boost::regex e2(";", boost::regex_constants::ECMAScript | boost::regex_constants::icase | boost::regex_constants::optimize);
    std::cout << "3: search: " << boost::regex_search(s2, e2, boost::regex_constants::match_any) << std::endl;
    std::cout << "4: match: " << boost::regex_match(s2, e2, boost::regex_constants::match_any) << std::endl;

    std::string s3("insert into table1 as t1 inner join table2 as t2 on t1.name = t2.name; select something in another query;");
    boost::regex re_preparable("SELECT|INSERT|UPDATE|DELETE|VALUES", boost::regex_constants::ECMAScript | boost::regex_constants::icase | boost::regex_constants::optimize);
    boost::regex re_multiple_statements(";", boost::regex_constants::ECMAScript | boost::regex_constants::icase | boost::regex_constants::optimize);
    const size_t NUM_ITERATIONS = 10000000;
    bool is_valid_preparable;
    for(unsigned long long i = 0; i < NUM_ITERATIONS; ++i)   // !!! TODO: COMPARE WITH SEPARATE SEARCHES !!!
    {
        is_valid_preparable = (boost::regex_search(s3, re_preparable, boost::regex_constants::match_any) && !boost::regex_search(s3, re_multiple_statements, boost::regex_constants::match_any));
        if(is_valid_preparable || !is_valid_preparable)
        {
            //pass
        }
    }
}



//==============================================================================
int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}

