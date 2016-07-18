
#include <string>
#include <sstream>
#include <iostream>
#include <thread>
#include <mutex>
#include <boost/asio/write.hpp>
#include <aux/hash.hpp>
#include <aux/timeout-guard.hpp>
#include <http/connection.hpp>
#include <http/parser.hpp>
#include <http/request.hpp>
#include <http/response.hpp>
#include <http/exception.hpp>


namespace http
{

//===============================================================================
Connection::Connection(Server* server, boost::asio::ip::tcp::socket&& socket, const ConnectionSettings& settings)
    : server_(server),
      socket_(server_->get_io_service()),
      timer_(server_->get_io_service()),
      strand_(server->get_io_service()),
      settings_(settings),
      handler_(nullptr),
      running_(false),
      timeout_occurred_(false),
      deletion_sheduled_(false),
      data_buffer_(nullptr)
{
    data_buffer_ = new char[settings_.buffer_size_bytes];
    reinit(std::move(socket));
}

//===============================================================================
Connection::~Connection()
{
    timer_.cancel();
    if(handler_ != nullptr)
    {
        delete handler_;
    }

    if(data_buffer_ != nullptr)
    {
        delete [] data_buffer_;
    }
}

//===============================================================================
void Connection::reinit(boost::asio::ip::tcp::socket&& socket)
{
    socket_ = std::move(socket);
    running_ = false;
    timeout_occurred_ = false;
    deletion_sheduled_ = false;

    boost::asio::ip::tcp::endpoint remote_endpoint = socket_.remote_endpoint();
    aux::hash_of_uint128 id_gen;
    id_ = id_gen(aux::hash(
                    std::to_string(remote_endpoint.protocol().type()) + "-" +
                    remote_endpoint.address().to_string() + "-" +
                    std::to_string(remote_endpoint.port())
          ));
}

//===============================================================================
void Connection::start()
{
    running_ = true;

    boost::asio::spawn(strand_, [&](boost::asio::yield_context yield)
    {
        try
        {
            boost::asio::ip::tcp::endpoint remote_endpoint = socket_.remote_endpoint();
            http::Request request;
            http::Response response;
            http::Parser parser(&request);
            boost::system::error_code ec;
            bool drop_connection_after_response = false;

            if(handler_ != nullptr) // re-init
            {
                handler_->reinit(yield);
            }

            while(socket_.is_open() && running_)
            {
                ec.clear();
                parser.clear();
                request.clear();
                response.clear();
                timeout_occurred_ = false;

                while(!parser.complete())
                {
                    std::size_t n = socket_.async_read_some(boost::asio::buffer(data_buffer_, settings_.buffer_size_bytes), yield/*[ec]*/);
                    parser.feed(data_buffer_, n);
                }

                request.remote_endpoint({remote_endpoint.address().to_string(), static_cast<uint16_t>(remote_endpoint.port())});


                if(request.flags().connection_close())
                {
                    drop_connection_after_response = true;
                }

                try
                {
                    aux::timeout_guard tg(timer_,
                                          std::chrono::milliseconds(settings_.timeout_msec),
                                          std::bind(&Connection::timeout_handler, this, std::placeholders::_1)
                                         );

                    if(handler_ == nullptr)  //deferred creation
                    {
                        handler_ = settings_.request_handler_factory->create_handler(yield);
                        if(handler_ == nullptr)
                        {
                        	throw ConnectionException("Request handler not ready yet. Please try again later.");
                        }
                    }

                    response = handler_->process_request(*this, request, yield);
                    check_timeout_occurred();  //TODO: possible situation: on real timeout connection is dropped, but handler can still be running...
                }
                catch (std::exception& e)
                {
                    // TODO: implement logging of e.what()
                    response.status_code(StatusCode::CODE_500_INTERNAL_SERVER_ERROR);
                    response.body(e.what());
                    std::cout << "CONNECTION Got exception: " << e.what() << std::endl;
                }

                std::stringstream result_ss;
                result_ss << "HTTP/1.1 " << status_code_to_sting.at(response.status_code()) << "\r\n";
                for(auto header : response.headers())
                {
                    result_ss << header.first << ": " << header.second << "\r\n";
                }

                result_ss << "Content-Length: " << std::to_string(response.body().size()) << "\r\n\r\n";
                result_ss << response.body();

                boost::asio::async_write(socket_, boost::asio::buffer(result_ss.str()), yield/*[ec]*/);

                if(drop_connection_after_response)
                {
                    break;
                }
            }
        }
        catch (std::exception& e)
        {
            //TODO: add logging
            //pass
        }

        kill();
     });
}

//===============================================================================
void Connection::stop()
{
    running_ = false;
    boost::system::error_code ignored_ec;
    socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
    socket_.close(ignored_ec);
    timer_.cancel(ignored_ec);
}

//===============================================================================
bool Connection::is_running()
{
    return running_;
}

//===============================================================================
bool Connection::is_timeout_occurred()
{
    return timeout_occurred_;
}

//===============================================================================
bool Connection::is_ok()
{
    return (running_ && !timeout_occurred_);
}

//===============================================================================
size_t Connection::get_id()
{
    return id_;
}

//===============================================================================
Server& Connection::get_server()
{
	return *server_;
}

//===============================================================================
void Connection::timeout_handler(const boost::system::error_code& ec)
{
    if (ec == boost::asio::error::operation_aborted)
    {
        return;
    }

    // TODO: implement logging
    //commented to avoid dropping connection: this->stop();
    timeout_occurred_ = true;
}

//===============================================================================
void Connection::check_timeout_occurred()
{
    if(timeout_occurred_)
    {
        throw ConnectionException("Processing timeout");
    }
}

//===============================================================================
void Connection::kill()
{
    if (!deletion_sheduled_)
    {
        this->stop();
        server_->schedule_connection_delete(id_);
        deletion_sheduled_ = true;
    }
}


}
