
#ifndef HTTP_REQUEST_HPP_
#define HTTP_REQUEST_HPP_

#include <http/message.hpp>
#include <http/constants.hpp>
#include <http/url.hpp>

namespace http
{

class Request : public Message
{
    public:

        struct RemoteEndpoint
        {
                std::string address;
                uint16_t port;
        };

        Request();
        Request(const Request& req);
        Request(Request&& req);
        Request& operator=(const Request& req);
        Request& operator=(Request&& req);
        virtual ~Request();

        Method& method();
        void method(const Method& val);

        Url& url();
        void url(const Url& val);

        RemoteEndpoint& remote_endpoint();
        void remote_endpoint(const RemoteEndpoint& val);

        virtual void clear();

    private:
        Method method_;
        Url url_;
        RemoteEndpoint remote_endpoint_;
};

}

#endif //HTTP_REQUEST_HPP_
