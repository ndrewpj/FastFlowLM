/*!
 *  Copyright (c) 2023 by Contributors
 * \file streaming_ostream.hpp
 * \brief Custom ostream for streaming
 * \author FastFlowLM Team
 * \date 2025-06-24
 * \version 1.0.0
 */
#pragma once

#include <ostream>
#include <streambuf>
#include <functional>
#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

///@brief Custom streambuf that captures tokens and sends them immediately
///@param model the model
///@param callback the callback
///@param is_chat_format the is chat format
///@return the streaming buf
///@note The packet is sent every std::flush
class streaming_buf : public std::streambuf {
public:
    ///@brief StreamCallback
    using StreamCallback = std::function<void(const json&, bool)>;
    
    streaming_buf(const std::string& model, StreamCallback callback, bool is_chat_format = false)
        : model_name(model), stream_callback(callback), is_chat(is_chat_format) {}

protected:
    ///@brief Called when buffer is full or flush is requested
    ///@param ch the character
    ///@return the character
    int_type overflow(int_type ch) override {
        if (ch != traits_type::eof()) {
            buffer += static_cast<char>(ch);
        }
        return ch;
    }
    
    ///@brief Called when stream is flushed
    ///@return 0
    int sync() override {
        flush_buffer(false);
        return 0;
    }

public:
    ///@brief Call this when generation is complete
    void finalize_chat() {
        if (!buffer.empty()) {
            flush_buffer(true);
        } else {
            send_chat_final_response();
        }
    }
    ///@brief Call this when generation is complete
    ///@param context the context
    void finalize_generate(std::vector<int>& context) {
        if (!buffer.empty()) {
            flush_buffer(true);
        } else {
            send_generate_final_response(context);
        }
    }

private:
    ///@brief Flush the buffer
    ///@param is_final the is final
    void flush_buffer(bool is_final) {
        if (!buffer.empty()) {
            send_response(buffer, is_final);
            buffer.clear();
        }
    }
    
    ///@brief Send the response
    ///@param content the content
    ///@param is_final the is final
    void send_response(const std::string& content, bool is_final) {
        json response;
        
        if (is_chat) {
            response = {
                {"model", model_name},
                {"message", {
                    {"role", "assistant"},
                    {"content", content}
                }},
                {"done", is_final}
            };
        } else {
            response = {
                {"model", model_name},
                {"response", content},
                {"done", is_final}
            };
        }
        
        stream_callback(response, is_final);
    }

    ///@brief Send the chat final response
    void send_chat_final_response() {
        json response;
        
        response = {
            {"model", model_name},
            {"message", {
                {"role", "assistant"},
                {"content", ""}
            }},
            {"done", true}
        };
        
        stream_callback(response, true);
    }
    
    ///@brief Send the generate final response
    ///@param content the content
    void send_generate_final_response(const std::vector<int>& content) {
        json response;
        
        response = {
            {"model", model_name},
            {"response", ""},
            {"context", content},
            {"done", true}
        };
        
        stream_callback(response, true);
    }
    ///@brief Buffer
    std::string buffer;
    ///@brief Model name
    std::string model_name;
    ///@brief Stream callback
    StreamCallback stream_callback;
    ///@brief Is chat
    bool is_chat;
};

///@brief Custom ostream for streaming
///@param model the model
///@param callback the callback
///@param is_chat_format the is chat format
///@return the streaming ostream
class streaming_ostream : public std::ostream {
public:
    streaming_ostream(const std::string& model, streaming_buf::StreamCallback callback, bool is_chat_format = false)
        : std::ostream(&buf), buf(model, callback, is_chat_format) {}
    
    ///@brief Finalize the chat
    void finalize_chat() {
        buf.finalize_chat();
    }
    ///@brief Finalize the generate
    ///@param context the context
    void finalize_generate(std::vector<int>& context) {
        buf.finalize_generate(context);
    }

private:
    ///@brief Buffer
    streaming_buf buf;
}; 