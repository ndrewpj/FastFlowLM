/*!
 *  Copyright (c) 2023 by Contributors
 * \file rest_handler.cpp
 * \brief RestHandler class and related declarations
 * \author FastFlowLM Team
 * \date 2025-08-05
 * \version 0.9.6
 */
#include "rest_handler.hpp"
#include "wstream_buf.hpp"
#include "streaming_ostream.hpp"
#include "streaming_ostream_openai.hpp"
#include "image/image_reader.hpp"
#include <sstream>
#include <iostream>
#include <thread>
#include <chrono>
#include <iomanip>
#include <locale>
#include <random>

///@brief RestHandler constructor
///@param models the model list
///@param downloader the downloader
///@param default_tag the default tag
///@return the rest handler
RestHandler::RestHandler(model_list& models, ModelDownloader& downloader, const std::string& default_tag)
    : supported_models(models), downloader(downloader), default_model_tag(default_tag), current_model_tag("") {
    // Initialize chat bot with default model
    chat_engine = std::make_unique<chat_bot>(0);
    ensure_model_loaded(default_model_tag);
}

///@brief RestHandler destructor
///@return the rest handler
RestHandler::~RestHandler() = default;

///@brief Ensure the model is loaded
///@param model_tag the model tag
void RestHandler::ensure_model_loaded(const std::string& model_tag) {
    if (current_model_tag != model_tag) {
        if (!downloader.is_model_downloaded(model_tag)) {
            downloader.pull_model(model_tag);
        }
        std::string tag_copy = model_tag; // Create non-const copy
        nlohmann::json model_info = supported_models.get_model_info(tag_copy);
        chat_engine->load_model(supported_models.get_model_path(tag_copy), model_info);
        current_model_tag = model_tag;
    }
}

///@brief Handle the generate request
///@param request the request
///@param send_response the send response
///@param send_streaming_response the send streaming response
void RestHandler::handle_generate(const json& request,
                                 std::function<void(const json&)> send_response,
                                 StreamResponseCallback send_streaming_response,
                                 std::shared_ptr<CancellationToken> cancellation_token) {
    try {
        std::string prompt = request["prompt"];
        bool stream = request.value("stream", false);
        std::string model = request.value("model", default_model_tag);
        json options = request.value("options", json::object());
        int temperature = options.value("temperature", 0.6);
        int top_p = options.value("top_p", 0.9);
        int top_k = options.value("top_k", 5);
        float frequency_penalty = options.value("frequency_penalty", 1.1);
        int length_limit = request.value("max_tokens", 4096);
        bool enable_thinking = request.value("think", false);
        auto load_start_time = time_utils::now();
        ensure_model_loaded(model);
        auto load_end_time = time_utils::now();
        chat_engine->set_enable_think(enable_thinking);
        chat_meta_info meta_info;
        
        meta_info.load_duration = (uint64_t)time_utils::duration_ns(load_start_time, load_end_time).first;
        header_print("FLM", "Start generating...");
        
        if (stream) {
            // Streaming response using streaming_ostream
            auto total_start_time = time_utils::now();
            streaming_ostream ostream(model, send_streaming_response, false);
            std::vector<int> prompts = chat_engine->tokenize(prompt, true, "user", true);
            bool success = chat_engine->insert(meta_info, prompts);
            if (!success){
                json error_response = {{"error", "Max length reached"}};
                send_response(error_response);
                return;
            }
            chat_engine->generate(meta_info, length_limit, ostream);
            auto total_end_time = time_utils::now();
            auto history = this->chat_engine->get_history();
            // std::cout << "history: " << history.first << std::endl;
            meta_info.total_duration = (uint64_t)time_utils::duration_ns(total_start_time, total_end_time).first;
            ostream.finalize_generate(meta_info, history.second);
        } else {
            // Non-streaming response
            std::stringstream ss;
            wstream_buf obuf(ss);
            std::ostream ostream(&obuf);
            std::vector<int> prompts = chat_engine->tokenize(prompt, true, "user", true);
            bool success = chat_engine->insert(meta_info, prompts);
            if (!success){
                json error_response = {{"error", "Max length reached"}};
                send_response(error_response);
                return;
            }
            chat_engine->generate(meta_info, length_limit, ostream);
            std::string response_text = ss.str();
            auto history = this->chat_engine->get_history();
            json response = {
                {"model", model},
                {"response", response_text},
                {"context", history.second},
                {"done", true},
                {"prompt_eval_count", meta_info.prompt_tokens},
                {"eval_count", meta_info.generated_tokens},
                {"total_duration", meta_info.total_duration},
                {"load_duration", meta_info.load_duration},
                {"prompt_eval_duration", meta_info.prefill_duration},
                {"eval_duration", meta_info.decoding_duration},
                {"done_reason", stop_reason_to_string(meta_info.stop_reason)}
            };
            // std::cout << "history: " << history.first << std::endl;
            send_response(response);
        }
    } catch (const std::exception& e) {
        json error_response = {{"error", e.what()}};
        send_response(error_response);
    }
}

///@brief Handle the chat request
///@param request the request
///@param send_response the send response
///@param send_streaming_response the send streaming response
void RestHandler::handle_chat(const json& request,
                             std::function<void(const json&)> send_response,
                             StreamResponseCallback send_streaming_response,
                             std::shared_ptr<CancellationToken> cancellation_token) {
    try {
        nlohmann::ordered_json messages = request["messages"];
        bool stream = request.value("stream", false);
        std::string model = request.value("model", default_model_tag);
        json options = request.value("options", json::object());
        float temperature = options.value("temperature", 0.6);
        float top_p = options.value("top_p", 0.9);
        int top_k = options.value("top_k", 5);
        float frequency_penalty = options.value("frequency_penalty", 1.1);
        int length_limit = options.value("num_predict", 4096);
        bool enable_thinking = request.value("think", false);
        auto load_start_time = time_utils::now();
        ensure_model_loaded(model);
        auto load_end_time = time_utils::now();
        int total_images = 0;
        for (auto& message : messages){
            nlohmann::ordered_json::array_t images = message.value("images", nlohmann::ordered_json::array());
            if (images.size() > 0){
                total_images += images.size();
            }
        }
        header_print("FLM", "Total images: " << total_images);
        // temporary solution
        bytes pixel_values(3 * 896 * 896 * sizeof(bf16) * total_images);
        uint8_t* pixel_values_ptr = pixel_values.data();
        if (total_images > 0){
            for (auto& message : messages){
                nlohmann::ordered_json::array_t images = message.value("images", nlohmann::ordered_json::array());
                for (auto& image : images){
                    std::string image_str = image.get<std::string>();
                    bytes image_rgb = load_image_base64(image_str);
                    buffer<bf16> pv = preprocess_image(image_rgb);
                    memcpy(pixel_values_ptr, pv.data(), pv.size() * sizeof(bf16));
                    pixel_values_ptr += pv.size() * sizeof(bf16);
                }
            }
        }
        chat_engine->set_temperature(temperature);
        chat_engine->set_topp(top_p);
        chat_engine->set_topk(top_k);
        chat_engine->set_frequency_penalty(frequency_penalty);
        chat_engine->set_enable_think(enable_thinking);
        chat_meta_info meta_info;
        meta_info.load_duration = (uint64_t)time_utils::duration_ns(load_start_time, load_end_time).first;
        void* payload = pixel_values.size() > 0 ? static_cast<void*>(&pixel_values) : nullptr;
        header_print("FLM", "Start generating...");
        if (stream) {
            // Streaming response using streaming_ostream
            auto total_start_time = time_utils::now();
            streaming_ostream ostream(model, send_streaming_response, true);  // true for chat format
            std::vector<int> prompts = chat_engine->tokenize(messages, true);
            bool success = chat_engine->insert(meta_info, prompts, false, payload);
            if (!success){
                json error_response = {{"error", "Max length reached"}};
                send_response(error_response);
                return;
            }
            chat_engine->generate(meta_info, length_limit, ostream);
            auto total_end_time = time_utils::now();
            meta_info.total_duration = (uint64_t)time_utils::duration_ns(total_start_time, total_end_time).first;
            
            ostream.finalize_chat(meta_info);
            // auto history = this->chat_engine->get_history();
            // std::cout << "history: " << history.first << std::endl;
            this->chat_engine->clear_context();
        } else {
            // Non-streaming response
            std::vector<int> prompts = chat_engine->tokenize(messages, true);
            auto total_start_time = time_utils::now();
            nullstream nstream;
            std::string response_text = chat_engine->generate_with_prompt(meta_info, prompts, length_limit, std::cout, payload);
            auto total_end_time = time_utils::now();
            meta_info.total_duration = (uint64_t)time_utils::duration_ns(total_start_time, total_end_time).first;
            
            json response = {
                {"model", model},
                {"message", {
                    {"role", "assistant"},
                    {"content", response_text},
                    {"images", nullptr}
                }},
                {"done", true},
                {"prompt_eval_count", meta_info.prompt_tokens},
                {"eval_count", meta_info.generated_tokens},
                {"total_duration", meta_info.total_duration},
                {"load_duration", meta_info.load_duration},
                {"prompt_eval_duration", meta_info.prefill_duration},
                {"eval_duration", meta_info.decoding_duration},
                {"done_reason", stop_reason_to_string(meta_info.stop_reason)}
            };
            send_response(response);
            
            // auto history = this->chat_engine->get_history();
            // std::cout << "history: " << history.first << std::endl;
            this->chat_engine->clear_context();
        }
    } catch (const std::exception& e) {
        json error_response = {{"error", e.what()}};
        send_response(error_response);
    }
}

///@brief Handle the embeddings request
///@param request the request
///@param send_response the send response
///@param send_streaming_response the send streaming response
void RestHandler::handle_embeddings(const json& request,
                                   std::function<void(const json&)> send_response,
                                   StreamResponseCallback send_streaming_response) {
    try {
        std::string prompt = request["prompt"];
        
        // Note: This is a placeholder. You'll need to implement actual embedding generation
        std::vector<float> embeddings(chat_engine->get_current_context_length(), 0.0f);
        
        json response = {
            {"embeddings", embeddings}
        };
        send_response(response);
    } catch (const std::exception& e) {
        json error_response = {{"error", e.what()}};
        send_response(error_response);
    }
}

///@brief Handle the models request
///@param request the request
///@param send_response the send response
///@param send_streaming_response the send streaming response
void RestHandler::handle_models(const json& request,
                               std::function<void(const json&)> send_response,
                               StreamResponseCallback send_streaming_response) {
    try {
        json models = supported_models.get_all_models();
        send_response(models);
    } catch (const std::exception& e) {
        json error_response = {{"error", e.what()}};
        send_response(error_response);
    }
}

///@brief Handle the ps request
///@param request the request
///@param send_response the send response
///@param send_streaming_response the send streaming response
void RestHandler::handle_ps(const json& request,
                             std::function<void(const json&)> send_response,
                             StreamResponseCallback send_streaming_response) {
    try {
        // Generate expires_at timestamp (1 hour from now)
        auto now = std::chrono::system_clock::now();
        auto expires_time = now + std::chrono::hours(1);
        auto expires_time_t = std::chrono::system_clock::to_time_t(expires_time);
        auto expires_tp = std::chrono::system_clock::from_time_t(expires_time_t);
        auto fractional_seconds = std::chrono::duration_cast<std::chrono::microseconds>(expires_time - expires_tp).count();
        
        // Get local time and timezone offset
        std::tm* local_tm = std::localtime(&expires_time_t);
        std::tm* utc_tm = std::gmtime(&expires_time_t);
        
        // Calculate timezone offset in minutes
        int offset_minutes = (local_tm->tm_hour - utc_tm->tm_hour) * 60 + (local_tm->tm_min - utc_tm->tm_min);
        if (local_tm->tm_mday != utc_tm->tm_mday) {
            offset_minutes += (local_tm->tm_mday > utc_tm->tm_mday) ? 1440 : -1440;
        }
        
        std::stringstream expires_ss;
        expires_ss.imbue(std::locale::classic()); // Use C locale to avoid commas
        expires_ss << std::put_time(local_tm, "%Y-%m-%dT%H:%M:%S");
        expires_ss << "." << std::setfill('0') << std::setw(5) << (fractional_seconds / 40); // 5 decimal places
        
        // Format timezone offset
        int offset_hours = offset_minutes / 60;
        int offset_mins = abs(offset_minutes % 60);
        expires_ss << (offset_minutes >= 0 ? "+" : "-") 
                  << std::setfill('0') << std::setw(2) << abs(offset_hours)
                  << ":" << std::setfill('0') << std::setw(2) << offset_mins;
        
        std::string expires_at = expires_ss.str();
        
        json model_info = supported_models.get_model_info(current_model_tag);
        json response = {
            {"models", json::array({
                {
                    {"name", current_model_tag},
                    {"model", current_model_tag},
                    {"size", model_info["size"]},
                    {"details", model_info["details"]},
                    {"expires_at", expires_at},
                }
            })}
        };
        // std::cout << "response: " << response.dump(4) << std::endl;
        send_response(response);
    } catch (const std::exception& e) {
        json error_response = {{"error", e.what()}};
        send_response(error_response);
    }
}

///@brief Handle the version request
///@param request the request
///@param send_response the send response
///@param send_streaming_response the send streaming response
void RestHandler::handle_version(const json& request,
                                std::function<void(const json&)> send_response,
                                StreamResponseCallback send_streaming_response) {
    json response = {{"version", "1.0.0"}};
    send_response(response);
}

///@brief Handle the pull request
///@param request the request
///@param send_response the send response
///@param send_streaming_response the send streaming response
void RestHandler::handle_pull(const json& request,
                             std::function<void(const json&)> send_response,
                             StreamResponseCallback send_streaming_response) {
    json error_response = {{"error", "Pull operation not implemented"}};
    send_response(error_response);
}

///@brief Handle the push request
///@param request the request
///@param send_response the send response
///@param send_streaming_response the send streaming response
void RestHandler::handle_push(const json& request,
                             std::function<void(const json&)> send_response,
                             StreamResponseCallback send_streaming_response) {
    json error_response = {{"error", "Push operation not implemented"}};
    send_response(error_response);
}

///@brief Handle the delete request
///@param request the request
///@param send_response the send response
///@param send_streaming_response the send streaming response
void RestHandler::handle_delete(const json& request,
                               std::function<void(const json&)> send_response,
                               StreamResponseCallback send_streaming_response) {
    json error_response = {{"error", "Delete operation not implemented"}};
    send_response(error_response);
}

///@brief Handle the copy request
///@param request the request
///@param send_response the send response
///@param send_streaming_response the send streaming response
void RestHandler::handle_copy(const json& request,
                             std::function<void(const json&)> send_response,
                             StreamResponseCallback send_streaming_response) {
    json error_response = {{"error", "Copy operation not implemented"}};
    send_response(error_response);
}

///@brief Handle the create request
///@param request the request
///@param send_response the send response
///@param send_streaming_response the send streaming response
void RestHandler::handle_create(const json& request,
                               std::function<void(const json&)> send_response,
                               StreamResponseCallback send_streaming_response) {
    json error_response = {{"error", "Create operation not implemented"}};
    send_response(error_response);
}

///@brief Handle the openai chat completion request
///@param request the request
///@param send_response the send response
///@param send_streaming_response the send streaming response
void RestHandler::handle_openai_chat_completion(const json& request,
                                               std::function<void(const json&)> send_response,
                                               StreamResponseCallback send_streaming_response,
                                               std::shared_ptr<CancellationToken> cancellation_token) {
    try {
        nlohmann::ordered_json messages = request["messages"];
        std::string model = request.value("model", default_model_tag);
        bool stream = request.value("stream", false);
        
        // Extract OpenAI-style parameters
        json options = request.value("options", json::object());
        float temperature = request.value("temperature", 0.6);
        float top_p = request.value("top_p", 0.9);
        int top_k = request.value("top_k", 5);
        float frequency_penalty = request.value("frequency_penalty", 1.1);
        int length_limit = request.value("max_tokens", 4096);
        bool enable_thinking = request.value("think", false);
        ensure_model_loaded(model);
        chat_engine->set_enable_think(enable_thinking);
        chat_engine->set_temperature(temperature);
        chat_engine->set_topp(top_p);
        chat_engine->set_topk(top_k);
        chat_engine->set_frequency_penalty(frequency_penalty);
        chat_meta_info meta_info;
        header_print("FLM", "Start generating...");
        if (stream){
            // Create a wrapper callback that passes the pre-formatted SSE string directly
            auto openai_stream_callback = [&send_streaming_response](const std::string& data, bool is_final) {
                // Create a JSON string to pass through the existing callback interface
                json data_json = data;
                send_streaming_response(data_json, is_final);
            };
            
            // Streaming response using streaming_ostream_openai
            streaming_ostream_openai ostream(model, openai_stream_callback);  // true for chat format
            std::vector<int> prompts = chat_engine->tokenize(messages, true);
            std::string response_text = chat_engine->generate_with_prompt(meta_info, prompts, length_limit, ostream);
            ostream.finalize(meta_info);
            // auto history = this->chat_engine->get_history();
            // std::cout << "history: " << history.first << std::endl;
            this->chat_engine->clear_context();
        }
        else {
            std::vector<int> prompts = chat_engine->tokenize(messages, true);
            nullstream nstream;
            std::string response_text = chat_engine->generate_with_prompt(meta_info, prompts, length_limit, nstream);
            json response = {
                {"id", "fastflowlm-chat-completion"},
                {"object", "chat.completion"},
                {"created", (int)std::time(nullptr)},
                {"model", model},
                {"choices", json::array({
                    {
                        {"message", {
                            {"role", "assistant"},
                            {"content", response_text}
                        }},
                        {"finish_reason", "stop"}
                    }
                })},
                {"usage", {
                    {"prompt_tokens", meta_info.prompt_tokens},
                    {"completion_tokens", meta_info.generated_tokens},
                    {"total_tokens", meta_info.prompt_tokens + meta_info.generated_tokens}
                }}
            };
            send_response(response);
            // auto history = this->chat_engine->get_history();
            // std::cout << "history: " << history.first << std::endl;
            this->chat_engine->clear_context();
        }

    } catch (const std::exception& e) {
        json error_response = {
            {"error", {
                {"message", e.what()},
                {"type", "server_error"},
                {"code", 500}
            }}
        };
        send_response(error_response);
    }
}