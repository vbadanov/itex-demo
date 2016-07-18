
#include <sqldb/transaction.hpp>

namespace sqldb
{

//===============================================================================
const char* ISOLATION_LEVEL_NAME[] = {
    "SERIALIZABLE",
    "REPEATABLE READ",
    "READ COMMITTED",
    "READ UNCOMMITTED"
};

const char* DATA_MODIFICATION_NAME[] = {
    "READ WRITE",
    "READ ONLY"
};


//===============================================================================
Transaction::Transaction(Database& database, Transaction::Mode::ImplicitAutocommit implicit_autocommit, Transaction::Mode::IsolationLevel isolation_level, Transaction::Mode::DataModification data_modification)
    : db_(&database), connection_(nullptr), mode_({implicit_autocommit, isolation_level, data_modification})
{
    //pass
}

//===============================================================================
Transaction::Transaction(Database* database,  Transaction::Mode::ImplicitAutocommit implicit_autocommit, Transaction::Mode::IsolationLevel isolation_level, Transaction::Mode::DataModification data_modification)
    : db_(database), connection_(nullptr), mode_({implicit_autocommit, isolation_level, data_modification})
{
    //pass
}

//===============================================================================
Transaction::Transaction(Transaction&& tr)
{
    db_ = tr.db_;
    connection_ = tr.connection_;
    mode_ = tr.mode_;

    tr.db_ = nullptr;
    tr.connection_ = nullptr;
}

//===============================================================================
Transaction::~Transaction()
{
    if(connection_ != nullptr)
    {
        /*
         * If we get here, then probably some unhandled exception occurred, or user did not call commit() explicitly (in case of explicit commit mode)
         * For explicit commit mode - just drop connection and do not return it to connection pool of Database instance
         * For implicit commit mode - return connection to pool;
         */
        switch(mode_.implicit_autocommit_)
        {
            case Mode::ImplicitAutocommit::ON:
                db_->return_connection(connection_);
                break;

            case Mode::ImplicitAutocommit::OFF:
                delete connection_;
                //TODO: log error: throw TransactionException("Commit or rollback must be explicitly called before deleting transaction object");
                break;
        }
        connection_ = nullptr;
    }
}

//===============================================================================
Results Transaction::async_exec(const std::string& query, const ParametersList& param_list, boost::asio::yield_context& yield)
{
    if(connection_ == nullptr)
    {
        connection_ = db_->async_get_connection(yield);
        if (mode_.implicit_autocommit_ == Mode::ImplicitAutocommit::OFF)
        {
            async_begin(yield);
        }
    }

    Results results;
    try
    {
        if(param_list.size() > 0)
        {
            results = connection_->async_exec(query, param_list, yield);
        }
        else
        {
            results = connection_->async_exec(query, yield);
        }
    }
    catch (Exception& e)
    {
        if(connection_->status() == Connection::Status::BAD)
        {
            // We can enter here when query execution timeout occurred - in this case connection must be dropped, as query may still be executing in PostgreSQL
            delete connection_;
            connection_ = nullptr;
        }
        else if (mode_.implicit_autocommit_ == Mode::ImplicitAutocommit::OFF)
        {
            async_rollback(yield);
        }
        throw e;
    }

    return results;
}

//===============================================================================
Results Transaction::async_exec(const std::string& query, boost::asio::yield_context& yield)
{
    return async_exec(query, ParametersList(), yield);
}

//===============================================================================
void Transaction::async_begin(boost::asio::yield_context& yield)
{
    if(connection_ == nullptr)
    {
        throw TransactionException("Connection pointer is NULL in transaction object");
    }
    Results res = connection_->async_exec(std::string("START TRANSACTION ISOLATION LEVEL ").append(ISOLATION_LEVEL_NAME[static_cast<int>(mode_.isolation_level_)]).append(" ").append(DATA_MODIFICATION_NAME[static_cast<int>(mode_.data_modification_)]).append(";"), yield);
    if (res[0].status() == QueryResult::Status::BAD)
    {
        throw TransactionException("Start transaction error", res[0].get_pg_result_error());
    }
}

//===============================================================================
void Transaction::async_commit(boost::asio::yield_context& yield)
{
    if (connection_ != nullptr && mode_.implicit_autocommit_ == Mode::ImplicitAutocommit::OFF)
    {
        Results res = connection_->async_exec("COMMIT;", yield);
        if (res[0].status() == QueryResult::Status::BAD)
        {
            throw TransactionException("Commit transaction error", res[0].get_pg_result_error());
        }
        db_->return_connection(connection_);
        connection_ = nullptr;
    }
}

//===============================================================================
void Transaction::async_rollback(boost::asio::yield_context& yield)
{
    if (connection_ != nullptr && mode_.implicit_autocommit_ == Mode::ImplicitAutocommit::OFF)
    {
        Results res = connection_->async_exec("ROLLBACK;", yield);
        if (res[0].status() == QueryResult::Status::BAD)
        {
            throw TransactionException("Rollback transaction error", res[0].get_pg_result_error());
        }
        db_->return_connection(connection_);
        connection_ = nullptr;
    }
}


}

