#ifndef CGATEPP_EXCEPTION_HPP_
#define CGATEPP_EXCEPTION_HPP_

#include <stdexcept>
#include <cgate.h>

namespace cgatepp
{

//===============================================================================
class Exception : public std::runtime_error
{
    public:
        Exception(const std::string& error_text);
};

//===============================================================================
class EnvironmentException : public Exception
{
    public:
        EnvironmentException(const std::string& error_text);
        EnvironmentException(const std::string& error_text, const CG_RESULT& cg_error_code);
};

//===============================================================================
class ConnectionException : public Exception
{
    public:
        ConnectionException(const std::string& error_text);
        ConnectionException(const std::string& error_text, const CG_RESULT& cg_error_code);
};

//===============================================================================
class ListenerException : public Exception
{
    public:
        ListenerException(const std::string& error_text);
        ListenerException(const std::string& error_text, const CG_RESULT& cg_error_code);
};

//===============================================================================
class DumperException : public Exception
{
    public:
        DumperException(const std::string& error_text);
};


}
#endif //CGATEPP_EXCEPTION_HPP_
