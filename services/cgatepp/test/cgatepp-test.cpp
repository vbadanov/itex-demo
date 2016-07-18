
#include <thread>
#include <mutex>
#include <deque>
#include <sched.h>
#include <iostream>
#include <cstdlib>
#include <chrono>
#include <gtest/gtest.h>
#include <cgatepp/environment.hpp>
#include <cgatepp/connection.hpp>
#include <cgatepp/listener.hpp>
#include <cgatepp/dumper.hpp>

//==============================================================================
TEST(EnvironmentTest, CgateppTest)
{
    ASSERT_NO_THROW(cgatepp::Environment env("ini=./platform/cgatepp/test/settings.ini;key=11111111"));
}

//==============================================================================
TEST(ConnectionTest, CgateppTest)
{
    cgatepp::Environment env("ini=./platform/cgatepp/test/settings.ini;key=11111111");
    cgatepp::Connection* conn = nullptr;

    ASSERT_NO_THROW(conn = new cgatepp::Connection("p2tcp://127.0.0.1:4001;app_name=cgatepp_connection_test"));
    ASSERT_NO_THROW(conn->state());
    EXPECT_EQ(conn->state(), cgatepp::State::CLOSED);
    ASSERT_NO_THROW(conn->open());
    EXPECT_EQ(conn->state(), cgatepp::State::ACTIVE);
    ASSERT_NO_THROW(conn->process(0));
    EXPECT_EQ(conn->state(), cgatepp::State::ACTIVE);
    ASSERT_NO_THROW(conn->close());
    EXPECT_EQ(conn->state(), cgatepp::State::CLOSED);
    ASSERT_NO_THROW(delete conn);
}


//==============================================================================
class Handler : public cgatepp::Listener::IHandler
{
    public:
        void on_repl_online() noexcept
        {
            //std::cout << "===ONLINE===" << std::endl;
        }

        void on_transaction_begin() noexcept
        {
            //std::cout << "===BEGIN===" << std::endl;
        }

        void on_transaction_commit() noexcept
        {
            //std::cout << "===COMMIT===" << std::endl;
        }

        void on_stream_data(cg_msg_streamdata_t* msg) noexcept
        {
            //std::cout << msg->msg_name << std::endl;
        }

};

TEST(ListenerTest, CgateppTest)
{
    cgatepp::Environment env("ini=./platform/cgatepp/test/settings.ini;key=11111111");
    cgatepp::Connection conn("p2tcp://127.0.0.1:4001;app_name=cgatepp_connection_test");
    conn.open();
    while(conn.state() != cgatepp::State::ACTIVE) { };


    Handler handler;

    cgatepp::Listener* listener = nullptr;
    ASSERT_NO_THROW(listener = new cgatepp::Listener(conn, "p2repl://FORTS_FUTAGGR50_REPL;scheme=|FILE|./stage/etc/scheme/orders_aggr.ini|CustReplScheme", &handler));

    EXPECT_EQ(handler.get_listener(), listener);

    ASSERT_NO_THROW(listener->state());
    EXPECT_EQ(listener->state(), cgatepp::State::CLOSED);
    ASSERT_NO_THROW(listener->open("mode=snapshot+online"));

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point current_time = start_time;
    do
    {
        ASSERT_NO_THROW(conn.process(1));
        current_time = std::chrono::steady_clock::now();
    } while (std::chrono::duration_cast<std::chrono::seconds>(current_time - start_time) <= std::chrono::seconds(2));

    EXPECT_EQ(listener->state(), cgatepp::State::ACTIVE);
    ASSERT_NO_THROW(listener->close());
    EXPECT_EQ(listener->state(), cgatepp::State::CLOSED);
    ASSERT_NO_THROW(delete listener);
    conn.close();
}


//============================================================================//
TEST(DumperTest, CgateppTest)
{
    cgatepp::Listener::IHandler handler;
    cgatepp::Dumper("./stage/var/dumper-test.raw", 1, &handler);
}

//============================================================================//
int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
