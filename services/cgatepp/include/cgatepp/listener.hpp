#ifndef CGATEPP_LISTENER_HPP_
#define CGATEPP_LISTENER_HPP_

#include <functional>
#include <cgatepp/connection.hpp>

namespace cgatepp
{

class Listener
{
    public:
        class IHandler
        {
            friend class Listener;

            public:
                virtual void on_listener_open();
                virtual void on_listener_close();

                virtual void on_transaction_begin();
                virtual void on_transaction_commit();

                virtual void on_stream_data(cg_msg_streamdata_t* msg);

                virtual void on_repl_online();
                virtual void on_repl_lifenum(uint32_t lifenum);
                virtual void on_repl_creardeleted(cg_data_cleardeleted_t* msg);
                virtual void on_repl_replstate(std::string replstate);

                virtual void on_mq_reply(cg_msg_data_t* msg);
                virtual void on_mq_timeout(cg_msg_data_t* msg);

                virtual void on_unknown_msg_type(cg_msg_t* msg);

                Listener* get_listener();

            private:
                Listener* listener_;
        };

        Listener(Connection& conn, std::string settings, IHandler* handler);
        Listener(const Listener&) = delete;
        Listener(Listener&& listener);
        ~Listener();
        void open(std::string settings_open);
        void close();
        State state();
        cg_scheme_desc_t* get_scheme();
        Connection* get_connection();

    private:
        Connection* conn_;
        std::string settings_;
        cg_listener_t* cg_listener_;
        IHandler* handler_;
        std::string settings_open_;
};


}
#endif //CGATEPP_LISTENER_HPP_
