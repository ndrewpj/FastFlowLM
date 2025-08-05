/*!
 *  Copyright (c) 2023 by Contributors
 * \file streaming_ostream.hpp
 * \brief Custom ostream for streaming
 * \author FastFlowLM Team
 * \date 2025-06-24
 * \version 0.9.2
 */
#pragma once

#include <ostream>
#include <streambuf>
#include <functional>
#include <string>
#include <vector>
#include <random>
#include <sstream>
#include <iomanip>
#include <nlohmann/json.hpp>
#include "chat/chat_bot.hpp"

using json = nlohmann::ordered_json;

///@brief Custom streambuf that captures tokens and sends them immediately
///@param model the model
///@param callback the callback
///@param is_chat_format the is chat format
///@return the streaming buf
///@note The packet is sent every std::flush, but only when UTF-8 sequences are complete
class streaming_buf_openai : public std::streambuf {
public:
    ///@brief StreamCallback
    using StreamCallback = std::function<void(const std::string&, bool)>;
    
    streaming_buf_openai(const std::string& model, StreamCallback callback)
        : model_name(model), stream_callback(callback), first_chunk(true) {
        // Generate a unique ID for this stream
        generate_stream_id();
    }

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
        flush_complete_utf8_sequences(false);
        return 0;
    }

public:
    ///@brief Call this when generation is complete
    void finalize(chat_meta_info& meta_info) {
        // Send all remaining content, including incomplete sequences
        if (!buffer.empty()) {
            send_response(buffer, true);
            buffer.clear();
        }
        send_final_response(meta_info);
    }
 

private:
    ///@brief Generate a unique stream ID
    void generate_stream_id() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 15);
        
        std::stringstream ss;
        ss << "chatcmpl-";
        for (int i = 0; i < 24; ++i) {
            ss << std::hex << dis(gen);
        }
        stream_id = ss.str();
    }
    
    ///@brief Get UTF-8 sequence length from first byte
    ///@param first_byte the first byte
    ///@return sequence length, or 0 if invalid
    size_t get_utf8_sequence_length(unsigned char first_byte) {
        if ((first_byte & 0x80) == 0) {
            return 1; // Single byte sequence
        } else if ((first_byte & 0xE0) == 0xC0) {
            return 2; // Two byte sequence
        } else if ((first_byte & 0xF0) == 0xE0) {
            return 3; // Three byte sequence
        } else if ((first_byte & 0xF8) == 0xF0) {
            return 4; // Four byte sequence
        }
        return 0; // Invalid UTF-8 start byte
    }
    
    ///@brief Flush only complete UTF-8 sequences
    ///@param is_final the is final
    void flush_complete_utf8_sequences(bool is_final) {
        if (buffer.empty()) return;
        
        std::string complete_content;
        size_t pos = 0;
        
        // Process complete UTF-8 sequences
        while (pos < buffer.size()) {
            unsigned char first = static_cast<unsigned char>(buffer[pos]);
            size_t seq_len = get_utf8_sequence_length(first);
            
            if (seq_len == 0) {
                // Invalid UTF-8 start byte, skip it
                pos++;
                continue;
            }
            
            // Check if we have a complete sequence
            if (pos + seq_len > buffer.size()) {
                // Incomplete sequence, stop here
                break;
            }
            
            // Add complete sequence to output
            complete_content.append(buffer, pos, seq_len);
            pos += seq_len;
        }
        
        // Send complete sequences if any
        if (!complete_content.empty()) {
            send_response(complete_content, is_final);
        }
        
        // Remove processed bytes from buffer
        if (pos > 0) {
            buffer.erase(0, pos);
        }
    }
    
    ///@brief Send the response
    ///@param content the content
    ///@param is_final the is final
    void send_response(const std::string& content, bool is_final) {
        json response;
        
        if (first_chunk) {
            // First chunk with role
            response = {
                {"id", stream_id},
                {"object", "chat.completion.chunk"},
                {"choices", json::array({
                    {
                        {"delta", {
                            {"role", "assistant"},
                            {"content", ""}
                        }},
                        {"index", 0}
                    }
                })}
            };
            first_chunk = false;
            stream_callback("data: " + response.dump() + "\n\n", false);
        }
        
        // Content chunk
        response = {
            {"id", stream_id},
            {"object", "chat.completion.chunk"},
            {"choices", json::array({
                {
                    {"delta", {
                        {"content", content}
                    }},
                    {"index", 0}
                }
            })}
        };
        
        stream_callback("data: " + response.dump() + "\n\n", is_final);
      
    }

    ///@brief Send the chat final response
    void send_final_response(chat_meta_info& meta_info) {
        if (first_chunk) {
            // If no content was sent, send role first
            json response = {
                {"id", stream_id},
                {"object", "chat.completion.chunk"},
                {"choices", json::array({
                    {
                        {"delta", {
                            {"role", "assistant"}
                        }},
                        {"index", 0}
                    }
                })}
            };
            stream_callback("data: " + response.dump() + "\n\n", false);
        }
        
        // Send final chunk with finish_reason only (no empty delta)
        json final_response = {
            {"id", stream_id},
            {"object", "chat.completion.chunk"},
            {"choices", json::array({
                {
                    {"delta", json::object()},
                    {"finish_reason", "stop"},
                    {"index", 0}
                }
            })},
            {"usage", {
                {"prompt_tokens", meta_info.prompt_tokens},
                {"completion_tokens", meta_info.generated_tokens},
                {"total_tokens", meta_info.prompt_tokens + meta_info.generated_tokens}
            }}
        };
        stream_callback("data: " + final_response.dump() + "\n\n", false);
        // Send the [DONE] message
        stream_callback("data: [DONE]\n\n", true);
    }
    

    ///@brief Buffer
    std::string buffer;
    ///@brief Model name
    std::string model_name;
    ///@brief Stream callback
    StreamCallback stream_callback;
    ///@brief Stream ID
    std::string stream_id;
    ///@brief First chunk flag
    bool first_chunk;
};

///@brief Custom ostream for streaming
///@param model the model
///@param callback the callback
///@param is_chat_format the is chat format
///@return the streaming ostream
class streaming_ostream_openai : public std::ostream {
public:
    streaming_ostream_openai(const std::string& model, streaming_buf_openai::StreamCallback callback)
        : std::ostream(&buf), buf(model, callback) {}
    
    ///@brief Finalize the chat
    void finalize(chat_meta_info& meta_info) {
        buf.finalize(meta_info);
    }

private:
    ///@brief Buffer
    streaming_buf_openai buf;
};


