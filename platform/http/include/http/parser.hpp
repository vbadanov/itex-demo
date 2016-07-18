
#ifndef HTTP_PARSER_HPP_
#define HTTP_PARSER_HPP_

#include <stdexcept>
#include <string>
#include <utility>

#include <http_parser.h>
#include <http/message.hpp>

namespace http
{

//===============================================================================
class Parser
{
    public:
        Parser() = delete;
        Parser(Message* msg);
        Parser(const Parser& parser);
        Parser(Parser&& parser);
        Parser& operator=(const Parser& parser);
        Parser& operator=(Parser&& parser);
        ~Parser();

        size_t feed(const char* data, size_t size);
        void clear();

        bool complete();
        bool headers_complete();

        uint32_t version();

    protected:
        static int on_message_begin(http_parser* httpparser);
        static int on_url(http_parser* httpparser, const char* data, size_t size);
        static int on_status(http_parser* httpparser, const char* data, size_t size);
        static int on_header_field(http_parser* httpparser, const char* data, size_t size);
        static int on_header_value(http_parser* httpparser, const char* data, size_t size);
        static int on_headers_complete(http_parser* httpparser);
        static int on_body(http_parser* httpparser, const char* data, size_t size);
        static int on_message_complete(http_parser* httpparser);

        size_t feed_step(const char* data, size_t size);

    private:
        Message* message_;
        http_parser_settings *settings_;
        http_parser *parser_;

        std::pair<std::string, std::string> current_header_; // header field, value

        bool parsing_complete_;
        bool headers_parsing_complete_;
};


}

#endif //HTTP_PARSER_HPP_
