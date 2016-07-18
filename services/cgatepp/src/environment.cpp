
#include <cgate.h>
#include <cgatepp/environment.hpp>
#include <cgatepp/exception.hpp>

namespace cgatepp
{

//===============================================================================
Environment::Environment(std::string settings)
    : settings_(settings)
{
    CG_RESULT res = cg_env_open(settings_.c_str());
    if(res != CG_ERR_OK)
    {
        throw EnvironmentException("Can not create environment", res);
    }
}

//===============================================================================
Environment::~Environment()
{
    cg_env_close();
}

}
