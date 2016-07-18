#ifndef CGATEPP_CONNECTION_HPP_
#define CGATEPP_CONNECTION_HPP_

#include <string>
#include <cgate.h>
#include <cgatepp/state.hpp>

namespace cgatepp
{

class Connection
{
    public:
        Connection(std::string settings);
        Connection(const Connection&) = delete;
        Connection(Connection&& connection);
        ~Connection();
        void open();
        void close();
        State state();
        void process(uint32_t timeout);

        cg_conn_t* get_cg_conn_handle();

    private:
        std::string settings_;
        cg_conn_t* cg_conn_;
};

}
#endif //CGATEPP_CONNECTION_HPP_
