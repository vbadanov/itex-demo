
#include <exception>
#include <cgatepp/connection.hpp>
#include <cgatepp/exception.hpp>

namespace cgatepp
{

//===============================================================================
Connection::Connection(std::string settings)
    : settings_(settings), cg_conn_(nullptr)
{
    CG_RESULT res = cg_conn_new(settings_.c_str(), &cg_conn_);
    if(res != CG_ERR_OK || cg_conn_ == nullptr)
    {
        throw ConnectionException("Can not init connection", res);
    }
}

//===============================================================================
Connection::Connection(Connection&& connection)
{
    settings_ = std::move(connection.settings_);
    cg_conn_ = connection.cg_conn_;
    connection.cg_conn_ = nullptr;
}

//===============================================================================
Connection::~Connection()
{
    if(cg_conn_ == nullptr)
    {
        return;
    }
    if(state() != State::CLOSED)
    {
        cg_conn_close(cg_conn_); // do not worry about return code here, we are in destructor
    }
    CG_RESULT res = cg_conn_destroy(cg_conn_);
    if(res != CG_ERR_OK && !std::uncaught_exception()) // I know this is bad :-)
    {
        throw ConnectionException("Can not destroy connection", res);
    }
    cg_conn_ = nullptr;
}

//===============================================================================
void Connection::open()
{
    CG_RESULT res = cg_conn_open(cg_conn_, nullptr);
    if(res != CG_ERR_OK)
    {
        throw ConnectionException("Can not open connection", res);
    }
}

//===============================================================================
void Connection::close()
{
    CG_RESULT res = cg_conn_close(cg_conn_);
    if(res != CG_ERR_OK && res != CG_ERR_INCORRECTSTATE)
    {
        throw ConnectionException("Can not close connection", res);
    }
}


//===============================================================================
State Connection::state()
{
    uint32_t cg_state;
    CG_RESULT res = cg_conn_getstate(cg_conn_, &cg_state);
    if(res != CG_ERR_OK)
    {
        throw ConnectionException("Can not get connection state", res);
    }

    return map_cg_state(cg_state);
}

//===============================================================================
void Connection::process(uint32_t timeout)
{
    CG_RESULT res = cg_conn_process(cg_conn_, timeout, NULL);
    if(res != CG_ERR_OK && res != CG_ERR_TIMEOUT)
    {
        throw ConnectionException("Can not process connection", res);
    }
}


//===============================================================================
cg_conn_t* Connection::get_cg_conn_handle()
{
    return cg_conn_;
}


}


