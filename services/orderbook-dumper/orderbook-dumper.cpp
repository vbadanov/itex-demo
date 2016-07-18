
#include <thread>
#include <iostream>
#include <chrono>
#include <cmath>

#include <cgatepp/environment.hpp>
#include <cgatepp/connection.hpp>
#include <cgatepp/listener.hpp>
#include <cgatepp/dumper.hpp>
#include <cgatepp/exception.hpp>
#include "orders_aggr.cgate.h"

namespace
{
    constexpr size_t GIGABYTE = 1024*1024*1024;
    constexpr size_t MEGABYTE = 1024*1024;
    const std::string LISTENER_OPEN_MODE = std::string("mode=snapshot+online");
}

class OrderBook : public cgatepp::Listener::IHandler
{
    public:
        OrderBook(std::string prefix)
            : prefix_(prefix)
        {
            //pass
        }

        void on_repl_online() noexcept
        {
            std::cout << prefix_ << " ===ONLINE===" << std::endl;
        }

        /*
        void on_transaction_begin() noexcept
        {
            std::cout << prefix_ << "===BEGIN===" << std::endl;
        }

        void on_transaction_commit() noexcept
        {
            std::cout << prefix_ << "===COMMIT===" << std::endl;
        }
        */

        void on_stream_data(cg_msg_streamdata_t* msg) noexcept
        {
            if(msg->msg_index != orders_aggr_index || msg->data_size != sizeof_orders_aggr)
            {
                return;
            }
            orders_aggr* ord_aggr = reinterpret_cast<orders_aggr*>(msg->data);
            int64_t value;
            int8_t scale;
            CG_RESULT result = cg_bcd_get(ord_aggr->price, &value, &scale);
            if (result != CG_ERR_OK)
            {
                fprintf(stderr, "Failed to convert decimal: 0x%X\n", result);
            }
            std::cout << prefix_ << " " << ord_aggr->replID << ", " << ord_aggr->replRev << ", " << ord_aggr->replAct << ", " << ord_aggr->isin_id << ", " << ((double)value/pow(10.0, scale)) << ", " << ord_aggr->volume << ", " << (ord_aggr->dir == 1 ? " BUY" : "SELL") << ",   [" << (int)ord_aggr->moment.year << "-" << (int)ord_aggr->moment.month << "-" << (int)ord_aggr->moment.day << " " << (int)ord_aggr->moment.hour << ":" << (int)ord_aggr->moment.minute << ":" << (int)ord_aggr->moment.second << "." << (int)ord_aggr->moment.msec << "]" << std::endl;
        }

    private:
        std::string prefix_;

};




int main(void)
{
    cgatepp::Environment env("ini=./stage/etc/orderbook-dumper.ini;key=11111111");
    cgatepp::Connection conn("p2tcp://127.0.0.1:4001;app_name=orderbook-dumper");
    conn.open();
    while(conn.state() != cgatepp::State::ACTIVE) { };

    OrderBook orderbook_futures("[FUT]");
    OrderBook orderbook_options("[OPT]");

    cgatepp::Dumper dumper_futures("./stage/var/orderbook-dump-futures.raw", 100*MEGABYTE, &orderbook_futures);
    cgatepp::Dumper dumper_options("./stage/var/orderbook-dump-options.raw", 100*MEGABYTE, &orderbook_options);

    /*
    dumper.replay_recorded_stream(true);
    std::cout << "REPLAY COMPLETED" << std::endl;
    exit(0);
    */

    cgatepp::Listener listener_futures(conn, "p2repl://FORTS_FUTAGGR50_REPL;scheme=|FILE|./stage/etc/scheme/orders_aggr.ini|CustReplScheme", &dumper_futures);
    cgatepp::Listener listener_options(conn, "p2repl://FORTS_OPTAGGR50_REPL;scheme=|FILE|./stage/etc/scheme/orders_aggr.ini|CustReplScheme", &dumper_options);
    listener_futures.open(LISTENER_OPEN_MODE);
    listener_options.open(LISTENER_OPEN_MODE);

    while(1)
    {
        switch(conn.state())
        {
            case cgatepp::State::ERROR:
                conn.close();
                break;

            case cgatepp::State::CLOSED:
                conn.open();
                break;

            case cgatepp::State::ACTIVE:
            {
                switch(listener_futures.state())
                {
                    case cgatepp::State::ERROR:
                        listener_futures.close();
                        break;

                    case cgatepp::State::CLOSED:
                        listener_futures.open(LISTENER_OPEN_MODE);
                        break;

                    default:
                        break;
                }

                switch(listener_options.state())
                {
                    case cgatepp::State::ERROR:
                        listener_options.close();
                        break;

                    case cgatepp::State::CLOSED:
                        listener_options.open(LISTENER_OPEN_MODE);
                        break;

                    default:
                        break;
                }

                try
                {
                    conn.process(1);
                }
                catch (cgatepp::Exception& e)
                {
                    std::cout << e.what() << std::endl;
                }
            }

            default:
                break;
        }
    }

    listener_futures.close();
    listener_options.close();
    conn.close();

    return 0;
}

