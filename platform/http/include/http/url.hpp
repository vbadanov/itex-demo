
#ifndef HTTP_URL_HPP_
#define HTTP_URL_HPP_

#include <string>
#include <cstdint>

namespace http
{
    class Url
    {
        public:
            Url();
            Url(std::string full_url_str);
            Url(const Url& url);
            Url(Url&& url);
            Url& operator=(const Url& url);
            Url& operator=(Url&& url);
            ~Url();

            std::string& full();
            std::string& schema();
            std::string& host();
            uint16_t& port();
            std::string& path();
            std::string& query();
            std::string& fragment();
            std::string& userinfo();

        private:
            std::string full_;
            std::string schema_;
            std::string host_;
            uint16_t port_;
            std::string path_;
            std::string query_;
            std::string fragment_;
            std::string userinfo_;
    };


}

#endif //HTTP_URL_HPP_
