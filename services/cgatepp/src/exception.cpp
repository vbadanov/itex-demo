
#include <cgatepp/exception.hpp>

namespace cgatepp
{

//===============================================================================
Exception::Exception(const std::string& error_text)
    : std::runtime_error(std::string("CGATEPP: ") + error_text)
{
    //pass
}

//===============================================================================
EnvironmentException::EnvironmentException(const std::string& error_text)
    : Exception(error_text)
{
    //pass
}

//===============================================================================
EnvironmentException::EnvironmentException(const std::string& error_text, const CG_RESULT& cg_error_code)
    : Exception(error_text + " CGATE: " + cg_err_getstr(cg_error_code))
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
ConnectionException::ConnectionException(const std::string& error_text, const CG_RESULT& cg_error_code)
    : Exception(error_text + " CGATE: " + cg_err_getstr(cg_error_code))
{
    //pass
}

//===============================================================================
ListenerException::ListenerException(const std::string& error_text)
    : Exception(error_text)
{
    //pass
}

//===============================================================================
ListenerException::ListenerException(const std::string& error_text, const CG_RESULT& cg_error_code)
    : Exception(error_text + " CGATE: " + cg_err_getstr(cg_error_code))
{
    //pass
}

//===============================================================================
DumperException::DumperException(const std::string& error_text)
    : Exception(error_text)
{
    //pass
}

}
