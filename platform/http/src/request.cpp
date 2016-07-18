
#include <http/request.hpp>

namespace http
{

//===============================================================================
Request::Request()
{
    type_ = Message::Type::REQUEST;
}

//===============================================================================
Request::Request(const Request& req)
{
    *this = req;
}

//===============================================================================
Request::Request(Request&& req)
{
    *this = req;
}

//===============================================================================
Request& Request::operator=(const Request& req)
{
    this->Message::operator=(req);
    method_ = req.method_;
    url_ = req.url_;
    remote_endpoint_ = req.remote_endpoint_;
    return *this;
}

//===============================================================================
Request& Request::operator=(Request&& req)
{
    this->Message::operator=(req);
    method_ = std::move(req.method_);
    url_ = std::move(req.url_);
    remote_endpoint_ = std::move(req.remote_endpoint_);
    return *this;
}

//===============================================================================
Request::~Request()
{
    //pass
}

//===============================================================================
Method& Request::method()
{
    return method_;
}
void Request::method(const Method& val)
{
    method_ = val;
}

//===============================================================================
Url& Request::url()
{
    return url_;
}
void Request::url(const Url& val)
{
    url_ = val;
}

//===============================================================================
Request::RemoteEndpoint& Request::remote_endpoint()
{
    return remote_endpoint_;
}
void Request::remote_endpoint(const Request::RemoteEndpoint& val)
{
    remote_endpoint_ = val;
}

//===============================================================================
void Request::clear()
{
    method_ = Method::UNKNOWN;
    url_ = std::move(Url());
    remote_endpoint_.address.clear();
    remote_endpoint_.port = 0;
    Message::clear();
}


}
