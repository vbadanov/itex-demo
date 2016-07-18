
#ifndef SQLDB_TRANSACTION_HPP_
#define SQLDB_TRANSACTION_HPP_

#include <sqldb/database.hpp>
#include <sqldb/parameters-list.hpp>

namespace sqldb
{

class Transaction
{
    public:
        struct Mode
        {
            public:
                enum class ImplicitAutocommit
                {
                    ON,    // each statement is committed automatically without START TRANSACTION and COMMIT statements
                    OFF    // in this mode it is required to start and close transaction explicitly (START TRANSACTION / COMMIT / ROLLBACK)
                };

                enum class IsolationLevel  // see PostgreSQL doc for START TRANSACTION statement
                {
                    SERIALIZABLE,
                    REPEATABLE_READ,
                    READ_COMMITTED,
                    READ_UNCOMMITTED
                };

                enum class DataModification  // see PostgreSQL doc for START TRANSACTION statement
                {
                    READ_WRITE,
                    READ_ONLY
                };

                ImplicitAutocommit implicit_autocommit_;
                IsolationLevel isolation_level_;
                DataModification data_modification_;
        };

        Transaction() = delete;
        Transaction(Database& database,
                    Mode::ImplicitAutocommit implicit_autocommit = Mode::ImplicitAutocommit::OFF,
                    Mode::IsolationLevel isolation_level = Mode::IsolationLevel::READ_COMMITTED,
                    Mode::DataModification data_modification = Mode::DataModification::READ_WRITE
                   );
        Transaction(Database* database,
                    Mode::ImplicitAutocommit implicit_autocommit = Mode::ImplicitAutocommit::OFF,
                    Mode::IsolationLevel isolation_level = Mode::IsolationLevel::READ_COMMITTED,
                    Mode::DataModification data_modification = Mode::DataModification::READ_WRITE
                   );
        Transaction(const Transaction&) = delete;
        Transaction(Transaction&& tr);
        ~Transaction();

        Results async_exec(const std::string& query, const ParametersList& param_list, boost::asio::yield_context& yield);
        Results async_exec(const std::string& query, boost::asio::yield_context& yield);

        void async_commit(boost::asio::yield_context& yield);
        void async_rollback(boost::asio::yield_context& yield);

    protected:
        void async_begin(boost::asio::yield_context& yield);

    private:
        Database* db_;
        Connection* connection_;
        Mode mode_;
};

}

#endif //SQLDB_TRANSACTION_HPP_
