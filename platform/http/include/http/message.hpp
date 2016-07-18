
#ifndef HTTP_MESSAGE_HPP_
#define HTTP_MESSAGE_HPP_

#include <cstdint>
#include <unordered_map>
#include <string>
#include <http/constants.hpp>

namespace http
{

class Message
{
    public:
        enum class Type
        {
            BASE_MESSAGE,
            REQUEST,
            RESPONSE
        };

        class Flags
        {
            public:
                Flags();
                Flags(uint8_t flags);
                Flags(const Flags& flags);
                Flags(Flags&& flags);
                Flags& operator=(const Flags& flags);
                Flags& operator=(Flags&& flags);
                ~Flags();

                bool chunked();
                void chunked(const bool& val);

                bool connection_keep_alive();
                void connection_keep_alive(const bool& val);

                bool connection_close();
                void connection_close(const bool& val);

                bool trailing();
                void trailing(const bool& val);

                bool upgrade();
                void upgrade(const bool& val);

                bool skip_body();
                void skip_body(const bool& val);

            private:
                uint8_t flags_;
        };

        typedef std::unordered_map<std::string, std::string> HeadersMap;

        Message();
        Message(const Message& msg);
        Message(Message&& msg);
        Message& operator=(const Message& msg);
        Message& operator=(Message&& msg);
        virtual ~Message();

        HeadersMap& headers();
        void headers(const HeadersMap& val);

        std::string& body();
        void body(const std::string& val);

        Flags& flags();
        void flags(const Flags& val);

        Type type();

        virtual void clear();

    protected:
        Type type_;

    private:
        HeadersMap headers_;
        std::string body_;
        Flags flags_;
};

}

#endif //HTTP_MESSAGE_HPP_
