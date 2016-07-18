#ifndef CGATEPP_ENVIRONMENT_HPP_
#define CGATEPP_ENVIRONMENT_HPP_

#include <string>
#include <cgate.h>

namespace cgatepp
{

class Environment
{
    public:
        Environment(std::string settings);
        Environment(const Environment&) = delete;
        Environment(Environment&&) = delete;
        ~Environment();

    private:
        std::string settings_;
};

}
#endif //CGATEPP_ENVIRONMENT_HPP_
