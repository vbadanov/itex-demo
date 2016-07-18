
#include <http/response.hpp>

namespace http
{

//===============================================================================
Response::Response()
{
    type_ = Message::Type::RESPONSE;
}

//===============================================================================
Response::Response(const Response& res)
{
    *this = res;
}

//===============================================================================
Response::Response(Response&& res)
{
    *this = res;
}

//===============================================================================
Response& Response::operator=(const Response& res)
{
    this->Message::operator=(res);
    status_code_ = res.status_code_;
    return *this;
}

//===============================================================================
Response& Response::operator=(Response&& res)
{
    this->Message::operator=(res);
    status_code_ = std::move(res.status_code_);
    return *this;
}

//===============================================================================
Response::~Response()
{
    //pass
}

//===============================================================================
StatusCode& Response::status_code()
{
    return status_code_;
}
void Response::status_code(const StatusCode& val)
{
    status_code_ = val;
}

//===============================================================================
void Response::clear()
{
    status_code_ = StatusCode::UNKNOWN;
    Message::clear();
}


}
