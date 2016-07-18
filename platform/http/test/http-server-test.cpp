
#include <iostream>

#include <boost/asio/signal_set.hpp>
#include <boost/algorithm/string.hpp>

#define RAPIDJSON_ASSERT(x) if(!(x)) throw std::runtime_error("JSON assertion failed: " #x);

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include <http/server.hpp>
#include <http/router.hpp>


static const std::vector<std::string> angular_js_tests = {
std::string(R"raw(
<!DOCTYPE html>
<html ng-app>
    <head>
        <meta charset="utf-8" />
        <title>Hello World</title>
        <!--[if IE]>
            <script src="http://html5shiv.googlecode.com/svn/trunk/html5.js"></script>
        <![endif]-->
        <link href="css/default.css" rel="stylesheet" type="text/css" />
    </head>
    <body>
        <header>
            <h1 ng-controller="HelloWorldCtrl">{{helloMessage}}</h1>
        </header>
        <section>
        </section>
        <footer>
        </footer>
        <script src="https://ajax.googleapis.com/ajax/libs/angularjs/1.0.8/angular.min.js"></script>

        <script type="text/javascript">
            function HelloWorldCtrl($scope) {
                $scope.helloMessage = "Hello World";
            }
        </script>

        Write some text in textbox:
        <input ng-model="sometext" type="text">
        <h1>Hello {{ sometext }}</h1>
    </body>
</html>
)raw"),

std::string(R"raw(
<!doctype html>
<html ng-app>
<head>
<script src="https://ajax.googleapis.com/ajax/libs/angularjs/1.3.9/angular.min.js"></script>
<link rel="stylesheet" href="http://cdn.leafletjs.com/leaflet-0.7.3/leaflet.css" />
</head>
<body>
<div>
<label>Name:</label>
<input type="text" ng-model="yourName" placeholder="Enter a name here">
<hr>
<h1>Hello {{yourName}}!</h1>
</div>

<script src="http://cdn.leafletjs.com/leaflet-0.7.3/leaflet.js"></script>
<div id="map" style="width: 600px; height: 400px"></div>
<script>

var map = L.map('map').setView([51.505, -0.09], 13);

L.tileLayer('https://{s}.tiles.mapbox.com/v3/{id}/{z}/{x}/{y}.png', {
maxZoom: 18,
attribution: 'Map data &copy; <a href="http://openstreetmap.org">OpenStreetMap</a> contributors, ' +
'<a href="http://creativecommons.org/licenses/by-sa/2.0/">CC-BY-SA</a>, ' +
'Imagery © <a href="http://mapbox.com">Mapbox</a>',
id: 'examples.map-i875mjb7'
}).addTo(map);


L.marker([51.5, -0.09]).addTo(map)
.bindPopup("<b>Hello world!</b><br />I am a popup.").openPopup();

L.circle([51.508, -0.11], 500, {
color: 'red',
fillColor: '#f03',
fillOpacity: 0.5
}).addTo(map).bindPopup("I am a circle.");

L.polygon([
[51.509, -0.08],
[51.503, -0.06],
[51.51, -0.047]
]).addTo(map).bindPopup("I am a polygon.");


var popup = L.popup();

function onMapClick(e) {
popup
.setLatLng(e.latlng)
.setContent("You clicked the map at " + e.latlng.toString())
.openOn(map);
}

map.on('click', onMapClick);

</script>
</body>
</html>
)raw")

};


//==============================================================================
http::Response http_handler_angular_js_test(http::Router::URLPathParameters& url_parameters, http::Connection& conn, http::Request& req, boost::asio::yield_context& yield)
{
    http::Response response;
    response.status_code(http::StatusCode::CODE_200_OK);
    response.body(angular_js_tests[std::stoi(url_parameters["id"])]);
    response.headers()["Content-Type"] = "text/html";

    return response;
}

//==============================================================================
http::Response http_handler_json_test(http::Router::URLPathParameters& url_parameters, http::Connection& conn, http::Request& req, boost::asio::yield_context& yield)
{
    http::Response response;
    response.status_code(http::StatusCode::CODE_200_OK);
    response.headers()["Content-Type"] = "text/json";

    rapidjson::Document d;
    d.Parse(req.body().c_str());

    d.AddMember("name", "json-test", d.GetAllocator());
    d.AddMember("id", std::stoi(url_parameters["id"]), d.GetAllocator());
    d.AddMember("http-method", static_cast<int>(req.method()), d.GetAllocator());

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    d.Accept(writer);

    response.body() += buffer.GetString();

    return response;
}

//==============================================================================
http::Response http_handler_hello_world(http::Router::URLPathParameters& url_parameters, http::Connection& conn, http::Request& req, boost::asio::yield_context& yield)
{
    http::Response response;
    response.status_code(http::StatusCode::CODE_200_OK);
    response.body(std::string("Hello World\n"));

    return response;
}

//==============================================================================
http::Response http_handler(http::Router::URLPathParameters& url_parameters, http::Connection& conn, http::Request& req, boost::asio::yield_context& yield)
{
    http::Response response;
    response.status_code(http::StatusCode::CODE_200_OK);
    response.headers()["Content-Type"] = "text/plain";

    time_t rawtime = time(nullptr);
    response.body(std::string("Hello World\n") + req.remote_endpoint().address + " " + std::to_string(req.remote_endpoint().port) + "\nTime: " + asctime(localtime(&rawtime)));
    response.body() += std::string("URL path: ") + req.url().path() + "\n";
    response.body() += std::string("URL query: ") + req.url().query() + "\n";
    response.body() += std::string("Method: ") + (req.method() == http::Method::GET ? "GET" : "not GET") + "\n";
    response.body() += std::string("Connection Id: ") + std::to_string(conn.get_id()) + "\n";

    std::vector<std::string> split_res;
    boost::algorithm::split(split_res, req.url().path(), boost::algorithm::is_any_of("/"), boost::algorithm::token_compress_off);

    response.body() += "SPLIT RESULT:\n";
    for(size_t i = 0; i < split_res.size(); i++)
    {
        response.body() += ("[" + std::to_string(i) + "]: " + split_res[i] + "\n");
    }

    response.body() += "\nURL PARAMETERS BEGIN:\n";
    for(auto& elt : url_parameters)
    {
        response.body() += ("[" + elt.first + "]: " + elt.second + "\n");
    }
    response.body() += "URL PARAMETERS END\n";

    return response;
}

class RequestHandler : public http::IRequestHandler
{
    public:
        RequestHandler(const http::Router&& router)
            : router_(router)
        {
        	//pass
        }

        virtual void reinit(boost::asio::yield_context& yield)
        {
            //pass
        }

        virtual ~RequestHandler()
        {
            //pass
        }

        virtual http::Response process_request(http::Connection& conn, http::Request& req, boost::asio::yield_context& yield)
        {
            return router_.route_request(conn, req, yield);
        }

    private:
        http::Router router_;
};

class RequestHandlerFactory : public http::IRequestHandlerFactory
{
    public:
        virtual http::IRequestHandler* create_handler(boost::asio::yield_context& yield)
        {
            http::Router router;
            router.set("/hello/world", http::Method::GET, http_handler_hello_world);
            router.set("/hello/world/:id", http::Method::GET, http_handler);
            router.set("/hello/world/:id/action_one", http::Method::GET, http_handler);
            router.set("/hello/world/:id/action_two", http::Method::GET, http_handler);
            router.set("/hello/world/:id/action_one/:param", http::Method::GET, http_handler);
            router.set("/hello/world/:id/:action_name", http::Method::GET, http_handler);

            router.set("/angular/js/test/:id", http::Method::GET, http_handler_angular_js_test);

            router.set("/api/test/:id", http::Method::GET, http_handler_json_test);
            router.set("/api/test/:id", http::Method::POST, http_handler_json_test);
            router.set("/api/test/:id", http::Method::PUT, http_handler_json_test);
            router.set("/api/test/:id", http::Method::DELETE, http_handler_json_test);

            return new RequestHandler(std::move(router));
        }

        virtual ~RequestHandlerFactory()
        {
            //pass
        }
};

int main(int argc, char **argv)
{
    RequestHandlerFactory rh_factory;

    http::ServerSettings settings;
    settings.endpoint = boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 1502);
    settings.num_threads = 0;
    settings.sheduled_connection_delete_period_msec = 10000;
    settings.connection_settings.buffer_size_bytes = 32768;
    settings.connection_settings.timeout_msec = 10000;
    settings.connection_settings.request_handler_factory = &rh_factory;

    http::Server server(settings);

    boost::asio::spawn(server.get_io_service(), [&](boost::asio::yield_context yield)
    {
        boost::asio::signal_set signals(server.get_io_service(), SIGINT, SIGTERM);
        std::cout << "Created signal_set" << std::endl;

        signals.async_wait(yield);
        std::cout << "Received signal..." << std::endl;
        server.stop();
        std::cout << "Service is shutting down... please wait..." << std::endl;
    });

    std::cout << "Starting server" << std::endl;
    server.start();
    std::cout << "Server stopped" << std::endl;
}

