
#ifndef SQLDB_EXCEPTION_HPP_
#define SQLDB_EXCEPTION_HPP_

#include <boost/asio/spawn.hpp>
#include <libpq-fe.h>


namespace sqldb
{

//===============================================================================
class Exception : public std::runtime_error
{
    public:
        Exception(const std::string& error_text);
};

//===============================================================================
class ConnectionException : public Exception
{
    public:
        ConnectionException(const std::string& error_text);
        ConnectionException(const std::string& error_text, const std::string& pg_error);
        ConnectionException(const std::string& error_text, const boost::system::error_code& ec);
};

//===============================================================================
class QueryException : public ConnectionException
{
    public:
        QueryException(const std::string& error_text);
        QueryException(const std::string& error_text, const std::string& pg_error);
        QueryException(const std::string& error_text, const boost::system::error_code& ec);
};

//===============================================================================
class ResultException : public QueryException
{
    public:
        ResultException(const std::string& error_text);
        ResultException(const std::string& error_text, const std::string& pg_error);
};

//===============================================================================
class TransactionException : public ConnectionException
{
    public:
        TransactionException(const std::string& error_text);
        TransactionException(const std::string& error_text, const std::string& pg_error);
};


}

#endif //SQLDB_EXCEPTION_HPP_
