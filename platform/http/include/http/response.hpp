
#ifndef HTTP_RESPONSE_HPP_
#define HTTP_RESPONSE_HPP_

#include <http/message.hpp>

namespace http
{

class Response : public Message
{
    public:
        Response();
        Response(const Response& res);
        Response(Response&& res);
        Response& operator=(const Response& res);
        Response& operator=(Response&& res);
        ~Response();

        StatusCode& status_code();
        void status_code(const StatusCode& val);

        virtual void clear();

    private:
        StatusCode status_code_;
};

}

#endif //HTTP_RESPONSE_HPP_
