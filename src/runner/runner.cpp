/*
*  Copyright (c) 2025 by Contributors
*  \file runner.cpp
*  \brief Runner implementation for interactive model execution
*  \author FastFlowLM Team
*  \date 2025-06-24
*  \version 0.1.0
*/
#include "runner.hpp"
#include <iostream>
#include <sstream>
#include <filesystem>
#include <map>
#include <iomanip>
#include <fstream>
#include <algorithm>

/// \brief Command map for command line input
std::map<std::string, runner_cmd_t> cmd_map = {
    {"/set", CMD_SET},
    {"/show", CMD_SHOW},
    {"/load", CMD_LOAD},
    {"/save", CMD_SAVE},
    {"/clear", CMD_CLEAR},
    {"/bye", CMD_BYE},
    {"/pull", CMD_PULL},
    {"/?", CMD_HELP},
    {"/? shortcuts", CMD_HELP_SHOTCUT},
};

/// \brief Constructor
/// \param supported_models - the list of supported models
/// \param downloader - the downloader for the models
/// \param tag - the tag of the model to load
Runner::Runner(model_list& supported_models, ModelDownloader& downloader, std::string& tag) 
    : supported_models(supported_models), downloader(downloader), tag(tag) {
    this->chat_engine = std::make_unique<chat_bot>(0);
    if (!this->downloader.is_model_downloaded(this->tag)) {
        this->downloader.pull_model(this->tag);
    }
    nlohmann::json model_info = this->supported_models.get_model_info(this->tag);
    this->chat_engine->load_model(this->supported_models.get_model_path(this->tag), model_info);
    this->generate_limit = -1;
}





/// \brief Run the runner
void Runner::run() {
    
    chat_meta_info meta_info;
    bool verbose = false;
    this->system_prompt = "";
    this->chat_engine->set_user_system_prompt(this->system_prompt);
    std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8conv;
    wstream_buf obuf(std::cout);
    std::ostream ostream(&obuf);
    header_print("FLM", "Type /? for help");
    int empty_line_count = 0;
    while (true) {
        std::string input = cli.get_interactive_input();
        
        if (input.empty()) {
            empty_line_count++;
            if (empty_line_count > 2) {
                header_print("FLM", "Type /? for help");
                empty_line_count = 0;
            }
            continue;
        }

        // Convert line (UTF-8 bytes) → wide string (Unicode codepoints)
        std::wstring winput = utf8conv.from_bytes(input);

        // Split on *any* Unicode whitespace
        std::wistringstream wiss(winput);
        std::wstring wtoken;
        std::vector<std::string> input_list;
        while (wiss >> wtoken) {
            // Convert each token back → UTF-8 bytes
            input_list.push_back(utf8conv.to_bytes(wtoken));
        }

        // now input_list contains your UTF-8 tokens
        if (input_list.empty()) {
            continue;
        }
        
        // For commands, we only need to check the first token
        std::string first_token = input_list[0];
        
        // Check if this is a command (starts with /)
        bool is_command = (first_token[0] == '/');
        
        if (is_command) {
            if (first_token == "/bye") {
                break;
            }
            else if (first_token == "/clear") {
                this->cmd_clear(input_list);
            }
            else if (first_token == "/status") {
                this->cmd_status(input_list);
            }
            else if (first_token == "/load") {
                this->cmd_load(input_list);
            }
            else if (first_token == "/save") {
                this->cmd_save(input_list);
            }
            else if (first_token == "/show") {
                this->cmd_show(input_list);
            }
            else if (first_token == "/set") {
                this->cmd_set(input_list);
            }
            else if (first_token == "/think") {
                this->chat_engine->toggle_enable_think();
            }
            else if (first_token == "/help") {
                this->cmd_help(input_list);
            }
            else if (first_token == "/?") {
                this->cmd_help(input_list);
            }
            else if (first_token == "/verbose") {
                verbose = !verbose;
            }
            else if (first_token == "/history") {
                std::pair<std::string, std::vector<int>> history = this->chat_engine->get_history();
                std::cout << "History: " << std::endl;
                std::cout << history.first << std::endl;
                std::cout << "Tokens: " << history.second.size() << std::endl;
                for (int i = 0; i < history.second.size(); i++) {
                    std::cout << std::dec << history.second[i] << " ";
                }
                std::cout << std::endl;
            }
            else if (first_token == "/pull") {
                std::string model_name = input_list[1];
                this->downloader.pull_model(model_name);
            }
        } else {
            // This is a regular message, not a command
            std::cout << std::endl;  // Add newline before AI response
            this->chat_engine->start_ttft_timer();
            this->chat_engine->start_total_timer();
            std::vector<int> user_tokens = this->chat_engine->tokenize(input, true, "user", true);
            this->chat_engine->insert(meta_info, user_tokens, false);
            this->chat_engine->stop_ttft_timer();
            this->chat_engine->generate(meta_info, this->generate_limit, ostream);
            this->chat_engine->stop_total_timer();
            std::cout << std::endl;
            if (verbose) {
                this->chat_engine->verbose();
            }
        }
    }
}

/// \brief Clear the context
/// \param input_list, std::vector<std::string>
void Runner::cmd_clear(std::vector<std::string>& input_list) {
    this->chat_engine->clear_context();
}

/// \brief Show the status
/// \param input_list, std::vector<std::string>
void Runner::cmd_status(std::vector<std::string>& input_list) {
    std::cout << this->chat_engine->show_profile() << std::endl;
}

/// \brief Load a model
/// \param input_list, std::vector<std::string>
void Runner::cmd_load(std::vector<std::string>& input_list) {
    std::string model_name = input_list[1];
    this->tag = model_name;
    if (!this->downloader.is_model_downloaded(this->tag)) {
        this->downloader.pull_model(this->tag);
    }
    nlohmann::json model_info = this->supported_models.get_model_info(this->tag);
    this->chat_engine->load_model(this->supported_models.get_model_path(this->tag), model_info);
    this->chat_engine->set_user_system_prompt(this->system_prompt);
}

/// \brief Save the history
/// \param input_list, std::vector<std::string>
void Runner::cmd_save(std::vector<std::string>& input_list) {
    std::pair<std::string, std::vector<int>> history = this->chat_engine->get_history();
    
    // Get the Documents directory and create the history directory
    std::string documents_dir = utils::get_user_documents_directory();
    std::string history_dir = documents_dir + "/flm/history";
    
    // Create the history directory if it doesn't exist
    if (!std::filesystem::exists(history_dir)) {
        std::filesystem::create_directories(history_dir);
    }
    
    // save file to history_hh_mm_mm_dd_yyyy.txt
    // 1) get current date
    std::time_t t = std::time(nullptr);
    std::tm tm = *std::localtime(&t);

    std::ostringstream date_ss;
    date_ss << std::setw(2) << std::setfill('0') << tm.tm_hour
            << '_'
            << std::setw(2) << std::setfill('0') << tm.tm_min
            << '_'
            << std::setw(2) << std::setfill('0') << tm.tm_mon + 1
            << '_'
            << std::setw(2) << std::setfill('0') << tm.tm_mday
            << '_'
            << (tm.tm_year + 1900);
    std::string date_str = date_ss.str();

    // 2) build filename in the history directory
    std::string file_name = history_dir + "/history_" + date_str + ".txt";
    std::ofstream file(file_name);
    if (file.is_open()) {
        file << history.first << std::endl;
        file.close();
        std::cout << "History saved to " << file_name << std::endl;
    }
    else {
        std::cout << "Failed to open file: " << file_name << std::endl;
    }
}

/// \brief Show the model information
/// \param input_list, std::vector<std::string>
void Runner::cmd_show(std::vector<std::string>& input_list) {
    std::cout << this->chat_engine->show_model_info() << std::endl;
}

/// \brief Set the model parameters
/// \param input_list, std::vector<std::string>
void Runner::cmd_set(std::vector<std::string>& input_list) {
    // remove the /set
    if (input_list.size() < 3){
        std::cout << "Usage: /set [context] [value]" << std::endl;
        std::cout << "Available parameters: " << std::endl;
        std::cout << "  /set topk [value] - set the top-k" << std::endl;
        std::cout << "  /set topp [value] - set the top-p" << std::endl;
        std::cout << "  /set temperature [value] - set the temperature" << std::endl;
        std::cout << "  /set repetition_penalty [value] - set the repetition penalty" << std::endl;
        std::cout << "  /set frequency_penalty [value] - set the frequency penalty" << std::endl;
        std::cout << "  /set system_prompt [value] - set the system prompt" << std::endl;
        std::cout << "  /set context_length [value] - set the context length" << std::endl;
        std::cout << "  /set generate_limit [value] - set the generate limit" << std::endl;
        return;
    }
    
    std::string set_context = input_list[1];
    
    // Handle system_prompt specially since it can be multi-line
    if (set_context == "system_prompt") {
        // Reconstruct the original input to get the full system prompt
        std::string full_input;
        for (size_t i = 2; i < input_list.size(); i++) {
            if (i > 2) full_input += " ";
            full_input += input_list[i];
        }
        this->system_prompt = full_input;
        this->cmd_clear(input_list);
        this->chat_engine->set_user_system_prompt(this->system_prompt);
        return;
    }
    
    // For other parameters, use the third token as the value
    if (input_list.size() < 3) {
        std::cout << "Usage: /set [context] [value]" << std::endl;
        return;
    }
    
    std::string set_value = input_list[2];

    if (set_context == "topk"){
        this->chat_engine->set_topk(std::stoi(set_value));
    }
    else if (set_context == "topp"){
        this->chat_engine->set_topp(std::stof(set_value));
    }
    else if (set_context == "temperature"){
        this->chat_engine->set_temperature(std::stof(set_value));
    }
    else if (set_context == "repetition_penalty"){
        this->chat_engine->set_repetition_penalty(std::stof(set_value));
    }
    else if (set_context == "frequency_penalty"){
        this->chat_engine->set_frequency_penalty(std::stof(set_value));
    }
    else if (set_context == "context_length"){
        this->chat_engine->set_max_length(std::stoi(set_value));
    }
    else if (set_context == "generate_limit"){
        this->generate_limit = std::stoi(set_value);
    }
    else{
        std::cout << "Invalid context: " << set_context << std::endl;
        std::cout << "Available parameters: " << std::endl;
        std::cout << "  /set context_length [value] - set the context length" << std::endl;
        std::cout << "  /set topk [value] - set the top-k" << std::endl;
        std::cout << "  /set topp [value] - set the top-p" << std::endl;
        std::cout << "  /set temperature [value] - set the temperature" << std::endl;
        std::cout << "  /set repetition_penalty [value] - set the repetition penalty" << std::endl;
        std::cout << "  /set frequency_penalty [value] - set the frequency penalty" << std::endl;   
        std::cout << "  /set system_prompt [value] - set the system prompt" << std::endl;
    }
}

/// \brief Show the help
/// \param input_list, std::vector<std::string>
void Runner::cmd_help(std::vector<std::string>& input_list) {
    std::cout << "Available commands:" << std::endl;
    std::cout << "  /show - show the model information" << std::endl;
    std::cout << "  /load [model_name] - load a model" << std::endl;
    std::cout << "  /save - save the history" << std::endl;
    std::cout << "  /clear - clear the context" << std::endl;
    std::cout << "  /status - show the history" << std::endl;
    std::cout << "  /history - show the history" << std::endl;
    std::cout << "  /verbose - toggle the verbose" << std::endl;
    std::cout << "  /think - toggle the think" << std::endl;
    std::cout << "  /set [context] [value] - set the context" << std::endl;
    std::cout << "  /pull [model_name] - pull a model" << std::endl;
    std::cout << "  /list - list all the models" << std::endl;
    std::cout << "  /remove [model_name] - remove a model" << std::endl;
    std::cout << "  /bye - exit the program" << std::endl;
    std::cout << "  /? - show this help" << std::endl;
    std::cout << std::endl;
    std::cout << "Interactive input:" << std::endl;
    std::cout << "  - Press Enter to submit single-line input" << std::endl;
    std::cout << "  - Paste multi-line text and it will be detected automatically" << std::endl;
    std::cout << "  - Use '\\' at the end of a line to explicitly continue on next line" << std::endl;
    std::cout << "  - Commands (starting with /) are processed immediately" << std::endl;
}

/// \brief Show the help shotcut
/// \param input_list, std::vector<std::string>
void Runner::cmd_help_shotcut(std::vector<std::string>& input_list) {
    std::cout << "Help shotcut" << std::endl;
}


