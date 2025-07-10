/*!
 *  Copyright (c) 2023 by Contributors
 * \file rest_handler.hpp
 * \brief RestHandler class and related declarations
 * \author FastFlowLM Team
 * \date 2025-06-24
 * \version 0.1.0
 */
#pragma once

#include "chat/chat_bot.hpp"
#include "model_list.hpp"
#include "model_downloader.hpp"
#include <nlohmann/json.hpp>
#include <string>
#include <memory>
#include <functional>

using json = nlohmann::ordered_json;

// Forward declaration
struct CancellationToken;

///@brief Stream callback type for sending streaming responses
using StreamResponseCallback = std::function<void(const json&, bool)>; // data, is_final

class RestHandler {
public:
    RestHandler(model_list& models, ModelDownloader& downloader, const std::string& default_tag);
    ~RestHandler();

    void handle_generate(const json& request, 
                        std::function<void(const json&)> send_response,
                        StreamResponseCallback send_streaming_response,
                        std::shared_ptr<CancellationToken> cancellation_token = nullptr);

    void handle_chat(const json& request,
                    std::function<void(const json&)> send_response, 
                    StreamResponseCallback send_streaming_response,
                    std::shared_ptr<CancellationToken> cancellation_token = nullptr);
    

    void handle_embeddings(const json& request,
                          std::function<void(const json&)> send_response,
                          StreamResponseCallback send_streaming_response);
    

    void handle_models(const json& request,
                      std::function<void(const json&)> send_response,
                      StreamResponseCallback send_streaming_response);
    

    void handle_ps(const json& request,
                    std::function<void(const json&)> send_response,
                    StreamResponseCallback send_streaming_response);
    
    void handle_version(const json& request,
                       std::function<void(const json&)> send_response,
                       StreamResponseCallback send_streaming_response);
    
    // Placeholder handlers for unimplemented endpoints
    void handle_pull(const json& request,
                    std::function<void(const json&)> send_response,
                    StreamResponseCallback send_streaming_response);
    
    void handle_push(const json& request,
                    std::function<void(const json&)> send_response,
                    StreamResponseCallback send_streaming_response);
    
    void handle_delete(const json& request,
                      std::function<void(const json&)> send_response,
                      StreamResponseCallback send_streaming_response);
    
    void handle_copy(const json& request,
                    std::function<void(const json&)> send_response,
                    StreamResponseCallback send_streaming_response);
    
    void handle_create(const json& request,
                      std::function<void(const json&)> send_response,
                      StreamResponseCallback send_streaming_response);

    void handle_openai_chat_completion(const json& request,
                                      std::function<void(const json&)> send_response,
                                      StreamResponseCallback send_streaming_response,
                                      std::shared_ptr<CancellationToken> cancellation_token = nullptr);

private:
    void ensure_model_loaded(const std::string& model_tag);

    
    
    std::unique_ptr<chat_bot> chat_engine;
    model_list& supported_models;
    ModelDownloader& downloader;
    std::string current_model_tag;
    std::string default_model_tag;
    int generate_context_id;
    int chat_context_id;
    std::string last_question;
}; 