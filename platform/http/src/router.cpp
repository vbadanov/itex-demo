
#include <stdexcept>
#include <iostream>
#include <boost/algorithm/string.hpp>
#include <http/router.hpp>
#include <http/exception.hpp>


namespace http
{

//===============================================================================
Router::Node::Node()
{
    //pass
}

//===============================================================================
Router::Node::Node(std::string name)
    : name_(name)
{
    //pass
}

//===============================================================================
Router::Node::Node(const Router::Node& node)
{
    *this = node;
}

//===============================================================================
Router::Node::Node(Router::Node&& node)
{
    *this = node;
}

//===============================================================================
Router::Node& Router::Node::operator=(const Router::Node& node)
{
    name_ = node.name_;
    for(auto& elt : node.children_)
    {
        children_.emplace(elt.first, new Node(*(elt.second)));
    }
    handlers_ = node.handlers_;
    return *this;
}

//===============================================================================
Router::Node& Router::Node::operator=(Router::Node&& node)
{
    name_ = std::move(node.name_);
    children_ = std::move(node.children_);
    handlers_ = std::move(node.handlers_);
    return *this;
}

//===============================================================================
Router::Node::~Node()
{
    name_.clear();

    for(auto& elt : children_)
    {
        if(elt.second != nullptr)
        {
            delete elt.second;
        }
    }

    children_.clear();
    handlers_.clear();
}

//===============================================================================
std::string& Router::Node::name()
{
    return name_;
}

//===============================================================================
Router::Node::Children_t& Router::Node::children()
{
    return children_;
}

//===============================================================================
Router::Node::Handlers_t& Router::Node::handlers()
{
    return handlers_;
}

//===============================================================================
Router::Router()
{
    //pass
}

//===============================================================================
Router::Router(const Router& router)
{
    *this = router;
}

//===============================================================================
Router::Router(Router&& router)
{
    *this = router;
}

//===============================================================================
Router& Router::operator=(const Router& router)
{
    root_ = router.root_;
    return *this;
}

//===============================================================================
Router& Router::operator=(Router&& router)
{
    root_ = std::move(router.root_);
    return *this;
}

//===============================================================================
Router::~Router()
{
    //pass
}

//===============================================================================
Response Router::route_request(http::Connection& conn, http::Request& req, boost::asio::yield_context& yield)
{
    Response res;
    try
    {
        auto handler_info = this->get(req.url().path(), req.method());
        res = std::get<0>(handler_info)(std::get<1>(handler_info), conn, req, yield);
    }
    catch (RouterException& e)
    {
        res.status_code(StatusCode::CODE_404_NOT_FOUND);
        res.body(e.what());
    }

    return res;
}

//===============================================================================
void Router::set(const std::string& url_path_rule, const Method& method, const Router::Handler& handler)
{
    auto url_tokens = split_url_path(url_path_rule);
    Node* node = &root_;
    for(auto& token : url_tokens)
    {
        if(token.empty())
        {
            continue;
        }

        // If token is param name (e.g. ":param_name", then Node has internal name ":param_name" and is added to children map with key ":")
        std::string child_name(token[0] != ':' ? token : ":");
        if(node->children().count(child_name) == 0)
        {
            node->children().emplace(child_name, new Node(token[0] != ':' ? token : token.substr(1)));
        }
        node = node->children()[child_name];
    }
    node->handlers()[static_cast<size_t>(method)] = handler;
}

//===============================================================================
Router::RoutingResult Router::get(const std::string& url_path, const Method& method)
{
    try
    {
        auto url_tokens = split_url_path(url_path);
        Node* node = &root_;
        URLPathParameters params;
        for(auto& token : url_tokens)
        {
            if(token.empty())
            {
                continue;
            }

            std::string child_name = token;
            if(node->children().count(token) == 0)
            {
                // this token is treated as parameter
                child_name = ":";
            }
            node = node->children().at(child_name);

            if(child_name == ":")
            {
                params[node->name()] = token;  // store parameter value
            }
        }
        return std::make_tuple(node->handlers().at(static_cast<size_t>(method)), params);
    }
    catch(std::out_of_range& e)
    {
        throw RouterException(std::string("Could not find route for URL [") + url_path + "] by HTTP method [" + std::to_string(static_cast<size_t>(method)) + "]");
    }
}

//===============================================================================
std::vector<std::string> Router::split_url_path(const std::string url_path)
{
    std::vector<std::string> url_tokens;
    boost::algorithm::split(url_tokens, url_path, boost::algorithm::is_any_of("/"), boost::algorithm::token_compress_on);
    return url_tokens;
}


}
