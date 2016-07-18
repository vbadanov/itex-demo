
#ifndef HTTP_EXCEPTION_HPP_
#define HTTP_EXCEPTION_HPP_

#include <stdexcept>
#include <string>

#include <boost/asio/spawn.hpp>
#include <http_parser.h>

namespace http
{

//===============================================================================
class Exception : public std::runtime_error
{
    public:
        Exception(const std::string& error_text);
};

//===============================================================================
class ParsingException : public Exception
{
    public:
        ParsingException(const std::string& error_text);
        ParsingException(const std::string& error_text, const http_errno& http_parser_error);
};

//===============================================================================
class SerializingException : public Exception
{
    public:
        SerializingException(const std::string& error_text);
};

//===============================================================================
class UrlParsingException : public Exception
{
    public:
        UrlParsingException(const std::string& error_text);
};

//===============================================================================
class ServerException : public Exception
{
    public:
        ServerException(const std::string& error_text);
        ServerException(const std::string& error_text, const boost::system::error_code& ec);
};

//===============================================================================
class ConnectionException : public Exception
{
    public:
        ConnectionException(const std::string& error_text);
        ConnectionException(const std::string& error_text, const boost::system::error_code& ec);
};

//===============================================================================
class RouterException : public Exception
{
    public:
        RouterException(const std::string& error_text);
};

}

#endif //HTTP_EXCEPTION_HPP_
