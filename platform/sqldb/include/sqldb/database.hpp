
#ifndef SQLDB_DATABASE_HPP_
#define SQLDB_DATABASE_HPP_

#include <deque>
#include <mutex>
#include <string>
#include <boost/asio/spawn.hpp>
#include <boost/asio/steady_timer.hpp>
#include <aux/spinlock.hpp>
#include <sqldb/settings.hpp>
#include <sqldb/connection.hpp>
#include <sqldb/exception.hpp>

namespace sqldb
{

//===============================================================================
class Connection;
class Transaction;

//===============================================================================
struct PoolStats
{
    void update(size_t value);
    size_t cumulative_size;
    size_t counter;
};

//===============================================================================
class Database
{
    friend Connection;
    friend Transaction;

    public:
        Database(boost::asio::io_service& is, const std::string& connection_string, const ConnectionPoolSettings& conn_pool_settings);
        ~Database();

        Database() = delete;
        Database(Database&) = delete;
        Database(Database&& db) = delete;
        Database& operator=(Database&) = delete;
        Database& operator=(Database&& db) = delete;

        boost::asio::io_service& get_io_service();

    protected:
        Connection* async_get_connection(boost::asio::yield_context& yield);
        void return_connection(Connection* connection);

        void pool_shrink_timer_handler(const boost::system::error_code& ec);

    private:
        boost::asio::io_service& is_;
        std::string connection_string_;
        ConnectionPoolSettings pool_settings_;
        std::deque<Connection*> pool_queue_;
        aux::spinlock pool_lock_;
        boost::asio::steady_timer pool_shrink_timer_;
        PoolStats pool_stats_per_period_;
};


}

#endif //SQLDB_DATABASE_HPP_
