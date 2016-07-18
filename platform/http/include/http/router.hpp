
#ifndef HTTP_ROUTER_HPP_
#define HTTP_ROUTER_HPP_

#include <unordered_map>
#include <string>
#include <tuple>
#include <functional>

#include <boost/asio/spawn.hpp>

#include <http/message.hpp>
#include <http/constants.hpp>
#include <http/url.hpp>
#include <http/connection.hpp>


namespace http
{

class Router
{
    public:
        using URLPathParameters = std::unordered_map<std::string, std::string>;
        using Handler = std::function<Response(URLPathParameters& parameters, Connection&, Request&, boost::asio::yield_context&)>;
        using RoutingResult = std::tuple<Handler, URLPathParameters>;

    private:
        class Node
        {
            using Children_t = std::unordered_map<std::string, Node*>;
            using Handlers_t = std::unordered_map<size_t, Handler>;  // key is http::Method (enum class) value

            public:
            Node();
            Node(std::string name);
            Node(const Node& node);
            Node(Node&& node);
            Node& operator=(const Node& node);
            Node& operator=(Node&& node);
            virtual ~Node();

            std::string& name();
            Children_t& children();
            Handlers_t& handlers();

            private:
            std::string name_;
            Children_t children_;
            Handlers_t handlers_;
        };

    public:
        Router();
        Router(const Router& router);
        Router(Router&& router);
        Router& operator=(const Router& router);
        Router& operator=(Router&& router);
        virtual ~Router();

        Response route_request(Connection&, Request&, boost::asio::yield_context&);

        void set(const std::string& url_path_rule, const http::Method& method, const Handler& handler);
        RoutingResult get(const std::string& url_path, const http::Method& method);

    protected:
        std::vector<std::string> split_url_path(const std::string url_path);

    private:
        Node root_;
};

}

#endif //HTTP_ROUTER_HPP_
