
#include <boost/asio/error.hpp>
#include <sqldb/connection.hpp>
#include <aux/hash.hpp>
#include <aux/timeout-guard.hpp>

namespace sqldb
{

static const char* PREPARABLE_SQL_STATEMENTS_REGEX = "SELECT|INSERT|UPDATE|DELETE|VALUES";
//NOTE: The following regex can not be processed by Boost, but works: "(SELECT|INSERT|UPDATE|DELETE|VALUES)(((?!;.*\w+)|(?!;)).)*$"
static const char* MULTIPLE_SQL_STATEMENTS_REGEX = ";";
static const boost::regex_constants::syntax_option_type REGEX_FLAGS = boost::regex_constants::ECMAScript | boost::regex_constants::icase | boost::regex_constants::optimize;

/*
 * Get results in text format.
 * If you change this constant, then rework all places in code to support binary parameters and results
 * (currently leave this in TODO list)
 */
static const int DEFAULT_PG_RESULT_FORMAT = 0;  //Text format

//===============================================================================
Connection::Connection(Database& database, const std::string& connection_string, const ConnectionSettings& settings)
    : db_(database),
      connection_string_(connection_string),
      pg_handle_(nullptr),
      settings_(settings),
      prepared_statements_names_list_(),
      socket_(db_.get_io_service()),
      timer_(db_.get_io_service()),
      timeout_occurred_(false),
      preparable_sql_statements_regex_(PREPARABLE_SQL_STATEMENTS_REGEX, REGEX_FLAGS),
      multiple_sql_statements_regex_(MULTIPLE_SQL_STATEMENTS_REGEX, REGEX_FLAGS)
{
    //pass
}

//===============================================================================
Connection::~Connection()
{
    if(pg_handle_ != nullptr)
    {
        PQfinish(pg_handle_);
        pg_handle_ = nullptr;
    }
}

//===============================================================================
void Connection::async_connect(boost::asio::yield_context& yield)
{
    aux::timeout_guard tg(timer_,
                       std::chrono::milliseconds(settings_.connection_timeout_msec),
                       std::bind(&Connection::timeout_handler, this, std::placeholders::_1)
                      );

    //clear old connection if any
    if(pg_handle_ != nullptr)
    {
        PQfinish(pg_handle_);
        pg_handle_ = nullptr;
    }

    // create new connection
    pg_handle_ = PQconnectStart(connection_string_.c_str());
    if(pg_handle_ == nullptr || (pg_handle_ != nullptr && status() == Connection::Status::BAD))
    {
        throw ConnectionException("Can not allocate/create PGConn", get_last_error_message());
    }

    if(PQsetnonblocking(pg_handle_, 1)) // returns 0 on OK
    {
        throw ConnectionException("Can not set nonblocking mode on PGConn", get_last_error_message());
    }

    socket_.assign(boost::asio::ip::tcp::v4(), PQsocket(pg_handle_));

    bool connected = false;
    boost::system::error_code ec;
    while(!connected)
    {
        switch(PQconnectPoll(pg_handle_))
        {
            case PGRES_POLLING_READING:
                socket_.async_read_some(boost::asio::null_buffers(), yield[ec]);
                check_timeout_occurred(TimeoutType::CONNECTION);
                if (ec)
                {
                    throw ConnectionException("Connection failed - read error", ec);
                }
                break;

            case PGRES_POLLING_WRITING:
                socket_.async_write_some(boost::asio::null_buffers(), yield[ec]);
                check_timeout_occurred(TimeoutType::CONNECTION);
                if (ec)
                {
                    throw ConnectionException("Connection failed - write error", ec);
                }
                break;

            case PGRES_POLLING_FAILED:
                check_timeout_occurred(TimeoutType::CONNECTION);
                throw ConnectionException("Connection failed", get_last_error_message());
                break;

            case PGRES_POLLING_OK:
                connected = true;
                break;

            default:
            case PGRES_POLLING_ACTIVE: // unused, according to notes in PosgreSQL source code
                break;
        }
    }

    check_timeout_occurred(TimeoutType::CONNECTION);

    // Connection is OK here

    Results res = async_pg_query("SET bytea_output = hex", yield);
    check_results_errors(res);
}

//===============================================================================
Results Connection::async_exec(const std::string& query, const ParametersList& param_list, boost::asio::yield_context& yield)
{
    aux::timeout_guard tg(timer_,
                       std::chrono::milliseconds(settings_.command_timeout_msec),
                       std::bind(&Connection::timeout_handler, this, std::placeholders::_1)
                      );

    PostgresqlParametersList pg_params(param_list);
    Results res;

    if(!is_preparable(query) || is_multiple_statements(query))
    {
        res = async_pg_query(query, pg_params, yield);
    }
    else
    {
        uint128 statement_hash = aux::hash(query);
        std::string statement_name = aux::to_string(statement_hash);
        if(prepared_statements_names_list_.find(statement_hash) == prepared_statements_names_list_.end())
        {
            // Statement has not been prepared yet, need to prepare it here
            res = async_pg_prepare(statement_name, query, pg_params, yield);
            if(res[0].status() == QueryResult::Status::BAD)
            {
                throw QueryException("Could not prepare statement", res[0].get_pg_result_error());
            }
            prepared_statements_names_list_.insert(statement_hash);
        }

        res = async_pg_exec_prepared(statement_name, pg_params, yield);
    }

    check_results_errors(res);
    return res;
}

//===============================================================================
Results Connection::async_exec(const std::string& query, boost::asio::yield_context& yield)
{
    aux::timeout_guard tg(timer_,
                       std::chrono::milliseconds(settings_.command_timeout_msec),
                       std::bind(&Connection::timeout_handler, this, std::placeholders::_1)
                      );

    if (is_multiple_statements(query))
    {
        Results res = async_pg_query(query, yield);
        check_results_errors(res);
        return res;
    }
    else
    {
        return async_exec(query, ParametersList(), yield);
    }
}

//===============================================================================
Connection::Status Connection::status()
{
    if(pg_handle_ == nullptr || timeout_occurred_)
    {
        return Connection::Status::BAD;
    }

    ConnStatusType pg_status = PQstatus(pg_handle_);
    switch(pg_status)
    {
        case CONNECTION_OK:
            return Connection::Status::CONNECTED;

        case CONNECTION_BAD:
            return Connection::Status::BAD;

        default:
            return Connection::Status::IN_PROGRESS;
    }
}

//===============================================================================
std::string Connection::get_last_error_message()
{
    return std::string(PQerrorMessage(pg_handle_));
}

//===============================================================================
Results Connection::async_pg_query(const std::string& query, const PostgresqlParametersList& params, boost::asio::yield_context& yield)
{
    int ret = 0;
    ret = PQsendQueryParams(pg_handle_,
                            query.c_str(),
                            params.get_n_params(),
                            params.get_param_types(),
                            params.get_param_values(),
                            params.get_param_lengths(),
                            params.get_param_formats(),
                            DEFAULT_PG_RESULT_FORMAT
                           );
    if(!ret)
    {
        throw QueryException("PQsendQueryParams failed", get_last_error_message());
    }

    return async_wait_pg_results(yield);
}

//===============================================================================
Results Connection::async_pg_query(const std::string& query, boost::asio::yield_context& yield)
{
    int ret = 0;
    ret = PQsendQuery(pg_handle_, query.c_str());

    if(!ret)
    {
        throw QueryException("PQsendQuery failed", get_last_error_message());
    }

    return async_wait_pg_results(yield);
}

//===============================================================================
Results Connection::async_pg_prepare(const std::string& statement_name, const std::string& query, const PostgresqlParametersList& params, boost::asio::yield_context& yield)
{
    int ret = 0;
    ret = PQsendPrepare(pg_handle_,
                        statement_name.c_str(),
                        query.c_str(),
                        params.get_n_params(),
                        params.get_param_types()
                       );
    if(!ret)
    {
        throw QueryException("PQsendPrepare failed", get_last_error_message());
    }

    return async_wait_pg_results(yield);
}

//===============================================================================
Results Connection::async_pg_exec_prepared(const std::string& statement_name, const PostgresqlParametersList& params, boost::asio::yield_context& yield)
{
    int ret = 0;
    ret = PQsendQueryPrepared(pg_handle_,
                              statement_name.c_str(),
                              params.get_n_params(),
                              params.get_param_values(),
                              params.get_param_lengths(),
                              params.get_param_formats(),
                              DEFAULT_PG_RESULT_FORMAT
                             );
    if(!ret)
    {
        throw QueryException("PQsendQueryPrepared failed", get_last_error_message());
    }

    return async_wait_pg_results(yield);
}

//===============================================================================
Results Connection::async_wait_pg_results(boost::asio::yield_context& yield)
{
    PGresult* res = nullptr;
    Results results;
    boost::system::error_code ec;
    do // there are could be several commands in each query, thus we will receive several results pointers for that composite query
    {
        do
        {
            socket_.async_read_some(boost::asio::null_buffers(), yield[ec]);
            check_timeout_occurred(TimeoutType::COMMAND);
            if (ec)
            {
                throw QueryException("Query failed - read error", ec);
            }

            if(!PQconsumeInput(pg_handle_))
            {
                check_timeout_occurred(TimeoutType::COMMAND);
                throw QueryException("PQconsumeInput failed", get_last_error_message());
            }
        } while (PQisBusy(pg_handle_));

        do
        {
            res = PQgetResult(pg_handle_);
            check_timeout_occurred(TimeoutType::COMMAND);
            if (res != nullptr)
            {
                // We must complete results reception before throwing any exceptions for bad results
                QueryResult query_result(res);
                results.add(std::move(query_result));
            }
        } while (!PQisBusy(pg_handle_) && res != nullptr);
    } while(res != nullptr);

    check_timeout_occurred(TimeoutType::COMMAND);

    return std::move(results);
}

//===============================================================================
inline bool Connection::is_preparable(const std::string& query)
{
    return boost::regex_search(query, preparable_sql_statements_regex_, boost::regex_constants::match_any);
}

//===============================================================================
inline bool Connection::is_multiple_statements(const std::string& query)
{
    return boost::regex_search(query, multiple_sql_statements_regex_, boost::regex_constants::match_any);
}

//===============================================================================
void Connection::check_results_errors(Results& results)
{
    for(auto& query_result : results)
    {
        if(query_result.status() == QueryResult::Status::BAD)
        {
            throw ResultException("Bad query result", query_result.get_pg_result_error());
        }
    }
}
//===============================================================================
void Connection::timeout_handler(const boost::system::error_code& ec)
{
    if (ec == boost::asio::error::operation_aborted)
    {
        return;
    }

    /*
     * WARNING:
     * Query can be in progress on PosgreSQL server when timeout occurred!
     * We do not cancel that query here (it probably creates a separate thread inside libpq to handle that),
     * user code must delete and recreate connection object (request it from the connection pool) - this is one of the job for Transaction object
     */

    timeout_occurred_ = true;
    socket_.close();
}

//===============================================================================
void Connection::check_timeout_occurred(TimeoutType type)
{
    if(timeout_occurred_)
    {
        switch(type)
        {
            case TimeoutType::CONNECTION:
                throw ConnectionException("Connection timeout");
                break;

            case TimeoutType::COMMAND:
                throw QueryException("Query execution timeout");
                break;

            default:
                throw Exception("Unknown type timeout"); // :-) we should not enter here
                break;
        }
    }
}



}


