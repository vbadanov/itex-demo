
#include <http_parser.h>
#include <http/url.hpp>
#include <http/exception.hpp>

namespace
{
inline void assign_field_value(http_parser_url& u, std::string& full_url_buffer, std::string& field, int field_num)
{
    if(u.field_set & (1 << field_num))
    {
        field.assign(full_url_buffer, u.field_data[field_num].off, u.field_data[field_num].len);
    }
}

inline void assign_port_value(http_parser_url& u, uint16_t& port)
{
    if(u.field_set & (1 << UF_PORT))
    {
        port = u.port;
    }
}

}

namespace http
{

//===============================================================================
Url::Url()
    : port_(0)
{
    //pass
}

//===============================================================================
Url::Url(std::string full_url_str)
    : full_(full_url_str), port_(0)
{
    if(full_url_str.empty())
    {
        return;
    }

    http_parser_url u;
    if(http_parser_parse_url(full_.c_str(), full_.size(), 0, &u))
    {
        throw UrlParsingException("Could not parse URL: " + full_);
    }

    assign_field_value(u, full_, schema_, UF_SCHEMA);
    assign_field_value(u, full_, host_, UF_HOST);
    assign_port_value(u, port_);
    assign_field_value(u, full_, path_, UF_PATH);
    assign_field_value(u, full_, query_, UF_QUERY);
    assign_field_value(u, full_, fragment_, UF_FRAGMENT);
    assign_field_value(u, full_, userinfo_, UF_USERINFO);
}

//===============================================================================
Url::Url(const Url& url)
{
    *this = url;
}

//===============================================================================
Url::Url(Url&& url)
{
    *this = url;
}

//===============================================================================
Url& Url::operator=(const Url& url)
{
    full_ = url.full_;
    schema_ = url.schema_;
    host_ = url.host_;
    port_ = url.port_;
    path_ = url.path_;
    query_ = url.query_;
    fragment_ = url.fragment_;
    userinfo_ = url.userinfo_;
    return *this;
}

//===============================================================================
Url& Url::operator=(Url&& url)
{
    full_ = std::move(url.full_);
    schema_ = std::move(url.schema_);
    host_ = std::move(url.host_);
    port_ = std::move(url.port_);
    path_ = std::move(url.path_);
    query_ = std::move(url.query_);
    fragment_ = std::move(url.fragment_);
    userinfo_ = std::move(url.userinfo_);
    return *this;
}

//===============================================================================
Url::~Url()
{
    //pass
}

//===============================================================================
std::string& Url::full()
{
    return full_;
}

//===============================================================================
std::string& Url::schema()
{
    return schema_;
}

//===============================================================================
std::string& Url::host()
{
    return host_;
}

//===============================================================================
uint16_t& Url::port()
{
    return port_;
}

//===============================================================================
std::string& Url::path()
{
    return path_;
}

//===============================================================================
std::string& Url::query()
{
    return query_;
}

//===============================================================================
std::string& Url::fragment()
{
    return fragment_;
}

//===============================================================================
std::string& Url::userinfo()
{
    return userinfo_;
}


}
