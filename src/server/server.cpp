/*!
 *  Copyright (c) 2023 by Contributors
 * \file server.cpp
 * \brief WebServer class and related declarations
 * \author FastFlowLM Team
 * \date 2025-06-24
 * \version 0.1.0
 */
#include "server.hpp"
#include "rest_handler.hpp"
#include <sstream>
#include <thread>
#include <iostream>


// Global NPU access control
std::mutex g_npu_access_mutex;
std::atomic<bool> g_npu_in_use{false};

std::atomic<int> g_npu_active_requests{0};

///@brief get current time string, format: hh:mm:ss mm:dd:yyyy
///@return the current time string
std::string get_current_time_string() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%H:%M:%S %m:%d:%Y");
    return ss.str();
}

///@brief brief print request
///@param request the request
void brief_print_message_request(nlohmann::json request) {
    // if request has "messages" or "message" field, the context in meassages shall be printed briefly
    if (request.contains("messages")) {
        for (auto& message : request["messages"]) {
            if (message.contains("content")) {
                std::string content = message["content"].get<std::string>();
                if (content.size() > 20) {
                    message["content"] = content.substr(0, 10) + "..." + content.substr(content.size() - 10);
                }
            }
        }
    }
    if (request.contains("message")){
        std::string content = request["message"]["content"].get<std::string>();
        if (content.size() > 20) {
            request["message"]["content"] = content.substr(0, 10) + "..." + content.substr(content.size() - 10);
        }
    }
    header_print("LOG", "Body: ");
    std::cout << request.dump(4) << std::endl;

}
// NPU Access Manager implementation
bool NPUAccessManager::try_acquire_npu_access() {
    std::lock_guard<std::mutex> lock(g_npu_access_mutex);
    if (g_npu_in_use.load()) {
        return false; // NPU is already in use
    }
    g_npu_in_use.store(true);
    g_npu_active_requests.fetch_add(1);
    return true;
}

void NPUAccessManager::release_npu_access() {
    std::lock_guard<std::mutex> lock(g_npu_access_mutex);
    g_npu_in_use.store(false);
    g_npu_active_requests.fetch_sub(1);
}

bool NPUAccessManager::is_npu_available() {
    return !g_npu_in_use.load();
}

int NPUAccessManager::get_active_npu_requests() {
    return g_npu_active_requests.load();
}

// Helper function to check if an endpoint requires NPU access
bool requires_npu_access(const std::string& method, const std::string& path) {
    // NPU-intensive endpoints that should be restricted to one user at a time
    if (method == "POST") {
        return path == "/api/generate" || 
               path == "/api/chat" || 
               path == "/v1/chat/completions";
    }
    return false;
}

///@brief HttpSession class implementation
///@param socket the socket
///@param server the server
HttpSession::HttpSession(tcp::socket socket, WebServer& server)
    : socket_(std::move(socket))
    , server_(server)
    , is_streaming_(false) {
    // Set socket timeout
    socket_.set_option(tcp::socket::keep_alive(false));
    socket_.set_option(tcp::socket::linger(true, 0));
    
    // Debug: Log TCP connection formation
    header_print("🔗 ", "TCP connection established - Remote: " + socket_.remote_endpoint().address().to_string() + ":" + std::to_string(socket_.remote_endpoint().port()));
}

///@brief start
void HttpSession::start() {
    read_request();
}

///@brief close connection
void HttpSession::close_connection() {
    boost::system::error_code ec;
    
    // Debug: Log TCP connection disconnection
    try {
        header_print("🔒 ", "TCP connection closing - Remote: " + socket_.remote_endpoint().address().to_string() + ":" + std::to_string(socket_.remote_endpoint().port()));
    } catch (const std::exception& e) {
        header_print("🔒 ", "TCP connection closing - Remote endpoint unavailable");
    }
    socket_.shutdown(tcp::socket::shutdown_both, ec);
    server_.active_connections_.fetch_sub(1);
}

///@brief read request
void HttpSession::read_request() {
    auto self = shared_from_this();

    http::async_read(socket_, buffer_, req_,
        [self](beast::error_code ec, std::size_t bytes_transferred) {
            if (!ec) {
                // Log the bytes read to debug
                header_print("TCP", "Read " + std::to_string(bytes_transferred) + " bytes from socket");
                
                self->handle_request();
            } else {
                // Connection closed or error, decrement connection counter
                header_print("🔒 ", "TCP connection closed - Remote: " + self->socket_.remote_endpoint().address().to_string() + ":" + std::to_string(self->socket_.remote_endpoint().port()));
                self->server_.active_connections_.fetch_sub(1);
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
    
    // Clear the buffer after processing to prevent data accumulation
    buffer_.consume(buffer_.size());
    
    if (!is_streaming_) {
        write_response();
    } else {
        // For streaming responses, we need to handle connection cleanup differently
        // The connection will be closed when streaming ends
    }
}

///@brief write response
void HttpSession::write_response() {
    auto self = shared_from_this();

    header_print("⬆️ ", "Outgoing Response: ");
    header_print("LOG", "Time stamp: " << get_current_time_string()); // hh:mm:ss mm:dd:yyyy
    try{
        nlohmann::json response_json = json::parse(res_.body());
        brief_print_message_request(response_json);
    } catch (const std::exception& e) {
        header_print("LOG", "Body: ");
        std::cout << res_.body() << std::endl;
    }

    std::cout << "================================================" << std::endl;

    http::async_write(socket_, res_,
        [self](beast::error_code ec, std::size_t) {
            if (!self->req_.keep_alive()) {
                header_print("🔒  ", "Closing TCP connection (non-keep-alive)");
                self->socket_.shutdown(tcp::socket::shutdown_both, ec);
                // Decrement connection counter for non-keep-alive connections
                self->server_.active_connections_.fetch_sub(1);
            } else {
                header_print("TCP", "Keeping TCP connection alive for next request");
                // Clear the request object before reading the next request
                self->req_ = {};
                // For keep-alive connections, read the next request
                self->read_request();
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
    std::string chunk_content;
    
    // Check if the data is a string (pre-formatted SSE with "data: " prefix)
    if (data.is_string()) {
        std::string data_str = data.get<std::string>();
        // If it starts with "data: ", it's already formatted for SSE
        if (data_str.substr(0, 6) == "data: ") {
            chunk_content = data_str;
        } else {
            // Regular JSON string, add single newline for Ollama format
            chunk_content = data_str;
        }
    } else {
        // Regular JSON object, format for Ollama compatibility
        chunk_content = data.dump() + "\n";
    }
    
    // std::cout << "Chunk: " << chunk_content;
    
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
            header_print("🔒 ", "Closing TCP connection (streaming, non-keep-alive)");
            socket_.shutdown(tcp::socket::shutdown_both, ec);
            // Decrement connection counter for non-keep-alive connections
            server_.active_connections_.fetch_sub(1);
        } else {
            header_print("🔗 ", "Keeping TCP connection alive for next request (streaming)");
            // Clear the buffer before reading the next request
            buffer_.consume(buffer_.size());
            // Clear the request object before reading the next request
            req_ = {};
            // For keep-alive connections, read the next request
            read_request();
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
    
    // Run the I/O service on multiple threads for better concurrency
    for (size_t i = 0; i < io_thread_count_; ++i) {
        io_threads_.emplace_back([this]() {
            try {
                ioc.run();
            } catch (const std::exception& e) {
                header_print("LOG", "Error in WebServer I/O thread: " + std::string(e.what()));
            }
        });
    }
    
    header_print("LOG", "WebServer started on port " + std::to_string(port) + " with " + std::to_string(io_thread_count_) + " I/O threads");
}

///@brief stop
void WebServer::stop() {
    running = false;
    ioc.stop();
    
    // Wait for all I/O threads to finish
    for (auto& thread : io_threads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    io_threads_.clear();
}

///@brief register active request
///@param request_id the request ID
///@param token the cancellation token
void WebServer::register_active_request(const std::string& request_id, std::shared_ptr<CancellationToken> token) {
    std::lock_guard<std::mutex> lock(active_requests_mutex_);
    active_requests_[request_id] = token;
}

///@brief unregister active request
///@param request_id the request ID
void WebServer::unregister_active_request(const std::string& request_id) {
    std::lock_guard<std::mutex> lock(active_requests_mutex_);
    active_requests_.erase(request_id);
}

///@brief cancel request
///@param request_id the request ID
///@return true if request was found and cancelled
bool WebServer::cancel_request(const std::string& request_id) {
    std::lock_guard<std::mutex> lock(active_requests_mutex_);
    auto it = active_requests_.find(request_id);
    if (it != active_requests_.end()) {
        it->second->cancel();
        active_requests_.erase(it);
        return true;
    }
    return false;
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
                // Check connection limit
                if (active_connections_.load() >= max_connections_) {
                    header_print("LOG", "Connection limit reached (" + std::to_string(max_connections_) + "), rejecting new connection");
                    // Close the socket and continue accepting
                    socket.close();
                } else {
                    // Increment connection counter
                    active_connections_.fetch_add(1);
                    
                    // Create a new session for this connection
                    auto session = std::make_shared<HttpSession>(std::move(socket), *this);
                    session->start();
                }
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
    std::cout << "================================================" << std::endl;
    header_print("⬇️ ", "Incoming Request: " << req.method_string());
    header_print("LOG", "Time stamp: " << get_current_time_string()); // hh:mm:ss mm:dd:yyyy
    header_print("LOG", "Target: " << req.target());
    header_print("LOG", "Version: " << req.version());
    header_print("LOG", "Keep-Alive: " << req.keep_alive());
    json request_json;
    try {
        if (!req.body().empty()) {
            request_json = json::parse(req.body());
        }
    } catch (const std::exception& e) {
        header_print("LOG", "Error parsing request body: " + std::string(e.what()));
    }
    brief_print_message_request(request_json);
    
    std::string key = std::string(req.method_string()) + " " + std::string(req.target());

    auto it = routes.find(key);
    if (it != routes.end()) {
        // Check if this endpoint requires NPU access
        bool needs_npu = requires_npu_access(std::string(req.method_string()), std::string(req.target()));
        
        if (needs_npu) {
            // Try to acquire NPU access using the shared manager
            if (!NPUAccessManager::try_acquire_npu_access()) {
                // NPU is currently in use by another request
                res.result(http::status::service_unavailable);
                res.body() = json{{"error", "NPU is currently in use by another request. Please try again later."}}.dump();
                res.set(http::field::content_type, "application/json");
                res.prepare_payload();
                header_print("🚫 ", "NPU access denied for request: " + key);
                return;
            }
            
            header_print("LOG", "NPU access granted for request: " + key);
        }
        
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
        
        // Create cancellation token for this request
        auto cancellation_token = std::make_shared<CancellationToken>(session);
        
        // Extract request_id for tracking (generate one if not provided)
        std::string request_id;
        if (request_json.contains("request_id")) {
            request_id = request_json["request_id"];
        } else {
            // Generate a unique request ID
            static std::atomic<int> counter{0};
            request_id = "req_" + std::to_string(counter.fetch_add(1));
        }
        
        // Register the request for potential cancellation
        register_active_request(request_id, cancellation_token);
        
        // Create response callback that unregisters the request when done
        auto send_response = [&res, this, request_id, needs_npu](const json& response_data) {
            res.result(http::status::ok);
            res.body() = response_data.dump();
            res.set(http::field::content_type, "application/json");
            res.prepare_payload();
            unregister_active_request(request_id);
            
            // Release NPU access if this was an NPU-intensive request
            if (needs_npu) {
                NPUAccessManager::release_npu_access();
                header_print("LOG", "NPU access released for request: " + request_id);
            }
        };
        
        // Create streaming response callback that uses the session
        auto send_streaming_response = [session, this, request_id, needs_npu](const json& data, bool is_final) {
            if (session) {
                session->write_streaming_response(data, is_final);
            }
            if (is_final) {
                unregister_active_request(request_id);
                
                // Release NPU access if this was an NPU-intensive request
                if (needs_npu) {
                    NPUAccessManager::release_npu_access();
                    header_print("LOG", "NPU access released for streaming request: " + request_id);
                }
            }
        };
        
        // Call the handler with the session and cancellation token
        it->second(req, send_response, send_streaming_response, session, cancellation_token);
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
                      std::shared_ptr<HttpSession> session,
                      std::shared_ptr<CancellationToken> cancellation_token) {
            json request_json;
            if (!req.body().empty()) {
                request_json = json::parse(req.body());
            }
            rest_handler->handle_generate(request_json, send_response, send_streaming_response, cancellation_token);
        });
    
    server->register_handler("POST", "/api/chat",
        [rest_handler](const http::request<http::string_body>& req,
                      std::function<void(const json&)> send_response,
                      std::function<void(const json&, bool)> send_streaming_response,
                      std::shared_ptr<HttpSession> session,
                      std::shared_ptr<CancellationToken> cancellation_token) {
            json request_json;
            if (!req.body().empty()) {
                request_json = json::parse(req.body());
            }
            rest_handler->handle_chat(request_json, send_response, send_streaming_response, cancellation_token);
        });

    server->register_handler("GET", "/api/ps",
        [rest_handler](const http::request<http::string_body>& req,
                      std::function<void(const json&)> send_response,
                      std::function<void(const json&, bool)> send_streaming_response,
                      std::shared_ptr<HttpSession> session,
                      std::shared_ptr<CancellationToken> cancellation_token) {
            json request_json;
            rest_handler->handle_ps(request_json, send_response, send_streaming_response);
        });

    server->register_handler("POST", "/api/embeddings",
        [rest_handler](const http::request<http::string_body>& req,
                      std::function<void(const json&)> send_response,
                      std::function<void(const json&, bool)> send_streaming_response,
                      std::shared_ptr<HttpSession> session,
                      std::shared_ptr<CancellationToken> cancellation_token) {
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
                      std::shared_ptr<HttpSession> session,
                      std::shared_ptr<CancellationToken> cancellation_token) {
            json request_json;
            rest_handler->handle_models(request_json, send_response, send_streaming_response);
        });
    
    server->register_handler("GET", "/api/version",
        [rest_handler](const http::request<http::string_body>& req,
                      std::function<void(const json&)> send_response,
                      std::function<void(const json&, bool)> send_streaming_response,
                      std::shared_ptr<HttpSession> session,
                      std::shared_ptr<CancellationToken> cancellation_token) {
            json request_json;
            rest_handler->handle_version(request_json, send_response, send_streaming_response);
        });
    
    // Add NPU status endpoint
    server->register_handler("GET", "/api/npu/status",
        [](const http::request<http::string_body>& req,
           std::function<void(const json&)> send_response,
           std::function<void(const json&, bool)> send_streaming_response,
           std::shared_ptr<HttpSession> session,
           std::shared_ptr<CancellationToken> cancellation_token) {
            json response = {
                {"npu_available", NPUAccessManager::is_npu_available()},
                {"active_requests", NPUAccessManager::get_active_npu_requests()},
                {"message", NPUAccessManager::is_npu_available() ? "NPU is available" : "NPU is currently in use"}
            };
            send_response(response);
        });
    
    // Add other endpoints...
    server->register_handler("POST", "/api/pull",
        [rest_handler](const http::request<http::string_body>& req,
                      std::function<void(const json&)> send_response,
                      std::function<void(const json&, bool)> send_streaming_response,
                      std::shared_ptr<HttpSession> session,
                      std::shared_ptr<CancellationToken> cancellation_token) {
            json request_json;
            if (!req.body().empty()) {
                request_json = json::parse(req.body());
            }
            rest_handler->handle_pull(request_json, send_response, send_streaming_response);
        });
    
    server->register_handler("POST", "/v1/chat/completions",
        [rest_handler](const http::request<http::string_body>& req,
                      std::function<void(const json&)> send_response,
                      std::function<void(const json&, bool)> send_streaming_response,
                      std::shared_ptr<HttpSession> session,
                      std::shared_ptr<CancellationToken> cancellation_token) {
            json request_json;
            if (!req.body().empty()) {
                request_json = json::parse(req.body());
            }
            rest_handler->handle_openai_chat_completion(request_json, send_response, send_streaming_response, cancellation_token);
        });
    
    // Add cancel endpoint - capture server by raw pointer
    WebServer* server_ptr = server.get();
    server->register_handler("POST", "/api/cancel",
        [server_ptr](const http::request<http::string_body>& req,
                     std::function<void(const json&)> send_response,
                     std::function<void(const json&, bool)> send_streaming_response,
                     std::shared_ptr<HttpSession> session,
                     std::shared_ptr<CancellationToken> cancellation_token) {
            json request_json;
            if (!req.body().empty()) {
                request_json = json::parse(req.body());
            }
            
            if (!request_json.contains("request_id")) {
                json error_response = {{"error", "request_id is required"}};
                send_response(error_response);
                return;
            }
            
            std::string request_id = request_json["request_id"];
            
            // Try to cancel the request
            bool cancelled = server_ptr->cancel_request(request_id);
            
            json response;
            if (cancelled) {
                response["cancelled"] = true;
                response["message"] = "Request cancelled successfully";
            } else {
                response["cancelled"] = false;
                response["message"] = "Request not found or already completed";
            }
            send_response(response);
        });
    
    return server;
}