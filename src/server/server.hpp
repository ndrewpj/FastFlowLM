/*!
 *  Copyright (c) 2023 by Contributors
 * \file server.hpp
 * \brief WebServer class and related declarations
 * \author FastFlowLM Team
 * \date 2025-06-24
 * \version 0.1.0
 */
#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <memory>
#include <functional>
#include <unordered_map>
#include <map>
#include "wstream_buf.hpp"
#include "streaming_ostream.hpp"
#include "model_downloader.hpp"

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;
using json = nlohmann::json;

// Forward declaration
class RestHandler;
class HttpSession;

// Stream response callback type for handling streaming responses
using StreamCallback = std::function<void(const std::string&)>;

// Request handler callback type
using RequestHandler = std::function<void(
    const http::request<http::string_body>& req,
    std::function<void(const json&)> send_response,
    std::function<void(const json&, bool)> send_streaming_response,  // data, is_final
    std::shared_ptr<HttpSession> session  // for streaming support
)>;

///@brief WebServer class
///@param port the port to listen on, default is 11434, same with the ollama server
class WebServer {
public:
    ///@brief constructor
    ///@param port the port to listen on, default is 11434, same with the ollama server
    WebServer(int port = 11434);
    ///@brief destructor
    ~WebServer();

    ///@brief start the server
    void start();
    ///@brief stop the server
    void stop();

    ///@brief register route handlers
    ///@param method the method of the request
    ///@param path the path of the request
    ///@param handler the handler of the request
    void register_handler(const std::string& method, const std::string& path, RequestHandler handler);

    ///@brief handle HTTP request (called by session)
    ///@param req the request
    ///@param res the response
    ///@param socket the socket
    ///@param session the session
    void handle_request(http::request<http::string_body>& req,
                       http::response<http::string_body>& res,
                       tcp::socket& socket,
                       std::shared_ptr<HttpSession> session);

private:
    ///@brief do accept
    void do_accept();
    
    ///@brief io context
    net::io_context ioc;
    ///@brief acceptor
    tcp::acceptor acceptor;
    ///@brief routes
    std::map<std::string, RequestHandler> routes;
    ///@brief running
    bool running;
    ///@brief port
    int port;
};

///@brief HttpSession class for handling individual connections
class HttpSession : public std::enable_shared_from_this<HttpSession> {
public:
    ///@brief constructor
    ///@param socket the socket
    ///@param server the server
    HttpSession(tcp::socket socket, WebServer& server);
    ///@brief start
    void start();
    ///@brief write streaming response
    ///@param data the data
    ///@param is_final the is final
    void write_streaming_response(const json& data, bool is_final);

private:
    ///@brief Read the request
    void read_request();
    ///@brief Handle the request
    void handle_request();
    ///@brief Write the response
    void write_response();
    ///@brief Send the chunk data
    ///@param data the data
    ///@param is_final the is final
    void send_chunk_data(const json& data, bool is_final);
    
    ///@brief socket
    tcp::socket socket_;
    ///@brief buffer
    beast::flat_buffer buffer_;
    ///@brief request
    http::request<http::string_body> req_;
    ///@brief response
    http::response<http::string_body> res_;
    ///@brief server
    WebServer& server_;
    ///@brief is streaming
    bool is_streaming_;
    ///@brief stream buffer
    std::shared_ptr<streaming_buf> stream_buf_;
};

// Forward declarations
class model_list;

///@brief create lm server
///@param models the model list
///@param downloader the downloader
///@param default_tag the default tag
///@param port the port to listen on, default is 11434, same with the ollama server
///@return the server
std::unique_ptr<WebServer> create_lm_server(model_list& models, ModelDownloader& downloader, const std::string& default_tag, int port = 11434);

