
#include <sqldb/exception.hpp>

namespace sqldb
{

//===============================================================================
Exception::Exception(const std::string& error_text)
    : std::runtime_error(std::string("SQLDB: ") + error_text)
{
    //pass
}

//===============================================================================
ConnectionException::ConnectionException(const std::string& error_text)
    : Exception(error_text)
{
    //pass
}

//===============================================================================
ConnectionException::ConnectionException(const std::string& error_text, const std::string& pg_error)
    : Exception(error_text + " POSTGRESQL: " + pg_error)
{
    //pass
}

//===============================================================================
ConnectionException::ConnectionException(const std::string& error_text, const boost::system::error_code& ec)
    : Exception(error_text + " BOOST-ASIO: " + ec.message())
{
    //pass
}

//===============================================================================
QueryException::QueryException(const std::string& error_text)
    : ConnectionException(error_text)
{
    //pass
}

//===============================================================================
QueryException::QueryException(const std::string& error_text, const std::string& pg_error)
    : ConnectionException(error_text, pg_error)
{
    //pass
}

//===============================================================================
QueryException::QueryException(const std::string& error_text, const boost::system::error_code& ec)
    : ConnectionException(error_text, ec)
{
    //pass
}

//===============================================================================
ResultException::ResultException(const std::string& error_text)
    : QueryException(error_text)
{
    //pass
}

//===============================================================================
ResultException::ResultException(const std::string& error_text, const std::string& pg_error)
    : QueryException(error_text, pg_error)
{
    //pass
}

//===============================================================================
TransactionException::TransactionException(const std::string& error_text)
    : ConnectionException(error_text)
{
    //pass
}

//===============================================================================
TransactionException::TransactionException(const std::string& error_text, const std::string& pg_error)
    : ConnectionException(error_text, pg_error)
{
    //pass
}



}
