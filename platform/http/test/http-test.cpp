
#include <iostream>
#include <string>
#include <thread>
#include <ctime>
#include <gtest/gtest.h>
#include <boost/regex.hpp>
#include <boost/asio/signal_set.hpp>

#include <boost/algorithm/string.hpp>

#include <http/url.hpp>
#include <http/message.hpp>
#include <http/request.hpp>
#include <http/parser.hpp>
#include <http/server.hpp>
#include <http/settings.hpp>
#include <http/router.hpp>

const std::string REQUEST =
        "GET /demo/path/to/test/page?login=user1&password=qwertyuiop#fragment_here HTTP/1.1\r\n"
        "Host: www.example.com\r\n"
        "Connection: Keep-Alive\r\n"
        "Content-Type: text/xml\r\n"
        "Content-Length: 16\r\n"
        "\r\n"
        "<xml>hello</xml>";

//==============================================================================
TEST(UrlParserTest, HttpTest)
{
    const std::string url_string = "http://username:password@www.example.com:8080/path/to/test/page?login=user1&password=qwertyuiop#fragment_here";

    http::Url url(url_string);

    EXPECT_EQ(url.full(), url_string);
    EXPECT_EQ(url.schema(), "http");
    EXPECT_EQ(url.host(), "www.example.com");
    EXPECT_EQ(url.port(), 8080);
    EXPECT_EQ(url.path(), "/path/to/test/page");
    EXPECT_EQ(url.query(), "login=user1&password=qwertyuiop");
    EXPECT_EQ(url.fragment(), "fragment_here");
    EXPECT_EQ(url.userinfo(), "username:password");
}

//==============================================================================
#define FLAG_TEST(flagname)      \
EXPECT_EQ(f.flagname(), false);  \
f.flagname(true);                \
EXPECT_EQ(f.flagname(), true);   \

TEST(MessageFlagsTest, HttpTest)
{
    http::Message::Flags f;
    FLAG_TEST(chunked);
    FLAG_TEST(connection_keep_alive)
    FLAG_TEST(connection_close)
    FLAG_TEST(trailing)
    FLAG_TEST(upgrade)
    FLAG_TEST(skip_body)
#undef FLAG_TEST
}

//==============================================================================
TEST(RequestParserTest, HttpTest)
{
    http::Request req;
    http::Parser parser(&req);

    EXPECT_EQ(parser.feed(REQUEST.c_str(), REQUEST.size()), REQUEST.size());
    EXPECT_EQ(parser.headers_complete(), true);
    EXPECT_EQ(parser.complete(), true);

    EXPECT_EQ(req.method(), http::Method::GET);
    http::Url& url = req.url();
    EXPECT_EQ(url.full(), "/demo/path/to/test/page?login=user1&password=qwertyuiop#fragment_here");
    EXPECT_EQ(url.schema(), "");
    EXPECT_EQ(url.host(), "");
    EXPECT_EQ(url.port(), 0);
    EXPECT_EQ(url.path(), "/demo/path/to/test/page");
    EXPECT_EQ(url.query(), "login=user1&password=qwertyuiop");
    EXPECT_EQ(url.fragment(), "fragment_here");
    EXPECT_EQ(url.userinfo(), "");

    EXPECT_EQ(req.headers()["Host"], "www.example.com");
    EXPECT_EQ(req.headers()["Connection"], "Keep-Alive");
    EXPECT_EQ(req.headers()["Content-Type"], "text/xml");
    EXPECT_EQ(req.headers()["Content-Length"], "16");
    EXPECT_EQ(req.body(), "<xml>hello</xml>");
}


//==============================================================================
int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}

