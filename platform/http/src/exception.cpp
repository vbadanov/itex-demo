
#include <http/exception.hpp>

namespace
{

#define HTTP_ERRNO_GEN(n, s) s,
    const char* STRINGS[] = {
        HTTP_ERRNO_MAP(HTTP_ERRNO_GEN)
    };
#undef HTTP_ERRNO_GEN

}

namespace http
{

//===============================================================================
Exception::Exception(const std::string& error_text)
    : std::runtime_error(std::string("HTTP: ") + error_text)
{
    //pass
}

//===============================================================================
ParsingException::ParsingException(const std::string& error_text)
    : Exception(error_text)
{
    //pass
}

//===============================================================================
ParsingException::ParsingException(const std::string& error_text, const http_errno& http_parser_error)
    : Exception(error_text + " HTTP-PARSER: " + std::string(STRINGS[http_parser_error]))
{
    //pass
}

//===============================================================================
SerializingException::SerializingException(const std::string& error_text)
    : Exception(error_text)
{
    //pass
}

//===============================================================================
UrlParsingException::UrlParsingException(const std::string& error_text)
    : Exception(error_text)
{
    //pass
}

//===============================================================================
ServerException::ServerException(const std::string& error_text)
    : Exception(error_text)
{
    //pass
}

//===============================================================================
ServerException::ServerException(const std::string& error_text, const boost::system::error_code& ec)
    : Exception(error_text + " BOOST-ASIO: " + ec.message())
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
ConnectionException::ConnectionException(const std::string& error_text, const boost::system::error_code& ec)
    : Exception(error_text + " BOOST-ASIO: " + ec.message())
{
    //pass
}

//===============================================================================
RouterException::RouterException(const std::string& error_text)
    : Exception(error_text)
{
    //pass
}


}
