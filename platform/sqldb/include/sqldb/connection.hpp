
#ifndef SQLDB_CONNECTION_HPP_
#define SQLDB_CONNECTION_HPP_

#include <unordered_set>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/regex.hpp>
#include <libpq-fe.h>
#include <aux/hash.hpp>
#include <sqldb/result.hpp>
#include <sqldb/database.hpp>
#include <sqldb/parameters-list.hpp>


namespace sqldb
{

//===============================================================================
class Database;
class Transaction;

//===============================================================================
class Connection
{
    friend Database;
    friend Transaction;

    public:
        enum class Status
        {
            IN_PROGRESS,
            CONNECTED,
            BAD
        };

        enum class TimeoutType
        {
            CONNECTION,
            COMMAND
        };

        Connection() = delete;
        Connection(Connection&) = delete;
        Connection(Connection&&) = delete;

        ~Connection();

        Connection& operator=(Connection&) = delete;
        Connection& operator=(Connection&&) = delete;

        Connection::Status status();
        std::string get_last_error_message();

    protected:
        Connection(Database& database, const std::string& connection_string, const ConnectionSettings& settings);
        void async_connect(boost::asio::yield_context& yield);
        Results async_exec(const std::string& query, const ParametersList& param_list, boost::asio::yield_context& yield);
        Results async_exec(const std::string& query, boost::asio::yield_context& yield);
        Results async_pg_query(const std::string& query, const PostgresqlParametersList& params, boost::asio::yield_context& yield);
        Results async_pg_query(const std::string& query, boost::asio::yield_context& yield);
        Results async_pg_prepare(const std::string& statement_name, const std::string& query, const PostgresqlParametersList& params, boost::asio::yield_context& yield);
        Results async_pg_exec_prepared(const std::string& statement_name, const PostgresqlParametersList& params, boost::asio::yield_context& yield);
        Results async_wait_pg_results(boost::asio::yield_context& yield);

        inline bool is_preparable(const std::string& query);
        inline bool is_multiple_statements(const std::string& query);
        void check_results_errors(Results& results);

        void timeout_handler(const boost::system::error_code& ec);
        void check_timeout_occurred(TimeoutType type);

    private:
        Database& db_;
        std::string connection_string_;
        PGconn* pg_handle_;
        ConnectionSettings settings_;
        std::unordered_set<uint128, aux::hash_of_uint128> prepared_statements_names_list_;
        boost::asio::ip::tcp::socket socket_;
        boost::asio::steady_timer timer_;
        volatile bool timeout_occurred_;
        const boost::regex preparable_sql_statements_regex_;
        const boost::regex multiple_sql_statements_regex_;
};

}

#endif //SQLDB_CONNECTION_HPP_
