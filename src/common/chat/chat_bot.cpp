/// \file chat_bot.cpp
/// \brief chat bot class
/// \author FastFlowLM Team
/// \date 2025-08-05
/// \version 0.9.2
/// \note This is a header file for the chat bot class
#pragma once
#include "chat/chat_bot.hpp"

chat_bot::chat_bot(unsigned int device_id){
    this->MAX_L = 4096;
    this->device_id = device_id;
    this->total_tokens = 0;
    this->profiler_list.resize(PROFILER_TYPE_NUM);
    for (size_t i = 0; i < PROFILER_TYPE_NUM; i++){
        this->profiler_list[i] = profiler();
    }
    this->last_prefill_time = {0, "us"};
    this->token_history.reserve(MAX_L);
}

/// \brief Load the model
/// \param model_path the path to the model
/// \param model_info the model info
/// \note The function will load the model
void chat_bot::load_model(std::string model_path, json model_info){
    // default models
    static const std::vector<std::pair<std::string, std::string>> default_models = {
        {"Llama-3.2-1B-q4nx", "1B"}, // 1B
        {"Llama-3.2-3B-q4nx", "3B"}, // 3B
        {"Llama-3.1-8B-q4nx", "8B"}, // 8B
        {"deepseek-distill-llama-8B-q4nx", "ds8B"}, // ds8B
    };
    if (this->is_model_loaded && this->model_path != model_path){
        header_print("FLM", "Unloading model " << this->model_path << "...");
        this->lm_engine.reset();
        this->lm_config.reset();
        this->q4nx.reset();
        this->tokenizer.reset();
        this->is_model_loaded = false;
    }
    if (this->is_model_loaded && this->model_path == model_path){
        header_print("FLM", "Model already loaded: " << this->model_path);
        return;
    }
    JSON_GET(this->is_think_model, model_info["details"], "think", false, bool);
    JSON_GET(this->is_think_toggleable, model_info["details"], "think_toggleable", false, bool);
    this->enable_think = this->is_think_model;
    this->model_path = model_path;
    for (const auto& model : default_models){
        if (model.second == model_path){
            this->model_path = "models/" + model.first;
            break;
        }
    }
    header_print("FLM", "Loading model: " << this->model_path);
    this->lm_config = std::make_unique<LM_Config>();
    this->lm_config->from_pretrained(this->model_path);
    this->npu = std::make_unique<npu_manager>(npu_device::device_npu2, device_id);
    this->MAX_L = model_info["default_context_length"];
    this->q4nx = std::make_unique<Q4NX>(this->model_path);
    if (this->lm_config->model_type == "llama"){
        this->lm_engine = std::make_unique<llama_npu>(*this->lm_config, this->npu.get(), this->MAX_L);
    }
    else if (this->lm_config->model_type == "qwen3"){
        this->lm_engine = std::make_unique<qwen_npu>(*this->lm_config, this->npu.get(), this->MAX_L);
    }
    else if (this->lm_config->model_type == "gemma3_text"){
        this->lm_engine = std::make_unique<gemma_npu>(*this->lm_config, this->npu.get(), this->MAX_L);
    }
    else {
        header_print("WARNING", "Model type not supported: " << this->lm_config->model_type);
        exit(1);
    }
    
    this->lm_engine->load_weights(*this->q4nx);
    
    //free the q4nx
    this->q4nx.reset();
    this->is_model_loaded = true;

    this->token_history.clear();
    this->token_history.reserve(this->MAX_L);
    this->tokenizer = std::make_unique<Tokenizer>(this->model_path);

    this->lm_engine->clear_context();
    this->last_token = -1;
    this->total_tokens = 0;

    this->sampler.reset();

    sampler_config config;
    config.rep_penalty = 0.1;
    config.temperature = 0.6;
    config.top_p = 0.95;
    config.top_k = 5;
    config.rep_penalty_window = 64;
    config.freq_penalty = 0.1;
    config.freq_penalty_window = 256;
    config.freq_penalty_decay = 0.995;
    this->set_sampler(config);
    for (size_t i = 0; i < PROFILER_TYPE_NUM; i++){
        this->profiler_list[i].reset();
    }
}

/// \brief Set the sampler
/// \param sampler_config the sampler config
/// \note The function will set the sampler
/// \note The function will reset the sampler
/// \note The function will set the sampler config
void chat_bot::set_sampler(sampler_config& sampler_config){
    if (this->sampler != nullptr){
        this->sampler.reset();
    }
    this->sampler = std::make_unique<Sampler>(this->lm_config->vocab_size, sampler_config);
}

/// \brief Set the max length
/// \param MAX_L the max length
/// \note The function will set the max length
/// \note The function will update the max length
void chat_bot::set_max_length(unsigned int MAX_L){
    this->MAX_L = MAX_L;
    if (this->lm_engine != nullptr){
        this->lm_engine->update_max_length(MAX_L);
    }
}

/// \brief Insert the tokens
/// \param tokens the tokens
/// \param is_system_prompt the is system prompt
/// \note The function will insert the tokens
/// \note The function will check if the tokens are valid
bool chat_bot::insert(chat_meta_info& meta_info, std::vector<int>& tokens, bool is_system_prompt){
    assert(this->lm_engine != nullptr);
    assert(this->lm_config != nullptr);
    assert(this->tokenizer != nullptr);
    assert(this->sampler != nullptr);
    if (this->total_tokens + tokens.size() >= this->MAX_L){
        header_print("WARNING", "Max length reached, stopping prefilling...");
        return false;
    }
    for (int token : tokens){
        this->token_history.push_back(token);
    }
    buffer<bf16> y;

    auto prefill_start_time = this->profiler_list[PREFILL_TIME].start();
    y = this->lm_engine->prefill(tokens);
    auto prefill_end_time = this->profiler_list[PREFILL_TIME].stop(tokens.size());
    meta_info.prefill_duration = (uint64_t)time_utils::duration_ns(prefill_start_time, prefill_end_time).first;
    meta_info.prompt_tokens = tokens.size();
    this->total_tokens += tokens.size() + 1;
    if (this->total_tokens >= this->MAX_L){
        header_print("WARNING", "Max length reached, stopping prefilling...");
    }
    this->profiler_list[SAMPLING_TIME].start();
    this->last_token = this->sampler->sample(y);
    this->profiler_list[SAMPLING_TIME].stop(1);
    if (is_system_prompt){
        this->profiler_list[PREFILL_TIME].reset();
        this->profiler_list[TKOEN_ENCODE_TIME].reset();
        this->profiler_list[TKOEN_DECODE_TIME].reset();
        this->profiler_list[SAMPLING_TIME].reset();
        this->profiler_list[DECODING_TIME].reset();
        this->profiler_list[TOTAL_TIME].reset();
    }
    return true;
}

/// \brief Generate the tokens
/// \param meta_info the meta info
/// \param length_limit the length limit, -1 means no limit
/// \param os the output stream
/// \note The function will generate the tokens
/// \note The function will check if the tokens are valid
/// \note The function will check if the max length is reached
/// \note The function will check if the last token is valid
std::string chat_bot::generate(chat_meta_info& meta_info, int length_limit, std::ostream& os){
    assert(this->lm_engine != nullptr);
    assert(this->lm_config != nullptr);
    assert(this->tokenizer != nullptr);
    assert(this->sampler != nullptr);
    std::vector<int> sampled_tokens;
    if (length_limit > 0){
        sampled_tokens.reserve(length_limit);
    }
    else{
        sampled_tokens.reserve(4096);
    }
    assert(this->last_token != -1);

    std::string result;
    meta_info.generated_tokens = 1;
    if (this->enable_think){
        int think_marker_id = this->tokenizer->get_think_marker_id();
        if (think_marker_id != -1){
            std::string think_result = this->tokenizer->run_time_decoder(this->tokenizer->get_think_marker_id());
            result += think_result;
            os << think_result << std::flush;
        }
    }

    stop_reason_t reason = EOT_DETECTED;
    int last_sampled_token = this->last_token;
    this->token_history.push_back(this->last_token);
    auto decoding_start_time = time_utils::now();
    if (this->tokenizer->is_normal_token(last_sampled_token) && last_sampled_token != -1){
        std::string token_str = this->tokenizer->run_time_decoder(last_sampled_token);
        result += token_str;
        os << token_str << std::flush;

    }
    if (this->tokenizer->is_eos(last_sampled_token)){
        return result;
    }
    this->profiler_list[TKOEN_DECODE_TIME].stop(1);
    if (this->total_tokens >= this->MAX_L){
        header_print("WARNING", "Max length reached, stopping generation...");
        reason = MAX_LENGTH_REACHED;
        return result;
    }
    while (this->total_tokens < this->MAX_L){
        this->profiler_list[DECODING_TIME].start();
        buffer<bf16> y = this->lm_engine->forward(last_sampled_token);
        this->profiler_list[DECODING_TIME].stop(1);

        this->profiler_list[SAMPLING_TIME].start();
        int sampled_token = this->sampler->sample(y);
        this->profiler_list[SAMPLING_TIME].stop(1);
        this->total_tokens++;
        last_sampled_token = sampled_token;

        this->profiler_list[TKOEN_DECODE_TIME].start();
        this->profiler_list[TKOEN_DECODE_TIME].stop(1);
        if (this->tokenizer->is_normal_token(sampled_token)){ // filter out special tokens
            std::string token_str = this->tokenizer->run_time_decoder(sampled_token);
            os << token_str << std::flush;
            result += token_str;
        }
        this->token_history.push_back(sampled_token);
        if (this->tokenizer->is_eos(sampled_token)){
            this->lm_engine->forward(last_sampled_token);
            break;
        }
        meta_info.generated_tokens++;
        if ((length_limit > 0) && (meta_info.generated_tokens >= length_limit)){
            reason = MAX_LENGTH_REACHED;
            break;
        }
    }
    auto decoding_end_time = time_utils::now();
    meta_info.decoding_duration = (uint64_t)time_utils::duration_ns(decoding_start_time, decoding_end_time).first;
    meta_info.stop_reason = reason;
    if (this->total_tokens >= this->MAX_L){
        header_print("WARNING", "Max length reached, stopping generation...");
    }
    return result;
}

/// \brief Generate the tokens with prompt
/// \param meta_info the meta info
/// \param tokens the tokens
/// \param length_limit the length limit, -1 means no limit
/// \param os the output stream
/// \note The function will generate the tokens
/// \note The function will insert the tokens
/// \note The function will check if the tokens are valid
std::string chat_bot::generate_with_prompt(chat_meta_info& meta_info, std::vector<int>& tokens, int length_limit, std::ostream& os){
    if (!this->insert(meta_info, tokens)){
        return "";
    }
    std::string result = this->generate(meta_info, length_limit, os);
    return result;
}

/// \brief Clear the context
/// \note The function will clear the context
/// \note The function will reset the total tokens
/// \note The function will reset the last token
/// \note The function will clear the context
/// \note The function will reset the total tokens
void chat_bot::clear_context(){
    this->total_tokens = 0;
    this->last_token = -1;
    this->token_history.clear();
    this->lm_engine->clear_context();
    this->total_tokens = 0;
    this->sampler->reset_penalties();
    for (size_t i = 0; i < PROFILER_TYPE_NUM; i++){
        this->profiler_list[i].reset(); 
    }
    this->last_prefill_time = {0, "us"};
}

/// \brief Get the current context length
/// \note The function will get the current context length
/// \note The function will return the current context length
int chat_bot::get_current_context_length(){
    return this->total_tokens;
}

/// \brief Show the model info
/// \note The function will show the model info
/// \note The function will return the model info
std::string chat_bot::show_model_info(){
    try{
        std::string ss = this->lm_config->_str();
        return ss;
    }
    catch(const std::exception& e){
        return "Error showing model info: " + std::string(e.what());
    }
}

/// \brief Show the profile
/// \note The function will show the profile
/// \note The function will return the profile
std::string chat_bot::show_profile(){
    std::stringstream ss;
    int total_tokens = this->lm_engine->get_current_context_length();
    time_utils::time_with_unit time = this->profiler_list[TOTAL_TIME].get_total_time();
    ss << "  Statistics:" << std::endl;
    ss << "    Total tokens:        " << this->get_current_context_length() << " (" << total_tokens << ")" << std::endl;
    ss << "    Total time:          " << time.first << " " << time.second << std::endl;
    time = this->profiler_list[DECODING_TIME].get_total_time();
    ss << "    Decoding time:       " << time.first << " " << time.second << std::endl;
    time = this->profiler_list[PREFILL_TIME].get_total_time();
    ss << "    Prefill time:        " << time.first << " " << time.second << std::endl;
    time = this->profiler_list[SAMPLING_TIME].get_total_time();
    ss << "    Sampling time:       " << time.first << " " << time.second << std::endl;
    time = this->profiler_list[TKOEN_ENCODE_TIME].get_total_time();
    ss << "    Token encoding time: " << time.first << " " << time.second << std::endl;
    time = this->profiler_list[TKOEN_DECODE_TIME].get_total_time();
    ss << "    Token decoding time: " << time.first << " " << time.second << std::endl;
    ss << "    Average decoding speed:       " << this->profiler_list[DECODING_TIME].get_average_speed() << " tokens/s" << std::endl;
    ss << "    Average prefill  speed:       " << this->profiler_list[PREFILL_TIME].get_average_speed() << " tokens/s" << std::endl;
    ss << "    Average sampling speed:       " << this->profiler_list[SAMPLING_TIME].get_average_speed() << " tokens/s" << std::endl;
    ss << "    Average token encoding speed: " << this->profiler_list[TKOEN_ENCODE_TIME].get_average_speed() << " tokens/s" << std::endl;
    ss << "    Average token decoding speed: " << this->profiler_list[TKOEN_DECODE_TIME].get_average_speed() << " tokens/s" << std::endl;
    ss << "    Average overall speed:        " << this->profiler_list[TOTAL_TIME].get_average_speed() << " tokens/s" << std::endl;

    return ss.str();
}

/// \brief Get the history
/// \note The function will get the history
/// \note The function will return the history
std::pair<std::string, std::vector<int>> chat_bot::get_history(){
    std::vector<int> history = this->token_history;
    std::string all_context = this->tokenizer->decode(history);
    return std::make_pair(all_context, history);
}

/// \brief Get the history string
/// \note The function will get the history string
/// \note The function will return the history string
std::string chat_bot::get_history_string(){
    std::vector<int> history = this->token_history;
    std::string all_context = this->tokenizer->decode(history);
    return all_context;
}

/// \brief Verbose
/// \note The function will verbose
/// \note The function will print the verbose
void chat_bot::verbose(){
    std::cout << std::endl;
    int total_tokens = this->get_current_context_length();
    float prefill_speed = this->profiler_list[PREFILL_TIME].get_average_speed();
    float decoding_speed = this->profiler_list[DECODING_TIME].get_average_speed();
    time_utils::time_with_unit ttft_time = this->profiler_list[TTFT_TIME].get_total_time();
    float context_percentage = (float)total_tokens / (float)this->MAX_L * 100;
    std::cout << "Verbose: " << std::endl;
    std::cout << "  Total tokens:        " << total_tokens << " (" << std::fixed << std::setprecision(2) << context_percentage << "%)" << std::endl;
    std::cout << "  TTFT:                " << ttft_time.first << " " << ttft_time.second << std::endl;
    std::cout << "  Prefill speed:       " << std::fixed << std::setprecision(2) << prefill_speed  << " tokens/s" << std::endl;
    std::cout << "  Decoding speed:      " << std::fixed << std::setprecision(2) << decoding_speed << " tokens/s" << std::endl << std::endl;


}

/// \brief Set the top-k
/// \param topk the top-k
/// \note The function will set the top-k
/// \note The function will check if the top-k is valid
void chat_bot::set_topk(int topk){
    if (topk < 1){
        header_print("WARNING", "Top-k must be greater than 0");
        return;
    }
    this->sampler->top_k = topk;
}

/// \brief Set the top-p
/// \param topp the top-p
/// \note The function will set the top-p
/// \note The function will check if the top-p is valid
void chat_bot::set_topp(float topp){
    if (topp < 0.0f || topp > 1.0f){
        header_print("WARNING", "Top-p must be between 0.0 and 1.0");
        return;
    }
    this->sampler->top_p = topp;
}

/// \brief Set the temperature
/// \param temperature the temperature
/// \note The function will set the temperature
/// \note The function will check if the temperature is valid
void chat_bot::set_temperature(float temperature){
    if (temperature < 0.0f){
        header_print("WARNING", "Temperature must be greater than 0.0");
        return;
    }
    this->sampler->temperature = temperature;
}

/// \brief Set the repetition penalty
/// \param repetition_penalty the repetition penalty
/// \note The function will set the repetition penalty
/// \note The function will check if the repetition penalty is valid
void chat_bot::set_repetition_penalty(float repetition_penalty){
    if (repetition_penalty < 0.0f || repetition_penalty > 1.0f){
        header_print("WARNING", "Repetition penalty must be between 0.0 and 1.0");
        return;
    }
    this->sampler->rep_penalty = repetition_penalty;
}

/// \brief Set the frequency penalty
/// \param frequency_
void chat_bot::set_frequency_penalty(float frequency_penalty){
    if (frequency_penalty < 0.0f){
        header_print("WARNING", "Frequency penalty must be greater than 0.0");
        return;
    }
    this->sampler->freq_penalty = frequency_penalty;
}

/// \brief Set the frequency penalty window
/// \param frequency_penalty_window the frequency penalty window
/// \note The function will set the frequency penalty window
/// \note The function will check if the frequency penalty window is valid
void chat_bot::set_frequency_penalty_window(int frequency_penalty_window){
    this->sampler->freq_penalty_window = frequency_penalty_window;
}


/// \brief Tokenize the text
/// \param text the text
/// \note The function will tokenize the text
/// \note The function will check if the text is valid
std::vector<int> chat_bot::tokenize(std::string& text, bool apply_chat_template, std::string role, bool add_generation_prompt){
    this->profiler_list[TKOEN_ENCODE_TIME].start();
    std::string new_text;
    if (apply_chat_template){
        nlohmann::ordered_json messages;
        messages.push_back({{"role", role}, {"content", text}});
        new_text = this->tokenizer->apply_chat_template(messages, add_generation_prompt, this->enable_think);
    }
    else{
        new_text = text;
    }
    std::vector<int> tokens = this->tokenizer->encode(new_text);
    this->profiler_list[TKOEN_ENCODE_TIME].stop(tokens.size());
    return tokens;
}

/// \brief Apply the chat template
/// \param messages the messages
/// \param add_generation_prompt the add generation prompt
/// \return the chat template
std::vector<int> chat_bot::tokenize(nlohmann::ordered_json& messages, bool add_generation_prompt){
    std::string text = this->tokenizer->apply_chat_template(messages, add_generation_prompt, this->enable_think);
    return this->tokenizer->encode(text);
}

/// \brief Decode the tokens
/// \param tokens the tokens
/// \note The function will decode the tokens
/// \note The function will check if the tokens are valid
std::string chat_bot::decode(std::vector<int>& tokens){
    return this->tokenizer->decode(tokens);
}

/// \brief Start the ttft timer
/// \note The function will start the ttft timer
/// \note The function will reset the ttft timer
void chat_bot::start_ttft_timer(){
    this->profiler_list[TTFT_TIME].reset();
    this->profiler_list[TTFT_TIME].start();
}

/// \brief Stop the ttft timer
/// \note The function will stop the ttft timer
/// \note The function will check if the ttft timer is valid
void chat_bot::stop_ttft_timer(){
    this->profiler_list[TTFT_TIME].stop(1);
}

/// \brief Reset the total timer
/// \note The function will reset the total timer
/// \note The function will check if the total timer is valid
void chat_bot::reset_total_timer(){
    this->profiler_list[TOTAL_TIME].reset();
}

/// \brief Start the total timer
/// \note The function will start the total timer
/// \note The function will check if the total timer is valid
void chat_bot::start_total_timer(){
    this->profiler_list[TOTAL_TIME].start();
}

/// \brief Stop the total timer
/// \note The function will stop the total timer
/// \note The function will check if the total timer is valid
void chat_bot::stop_total_timer(){
    this->profiler_list[TOTAL_TIME].stop(this->total_tokens, true);
}

/// \brief Set the user system prompt
/// \param user_system_prompt the user system prompt
/// \note The function will set the user system prompt
/// \note The function will check if the user system prompt is valid
void chat_bot::set_user_system_prompt(const std::string& user_system_prompt){
    this->tokenizer->set_user_system_prompt(user_system_prompt);
}

/// \brief Toggle the enable think
/// \note The function will toggle the enable think
/// \note The function will check if the enable think is valid
void chat_bot::toggle_enable_think(){
    if (this->is_think_toggleable){
        this->enable_think = !this->enable_think;
        header_print("FLM", "Think is " << (this->enable_think ? "enabled" : "disabled"));
    }
    else{
        header_print("FLM", "Think is not toggleable for this model!");
    }
}

/// \brief Set the enable think
/// \param enable_think the enable think
/// \note The function will set the enable think
/// \note The function will check if the enable think is valid
void chat_bot::set_enable_think(bool enable_think){
    if (this->is_think_toggleable){
        this->enable_think = enable_think;
        header_print("FLM", "Think is " << (this->enable_think ? "enabled!" : "disabled!"));
    }
    else{
        header_print("FLM", "Think is not toggleable for this model!");
    }
}