
#include <http/message.hpp>
#include <http/exception.hpp>

namespace
{

#define FLAG_FUNCIONS_BLOCK(func_name, flag_name)  \
bool Message::Flags::func_name()                   \
{                                                  \
    return (flags_ & flag_name) != 0;              \
}                                                  \
void Message::Flags::func_name(const bool& val)    \
{                                                  \
    flags_ |= flag_name;                           \
}                                                  \

}

//===============================================================================
namespace http
{

//===============================================================================
Message::Flags::Flags()
    : flags_(0)
{
    //pass
}

//===============================================================================
Message::Flags::Flags(uint8_t flags)
    : flags_(flags)
{
    //pass
}

//===============================================================================
Message::Flags::Flags(const Flags& flags)
{
    *this = flags;
}

//===============================================================================
Message::Flags::Flags(Flags&& flags)
{
    *this = flags;
}

//===============================================================================
Message::Flags& Message::Flags::operator=(const Flags& flags)
{
    flags_ = flags.flags_;
    return *this;
}

//===============================================================================
Message::Flags& Message::Flags::operator=(Flags&& flags)
{
    flags_ = flags.flags_;
    flags.flags_ = 0;
    return *this;
}

//===============================================================================
Message::Flags::~Flags()
{
    //pass
}

//===============================================================================
FLAG_FUNCIONS_BLOCK(chunked, F_CHUNKED)
FLAG_FUNCIONS_BLOCK(connection_keep_alive, F_CONNECTION_KEEP_ALIVE)
FLAG_FUNCIONS_BLOCK(connection_close, F_CONNECTION_CLOSE)
FLAG_FUNCIONS_BLOCK(trailing, F_TRAILING)
FLAG_FUNCIONS_BLOCK(upgrade, F_UPGRADE)
FLAG_FUNCIONS_BLOCK(skip_body, F_SKIPBODY)
#undef FLAG_FUNCIONS_BLOCK

//===============================================================================
Message::Message()
    : type_(Message::Type::BASE_MESSAGE)
{
    //pass
}

//===============================================================================
Message::Message(const Message& msg)
{
    *this = msg;
}

//===============================================================================
Message::Message(Message&& msg)
{
    *this = msg;
}

//===============================================================================
Message& Message::operator=(const Message& msg)
{
    headers_ = msg.headers_;
    body_ = msg.body_;
    flags_ = msg.flags_;
    return *this;
}

//===============================================================================
Message& Message::operator=(Message&& msg)
{
    headers_ = std::move(msg.headers_);
    body_ = std::move(msg.body_);
    flags_ = std::move(msg.flags_);
    return *this;
}

//===============================================================================
Message::~Message()
{
    headers_.clear();
}

//===============================================================================
Message::HeadersMap& Message::headers()
{
    return headers_;
}
void Message::headers(const Message::HeadersMap& val)
{
    headers_ = val;
}

//===============================================================================
std::string& Message::body()
{
    return body_;
}
void Message::body(const std::string& val)
{
    body_ = val;
}

//===============================================================================
Message::Flags& Message::flags()
{
    return flags_;
}
void Message::flags(const Message::Flags& val)
{
    flags_ = val;
}

//===============================================================================
Message::Type Message::type()
{
    return type_;
}

//===============================================================================
void Message::clear()
{
    headers_.clear();
    body_.clear();
    flags_ = std::move(Flags(0));
}

}
