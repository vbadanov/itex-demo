
#include <functional>
#include <cstring>
#include <iostream>

#include <http/parser.hpp>
#include <http/exception.hpp>
#include <http/request.hpp>
#include <http/response.hpp>
#include <http/url.hpp>
#include <http/constants.hpp>

namespace http
{

//===============================================================================
Parser::Parser(Message* msg)
    : message_(msg), parsing_complete_(false), headers_parsing_complete_(false)
{
    settings_ = new http_parser_settings;
    parser_ = new http_parser;

    settings_->on_message_begin    = &Parser::on_message_begin;
    settings_->on_message_complete = &Parser::on_message_complete;

    settings_->on_header_field     = &Parser::on_header_field;
    settings_->on_header_value     = &Parser::on_header_value;
    settings_->on_headers_complete = &Parser::on_headers_complete;

    settings_->on_body = &Parser::on_body;

    switch(message_->type())
    {
        case Message::Type::REQUEST:
        {
            http_parser_init(parser_, HTTP_REQUEST);
            settings_->on_url = &Parser::on_url;
            break;
        }

        case Message::Type::RESPONSE:
        {
            http_parser_init(parser_, HTTP_RESPONSE);
            settings_->on_status = &Parser::on_status;
            break;
        }

        case Message::Type::BASE_MESSAGE:
        default:
            throw ParsingException("Parser was associated with message of incorrect type");
    }
    parser_->data = this;
}

//===============================================================================
Parser::Parser(const Parser& parser)
{
    *this = parser;
}

//===============================================================================
Parser::Parser(Parser&& parser)
{
    *this = parser;
}

//===============================================================================
Parser& Parser::operator=(const Parser& parser)
{
    message_ = parser.message_;
    settings_ = parser.settings_;
    parser_ = parser.parser_;
    current_header_ = parser.current_header_;
    parsing_complete_ = parser.parsing_complete_;
    headers_parsing_complete_ = parser.headers_parsing_complete_;
    return *this;
}

//===============================================================================
Parser& Parser::operator=(Parser&& parser)
{
    message_ = std::move(parser.message_);
    settings_ = std::move(parser.settings_);
    parser_ = std::move(parser.parser_);
    current_header_ = std::move(parser.current_header_);
    parsing_complete_ = std::move(parser.parsing_complete_);
    headers_parsing_complete_ = std::move(parser.headers_parsing_complete_);
    return *this;
}

//===============================================================================
Parser::~Parser()
{
    delete parser_;
    delete settings_;
}

//===============================================================================
size_t Parser::feed_step(const char* data, size_t size)
{
    size_t used = http_parser_execute(parser_, settings_, data, size);

    const http_errno error = static_cast<http_errno>(parser_->http_errno);

    // The 'on_message_complete' and 'on_headers_complete' callbacks fail
    // on purpose to force the parser to stop between pipelined requests.
    // This allows the clients to reliably detect the end of headers and
    // the end of the message.  Make sure the parser is always unpaused
    // for the next call to 'feed'.
    if (error == HPE_PAUSED)
    {
        http_parser_pause(parser_, 0);
    }

    if (used < size)
    {
        if (error == HPE_PAUSED)
        {
            // Make sure the byte that triggered the pause
            // after the headers is properly processed.
            if (!parsing_complete_)
            {
                used += http_parser_execute(parser_, settings_, data + used, 1);
            }
        }
        else {
            throw ParsingException("Parser::Feed error", error);
        }
    }

    return used;
}

//===============================================================================
size_t Parser::feed(const char* data, size_t size)
{
    size_t used = 0;
    while ((size > 0) && !parsing_complete_)
    {
        std::size_t pass = feed_step(data, size);
        used += pass;
        data += pass;
        size -= pass;
    }
    return used;
}

//===============================================================================
void Parser::clear()
{
    current_header_.first.clear();
    current_header_.second.clear();

    parsing_complete_ = false;
    headers_parsing_complete_ = false;

    switch(message_->type())
    {
        case Message::Type::REQUEST:
        {
            http_parser_init(parser_, HTTP_REQUEST);
            break;
        }

        case Message::Type::RESPONSE:
        {
            http_parser_init(parser_, HTTP_RESPONSE);
            break;
        }

        case Message::Type::BASE_MESSAGE:
        default:
            throw ParsingException("Parser was associated with message of incorrect type");
    }
}

//===============================================================================
bool Parser::complete()
{
    return parsing_complete_;
}

//===============================================================================
bool Parser::headers_complete()
{
    return headers_parsing_complete_;
}

//===============================================================================
uint32_t Parser::version()
{
    return static_cast<uint32_t>(http_parser_version());
}

//===============================================================================
int Parser::on_message_begin(http_parser* httpparser)
{
    Parser* parser = static_cast<Parser*>(httpparser->data);
    parser->parsing_complete_ = false;
    parser->headers_parsing_complete_ = false;
    return 0;
}

//===============================================================================
int Parser::on_url(http_parser* httpparser, const char* data, size_t size)
{
    Parser* parser = static_cast<Parser*>(httpparser->data);
    Request* req = static_cast<Request*>(parser->message_);
    req->url(Url(std::string(data, size)));
    req->method(method_map.at(static_cast<http_method>(httpparser->method)));
    return 0;
}

//===============================================================================
int Parser::on_status(http_parser* httpparser, const char* data, size_t size)
{
    Parser* parser = static_cast<Parser*>(httpparser->data);
    static_cast<Response*>(parser->message_)->status_code(num_to_status_code.at(httpparser->status_code));
    return 0;
}

//===============================================================================
int Parser::on_header_field(http_parser* httpparser, const char* data, size_t size)
{
    Parser* parser = static_cast<Parser*>(httpparser->data);
    if (!parser->current_header_.second.empty())
    {
        parser->message_->headers()[parser->current_header_.first] = parser->current_header_.second;
        parser->current_header_.first.clear();
        parser->current_header_.second.clear();
    }
    parser->current_header_.first.append(data, size);
    return 0;
}

//===============================================================================
int Parser::on_header_value(http_parser* httpparser, const char* data, size_t size)
{
    Parser* parser = static_cast<Parser*>(httpparser->data);
    parser->current_header_.second.append(data, size);
    return 0;
}

//===============================================================================
int Parser::on_headers_complete(http_parser* httpparser)
{
    Parser* parser = static_cast<Parser*>(httpparser->data);
    if (!parser->current_header_.second.empty())
    {
        parser->message_->headers()[parser->current_header_.first] = parser->current_header_.second;
        parser->current_header_.first.clear();
        parser->current_header_.second.clear();
    }
    parser->headers_parsing_complete_ = true;

    // Force the parser to stop after the headers are parsed so clients
    // can process the request (or response).  This is to properly
    // handle HTTP/1.1 pipelined requests.
    http_parser_pause(httpparser, 1);

    return 0;
}

//===============================================================================
int Parser::on_body(http_parser* httpparser, const char* data, size_t size)
{
    Parser* parser = static_cast<Parser*>(httpparser->data);
    parser->message_->body().append(data, size);
    return 0;
}

//===============================================================================
int Parser::on_message_complete(http_parser* httpparser)
{
    Parser* parser = static_cast<Parser*>(httpparser->data);
    parser->message_->flags(httpparser->flags);
    parser->parsing_complete_ = true;

    // Force the parser to stop after the request is parsed so clients
    // can process the request (or response).  This is to properly
    // handle HTTP/1.1 pipelined requests.
    http_parser_pause(httpparser, 1);

    return 0;
}

}
