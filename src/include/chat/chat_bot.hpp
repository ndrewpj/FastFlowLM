/// \file chat_bot.hpp
/// \brief chat_bot class
/// \author FastFlowLM Team
/// \date 2025-06-24
/// \version 0.9.0
/// \note This is a header file for the chat_bot class
#pragma once

#include <ctime>
#include <iomanip>
#include <sstream>
#include <memory>
#include <vector>
#include <iostream>
#include <string>
#include <type_traits>
#include "typedef.hpp"
#include "causal_lm.hpp"
#include "lm_config.hpp"
#include "llama/llama_npu.hpp"
#include "qwen/qwen_npu.hpp"
#include "tokenizer/tokenizer.hpp"
#include "modules/sampler.hpp"
#include "utils/utils.hpp"
#include "utils/profiler.hpp"
#include "tensor_utils/q4_npu_eXpress.hpp"
#include "npu_utils/npu_utils.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::ordered_json;

typedef enum {
    EOT_DETECTED,
    MAX_LENGTH_REACHED,
    ERROR_DETECTED
} stop_reason_t;

inline std::string stop_reason_to_string(stop_reason_t reason){
    switch (reason){
        case EOT_DETECTED:
            return "stop";
        case MAX_LENGTH_REACHED:
            return "length";
        case ERROR_DETECTED:
            return "error";
        default:
            return "UNKNOWN";
    }
}

typedef struct {
    int prompt_tokens;
    int generated_tokens;
    uint64_t total_duration; // in nanoseconds
    uint64_t load_duration; // in nanoseconds
    uint64_t prefill_duration; // in nanoseconds
    uint64_t decoding_duration; // in nanoseconds
    stop_reason_t stop_reason;
} chat_meta_info;

/// \brief chat_bot class
/// \note This is a class for the chat_bot
class chat_bot {
private:
    std::unique_ptr<causal_lm> lm_engine = nullptr;
    std::unique_ptr<Tokenizer> tokenizer = nullptr;
    std::unique_ptr<Sampler> sampler = nullptr;
    std::unique_ptr<Q4NX> q4nx = nullptr;
    bool is_model_loaded = false;
    std::string model_path = "";
    std::string current_model = "Llama-3.2-1B-Instruct";
    bool is_think_model = false;
    bool is_think_toggleable = false;
    bool enable_think = false;
    std::vector<int> token_history;
    std::unique_ptr<npu_manager> npu = nullptr;

    uint32_t MAX_L = 0;
    int device_id = 0;
    int last_token = -1;
    uint32_t total_tokens = 0;
    std::unique_ptr<LM_Config> lm_config = nullptr;

    typedef enum{
        PREFILL_TIME,
        DECODING_TIME,
        SAMPLING_TIME,
        TKOEN_ENCODE_TIME,
        TKOEN_DECODE_TIME,
        TTFT_TIME,
        TOTAL_TIME,
        PROFILER_TYPE_NUM
    } profiler_type;
    std::vector<profiler> profiler_list;

    time_utils::time_with_unit last_prefill_time;

public:
    
    chat_bot(unsigned int device_id);

    /// \brief Clear the context
    void clear_context();

    /// \brief Insert the tokens
    /// \param tokens the tokens
    /// \param is_system_prompt the is system prompt
    void insert(chat_meta_info& meta_info, std::vector<int>& tokens, bool is_system_prompt = false);

    /// \brief Generate the tokens
    /// \param os the output stream
    /// \return the tokens
    std::string generate(chat_meta_info& meta_info, int length_limit, std::ostream& os = std::cout);

    /// \brief Generate the tokens with prompt
    std::string generate_with_prompt(chat_meta_info& meta_info, std::vector<int>& tokens, int length_limit, std::ostream& os = std::cout);

    /// \brief Get the current context length
    /// \return the current context length
    int get_current_context_length();

    /// \brief Load the model
    /// \param model_path the model path
    /// \param model_info the model info
    void load_model(std::string model_path, json model_info);

    /// \brief Average the embeddings
    /// \param tokens the tokens
    /// \return the average embeddings
    std::vector<int> average_pool_embeddings(std::vector<int>& tokens);

    /// \brief Set the sampler
    /// \param sampler_config the sampler config
    void set_sampler(sampler_config& sampler_config);

    /// \brief Set the max length
    /// \param MAX_L the max length 
    void set_max_length(unsigned int MAX_L);

    /// \brief Get the current model
    /// \return the current model
    std::string get_current_model() const { return current_model; }
    
    /// \brief Show the model info
    /// \return the model info
    std::string show_model_info();

    /// \brief Show the profile
    /// \return the profile
    std::string show_profile();

    /// \brief Get the history
    /// \return the history
    std::pair<std::string, std::vector<int>> get_history();

    /// \brief Get the history string
    /// \return the history string
    std::string get_history_string();

    /// \brief Verbose
    void verbose();

    /// \brief Tokenize the text
    /// \param text the text
    /// \param apply_chat_template the apply chat template
    /// \param role the role
    /// \param add_generation_prompt the add generation prompt
    /// \return the tokens
    std::vector<int> tokenize(std::string& text, bool apply_chat_template, std::string role, bool add_generation_prompt);

    /// \brief Tokenize the messages
    /// \param messages the messages, chat template is applied
    /// \param add_generation_prompt the add generation prompt
    /// \return the chat template
    std::vector<int> tokenize(nlohmann::ordered_json& messages, bool add_generation_prompt);

    /// \brief Decode the tokens
    /// \param tokens the tokens
    /// \return the decoded text
    std::string decode(std::vector<int>& tokens);

    /// \brief Set the topk
    /// \param topk the topk
    void set_topk(int topk);

    /// \brief Set the topp
    /// \param topp the topp
    void set_topp(float topp);

    /// \brief Set the temperature
    /// \param temperature the temperature
    void set_temperature(float temperature);

    /// \brief Set the repetition penalty
    /// \param repetition_penalty the repetition penalty
    void set_repetition_penalty(float repetition_penalty);

    /// \brief Set the frequency penalty
    /// \param frequency_penalty the frequency penalty
    void set_frequency_penalty(float frequency_penalty);

    /// \brief Set the frequency penalty window
    /// \param frequency_penalty_window the frequency penalty window
    void set_frequency_penalty_window(int frequency_penalty_window);

    /// \brief Start the ttft timer
    /// \return the ttft timer
    void start_ttft_timer();

    /// \brief Stop the ttft timer
    /// \return the ttft timer
    void stop_ttft_timer();

    /// \brief Reset the total timer
    /// \return the total timer
    void reset_total_timer();

    /// \brief Start the total timer
    /// \return the total timer
    void start_total_timer();

    /// \brief Stop the total timer
    /// \return the total timer
    void stop_total_timer();

    /// \brief Set the user system prompt
    /// \param user_system_prompt the user system prompt
    void set_user_system_prompt(const std::string& user_system_prompt);

    /// \brief Toggle the enable think
    /// \param enable_think the enable think
    void toggle_enable_think();

    /// \brief set think
    /// \param enable_think the enable think
    void set_enable_think(bool enable_think);

};