/*!
 *  Copyright (c) 2023 by Contributors
 * \file server.cpp
 * \brief WebServer class and related declarations
 * \author FastFlowLM Team
 * \date 2025-06-24
 * \version 1.0.0
 */
#include "server.hpp"
#include "rest_handler.hpp"
#include <sstream>
#include <thread>
#include <iostream>

///@brief HttpSession class implementation
///@param socket the socket
///@param server the server
HttpSession::HttpSession(tcp::socket socket, WebServer& server)
    : socket_(std::move(socket))
    , server_(server)
    , is_streaming_(false) {}

///@brief start
void HttpSession::start() {
    read_request();
}

///@brief read request
void HttpSession::read_request() {
    auto self = shared_from_this();

    http::async_read(socket_, buffer_, req_,
        [self](beast::error_code ec, std::size_t) {
            if (!ec) {
                self->handle_request();
            }
        });
}

///@brief handle request
void HttpSession::handle_request() {
    // Reset response
    res_ = {};
    res_.version(req_.version());
    res_.keep_alive(req_.keep_alive());
    
    // Handle the request through the server
    server_.handle_request(req_, res_, socket_, shared_from_this());
    
    if (!is_streaming_) {
        write_response();
    }
}

///@brief write response
void HttpSession::write_response() {
    auto self = shared_from_this();


    std::cout << "\n=== Outgoing Response ===" << std::endl;
    std::cout << "Response: " << res_.body() << std::endl;
    std::cout << "=====================\n" << std::endl;

    http::async_write(socket_, res_,
        [self](beast::error_code ec, std::size_t) {
            if (!self->req_.keep_alive()) {
                self->socket_.shutdown(tcp::socket::shutdown_both, ec);
            }
        });
}

///@brief write streaming response
///@param data the data
///@param is_final the is final
void HttpSession::write_streaming_response(const json& data, bool is_final) {
    if (!is_streaming_) {
        // Initialize streaming response headers
        is_streaming_ = true;
        res_.result(http::status::ok);
        res_.set(http::field::content_type, "application/x-ndjson");
        res_.set(http::field::cache_control, "no-cache");
        res_.set(http::field::connection, "keep-alive");
        res_.set(http::field::transfer_encoding, "chunked");
        
        // Send headers immediately using raw socket write
        std::string headers = "HTTP/1.1 200 OK\r\n";
        headers += "Content-Type: application/x-ndjson\r\n";
        headers += "Cache-Control: no-cache\r\n";
        headers += "Connection: keep-alive\r\n";
        headers += "Transfer-Encoding: chunked\r\n";
        headers += "\r\n";
        
        // Send headers synchronously
        boost::system::error_code ec;
        net::write(socket_, net::buffer(headers), ec);
        if (ec) return;
    }
    
    // Send this chunk immediately
    send_chunk_data(data, is_final);
}

///@brief send chunk data
///@param data the data
///@param is_final the is final
void HttpSession::send_chunk_data(const json& data, bool is_final) {
    // Format as HTTP chunk
    std::string chunk_content = data.dump() + "\n";
    
    std::cout << "\n=== Outgoing Chunk ===" << std::endl;
    std::cout << "Chunk: " << chunk_content << std::endl;
    std::cout << "=====================\n" << std::endl;
    // HTTP chunked format: size in hex + \r\n + data + \r\n
    std::ostringstream chunk_size;
    chunk_size << std::hex << chunk_content.length();
    
    std::string http_chunk = chunk_size.str() + "\r\n" + chunk_content + "\r\n";
    
    // Send chunk immediately
    boost::system::error_code ec;
    net::write(socket_, net::buffer(http_chunk), ec);
    
    if (is_final) {
        // Send final chunk (0-length chunk to end stream)
        std::string final_chunk = "0\r\n\r\n";
        net::write(socket_, net::buffer(final_chunk), ec);
        
        if (!req_.keep_alive()) {
            socket_.shutdown(tcp::socket::shutdown_both, ec);
        }
    }
}

///@brief WebServer implementation
///@param port the port
WebServer::WebServer(int port) : acceptor(ioc, {net::ip::make_address("0.0.0.0"), static_cast<unsigned short>(port)}), running(false), port(port) {}

///@brief destructor
WebServer::~WebServer() {
    stop();
}

///@brief start
void WebServer::start() {
    running = true;
    do_accept();
    
    // Run the I/O service on a separate thread
    std::thread([this]() {
        try {
            ioc.run();
        } catch (const std::exception& e) {
            std::cerr << "Error in WebServer: " << e.what() << std::endl;
        }
    }).detach();
    
    std::cout << "WebServer started on port " << port << std::endl;
}

///@brief stop
void WebServer::stop() {
    running = false;
    ioc.stop();
}

///@brief register handler
///@param method the method
///@param path the path
///@param handler the handler
void WebServer::register_handler(const std::string& method, const std::string& path, RequestHandler handler) {
    std::string key = method + " " + path;
    routes[key] = handler;
}

///@brief do accept
void WebServer::do_accept() {
    acceptor.async_accept(
        [this](beast::error_code ec, tcp::socket socket) {
            if (!ec) {
                // Create a new session for this connection
                std::make_shared<HttpSession>(std::move(socket), *this)->start();
            }
            if (running) {
                do_accept();
            }
        });
}

///@brief handle request
///@param req the request
///@param res the response
///@param socket the socket
///@param session the session
void WebServer::handle_request(http::request<http::string_body>& req,
                              http::response<http::string_body>& res,
                              tcp::socket& socket,
                              std::shared_ptr<HttpSession> session) {
    // Log request details
    std::cout << "\n=== Incoming Request ===" << std::endl;
    std::cout << "Method: " << req.method_string() << std::endl;
    std::cout << "Target: " << req.target() << std::endl;
    std::cout << "Version: " << req.version() << std::endl;
    json request_json;
    try {
        if (!req.body().empty()) {
            request_json = json::parse(req.body());
        }
    } catch (const std::exception& e) {
        std::cout << "Error parsing request body: " << e.what() << std::endl;
    }
    std::cout << "Body: " << std::endl;
    std::cout << request_json.dump(4) << std::endl;
    std::cout << "=====================\n" << std::endl;

    std::string key = std::string(req.method_string()) + " " + std::string(req.target());
    
    auto it = routes.find(key);
    if (it != routes.end()) {
        // Parse JSON request body
        json request_json;
        try {
            if (!req.body().empty()) {
                request_json = json::parse(req.body());
            }
        } catch (const std::exception& e) {
            res.result(http::status::bad_request);
            res.body() = json{{"error", "Invalid JSON"}}.dump();
            res.set(http::field::content_type, "application/json");
            res.prepare_payload();
            return;
        }
        
        // Create response callback
        auto send_response = [&res](const json& response_data) {
            res.result(http::status::ok);
            res.body() = response_data.dump();
            res.set(http::field::content_type, "application/json");
            res.prepare_payload();
        };
        
        // Create streaming response callback that uses the session
        auto send_streaming_response = [session](const json& data, bool is_final) {
            if (session) {
                session->write_streaming_response(data, is_final);
            }
        };
        
        // Call the handler with the session
        it->second(req, send_response, send_streaming_response, session);
    } else {
        res.result(http::status::not_found);
        res.body() = json{{"error", "Not Found"}}.dump();
        res.set(http::field::content_type, "application/json");
    }
    
    res.prepare_payload();
}

///@brief create lm server
///@param models the model list
///@param downloader the downloader
///@param default_tag the default tag
///@param port the port
///@return the server
std::unique_ptr<WebServer> create_lm_server(model_list& models, ModelDownloader& downloader, const std::string& default_tag, int port) {
    auto server = std::make_unique<WebServer>(port);
    auto rest_handler = std::make_shared<RestHandler>(models, downloader, default_tag);
    
    // Register Ollama-compatible routes
    server->register_handler("POST", "/api/generate", 
        [rest_handler](const http::request<http::string_body>& req, 
                      std::function<void(const json&)> send_response,
                      std::function<void(const json&, bool)> send_streaming_response,
                      std::shared_ptr<HttpSession> session) {
            json request_json;
            if (!req.body().empty()) {
                request_json = json::parse(req.body());
            }
            rest_handler->handle_generate(request_json, send_response, send_streaming_response);
        });
    
    server->register_handler("POST", "/api/chat",
        [rest_handler](const http::request<http::string_body>& req,
                      std::function<void(const json&)> send_response,
                      std::function<void(const json&, bool)> send_streaming_response,
                      std::shared_ptr<HttpSession> session) {
            json request_json;
            if (!req.body().empty()) {
                request_json = json::parse(req.body());
            }
            rest_handler->handle_chat(request_json, send_response, send_streaming_response);
        });

    server->register_handler("GET", "/api/ps",
        [rest_handler](const http::request<http::string_body>& req,
                      std::function<void(const json&)> send_response,
                      std::function<void(const json&, bool)> send_streaming_response,
                      std::shared_ptr<HttpSession> session) {
            json request_json;
            rest_handler->handle_ps(request_json, send_response, send_streaming_response);
        });

    server->register_handler("POST", "/api/embeddings",
        [rest_handler](const http::request<http::string_body>& req,
                      std::function<void(const json&)> send_response,
                      std::function<void(const json&, bool)> send_streaming_response,
                      std::shared_ptr<HttpSession> session) {
            json request_json;
            if (!req.body().empty()) {
                request_json = json::parse(req.body());
            }
            rest_handler->handle_embeddings(request_json, send_response, send_streaming_response);
        });
    
    server->register_handler("GET", "/api/tags",
        [rest_handler](const http::request<http::string_body>& req,
                      std::function<void(const json&)> send_response,
                      std::function<void(const json&, bool)> send_streaming_response,
                      std::shared_ptr<HttpSession> session) {
            json request_json;
            rest_handler->handle_models(request_json, send_response, send_streaming_response);
        });
    
    server->register_handler("GET", "/api/version",
        [rest_handler](const http::request<http::string_body>& req,
                      std::function<void(const json&)> send_response,
                      std::function<void(const json&, bool)> send_streaming_response,
                      std::shared_ptr<HttpSession> session) {
            json request_json;
            rest_handler->handle_version(request_json, send_response, send_streaming_response);
        });
    
    // Add other endpoints...
    server->register_handler("POST", "/api/pull",
        [rest_handler](const http::request<http::string_body>& req,
                      std::function<void(const json&)> send_response,
                      std::function<void(const json&, bool)> send_streaming_response,
                      std::shared_ptr<HttpSession> session) {
            json request_json;
            if (!req.body().empty()) {
                request_json = json::parse(req.body());
            }
            rest_handler->handle_pull(request_json, send_response, send_streaming_response);
        });
    
    return server;
}