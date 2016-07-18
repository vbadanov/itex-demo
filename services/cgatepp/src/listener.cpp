
#include <cgatepp/listener.hpp>
#include <cgatepp/exception.hpp>


namespace
{

CG_RESULT listener_base_callback(cg_conn_t* conn, cg_listener_t* listener, struct cg_msg_t* msg, void* data)
{
    cgatepp::Listener::IHandler* handler = static_cast<cgatepp::Listener::IHandler*>(data);

    if(handler == nullptr)
    {
        return CG_ERR_OK;
    }

    switch (msg->type)
    {
        case CG_MSG_OPEN:
            handler->on_listener_open();
            break;

        case CG_MSG_CLOSE:
            handler->on_listener_close();
            break;

        case CG_MSG_TN_BEGIN:
            handler->on_transaction_begin();
            break;

        case CG_MSG_TN_COMMIT:
            handler->on_transaction_commit();
            break;

        case CG_MSG_STREAM_DATA:
            handler->on_stream_data(reinterpret_cast<cg_msg_streamdata_t*>(msg));
            break;

        case CG_MSG_P2REPL_ONLINE:
            handler->on_repl_online();
            break;

        case CG_MSG_P2REPL_LIFENUM:
            handler->on_repl_lifenum(*(static_cast<uint32_t*>(msg->data)));
            break;

        case CG_MSG_P2REPL_CLEARDELETED:
            handler->on_repl_creardeleted(reinterpret_cast<cg_data_cleardeleted_t*>(msg->data));
            break;

        case CG_MSG_P2REPL_REPLSTATE:
            handler->on_repl_replstate(std::string(static_cast<const char*>(msg->data), msg->data_size));
            break;

        case CG_MSG_DATA:
            handler->on_mq_reply(reinterpret_cast<cg_msg_data_t*>(msg));
            break;

        case CG_MSG_P2MQ_TIMEOUT:
            handler->on_mq_timeout(reinterpret_cast<cg_msg_data_t*>(msg));
            break;

        default:
            handler->on_unknown_msg_type(msg);
            break;
    }

    return CG_ERR_OK;
}


}

namespace cgatepp
{

//===============================================================================
void Listener::IHandler::on_listener_open()
{
    //pass
}

void Listener::IHandler::on_listener_close()
{
    //pass
}

void Listener::IHandler::on_transaction_begin()
{
    //pass
}

void Listener::IHandler::on_transaction_commit()
{
    //pass
}

void Listener::IHandler::on_stream_data(cg_msg_streamdata_t* msg)
{
    //pass
}

void Listener::IHandler::on_repl_online()
{
    //pass
}

void Listener::IHandler::on_repl_lifenum(uint32_t lifenum)
{
    //pass
}

void Listener::IHandler::on_repl_creardeleted(cg_data_cleardeleted_t* msg)
{
    //pass
}

void Listener::IHandler::on_repl_replstate(std::string replstate)
{
    //pass
}

void Listener::IHandler::on_mq_reply(cg_msg_data_t* msg)
{
    //pass
}

void Listener::IHandler::on_mq_timeout(cg_msg_data_t* msg)
{
    //pass
}

void Listener::IHandler::on_unknown_msg_type(cg_msg_t* msg)
{
    //pass
}

Listener* Listener::IHandler::get_listener()
{
    return listener_;
}

//===============================================================================
Listener::Listener(Connection& conn, std::string settings, IHandler* handler)
    : conn_(&conn), settings_(settings), cg_listener_(nullptr), handler_(handler)
{
    handler_->listener_ = this;
    CG_RESULT res = cg_lsn_new(conn.get_cg_conn_handle(), settings_.c_str(), &listener_base_callback, handler, &cg_listener_);

    if(res != CG_ERR_OK || cg_listener_ == nullptr)
    {
        throw ListenerException("Can not init listener", res);
    }
}


//===============================================================================
Listener::Listener(Listener&& listener)
{
    conn_ = listener.conn_;
    listener.conn_ = nullptr;

    settings_ = std::move(listener.settings_);

    cg_listener_ = listener.cg_listener_;
    listener.cg_listener_ = nullptr;

    handler_ = listener.handler_;
    listener.handler_ = nullptr;

    settings_open_ = std::move(listener.settings_open_);
}


//===============================================================================
Listener::~Listener()
{
    if(cg_listener_ == nullptr)
    {
        return;
    }
    if(state() != State::CLOSED)
    {
        cg_lsn_close(cg_listener_); // do not worry about return code here, we are in destructor
    }
    CG_RESULT res = cg_lsn_destroy(cg_listener_);
    if(res != CG_ERR_OK && !std::uncaught_exception()) // I know this is bad :-)
    {
        throw ListenerException("Can not destroy listener", res);
    }
    cg_listener_ = nullptr;
}


//===============================================================================
void Listener::open(std::string settings_open)
{
    settings_open_ = settings_open;
    CG_RESULT res = cg_lsn_open(cg_listener_, settings_open_.c_str());
    if(res != CG_ERR_OK)
    {
        throw ListenerException("Can not open listener", res);
    }
}


//===============================================================================
void Listener::close()
{
    CG_RESULT res = cg_lsn_close(cg_listener_);
    if(res != CG_ERR_OK && res != CG_ERR_INCORRECTSTATE)
    {
        throw ListenerException("Can not close listener", res);
    }
}


//===============================================================================
State Listener::state()
{
    uint32_t cg_state;
    CG_RESULT res = cg_lsn_getstate(cg_listener_, &cg_state);
    if(res != CG_ERR_OK)
    {
        throw ListenerException("Can not get listener state", res);
    }

    return map_cg_state(cg_state);
}


//===============================================================================
cg_scheme_desc_t* Listener::get_scheme()
{
    cg_scheme_desc_t* cg_scheme;
    CG_RESULT res = cg_lsn_getscheme(cg_listener_, &cg_scheme);
    if(res != CG_ERR_OK)
    {
        throw ListenerException("Can not get listener scheme", res);
    }
    return cg_scheme;
}

//===============================================================================
Connection* Listener::get_connection()
{
    return conn_;
}


}

