/*
*  Copyright (c) 2025 by Contributors
*  \file runner.cpp
*  \brief Runner implementation for interactive model execution
*  \author FastFlowLM Team
*  \date 2025-06-24
*  \version 0.1.0
*/
#include "runner.hpp"
#include <chrono>
#include <thread>
#include <iostream>
#include <conio.h>
#include <sstream>
#include <windows.h>
#include <fcntl.h>
#include <io.h>

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
    this->chat_engine = std::make_unique<chat_bot>(131072);
    if (!this->downloader.is_model_downloaded(this->tag)) {
        this->downloader.pull_model(this->tag);
    }
    nlohmann::json model_info = this->supported_models.get_model_info(this->tag);
    this->chat_engine->load_model(this->supported_models.get_model_path(this->tag), model_info["default_context_length"]);
    
    // Set console to UTF-8 mode for proper Unicode handling
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    // Note: We don't use _setmode here as it can interfere with _getch()
    // The UTF-8 console mode should be sufficient for proper Unicode display
}

/// \brief Get interactive input
/// \return the input string
std::string Runner::get_interactive_input() {
    std::string full_input;
    std::string line;
    bool is_first_line = true;
    bool continue_line = true;

    while (continue_line) {
        // Print appropriate prompt
        std::cout << (is_first_line ? ">>> " : "... ");

        // Use _getch() to detect individual key presses
        std::string current_line;
        bool waiting_for_more_input = false;
        
        while (true) {
            int ch = _getch();
            
            // Handle special keys
            if (ch == 0 || ch == 224) {
                // Extended key code - read the next byte
                int ext_ch = _getch();
                // Handle arrow keys, function keys, etc. if needed
                continue;
            }
            
            // Handle Enter key
            if (ch == '\r' || ch == '\n') {
                std::cout << std::endl;
                if (current_line.empty()) {
                    // Empty line - check if this is end of input
                    if (full_input.empty()) {
                        return "";  // no input at all → exit
                    }
                    continue_line = false;  // partial input + empty line = finish
                    break;
                }
                // Line with content + Enter - check if more input is coming
                waiting_for_more_input = true;
                break;  // Break out of inner loop to check for paste
            }
            
            // Handle backspace
            if (ch == '\b') {
                if (!current_line.empty()) {
                    // Handle UTF-8 backspace properly
                    size_t last_char_size = 1;
                    if (current_line.size() >= 2) {
                        unsigned char last_byte = static_cast<unsigned char>(current_line.back());
                        if ((last_byte & 0xC0) == 0x80) {
                            // This is a continuation byte, find the start of the character
                            size_t pos = current_line.size() - 1;
                            while (pos > 0 && (static_cast<unsigned char>(current_line[pos-1]) & 0xC0) == 0x80) {
                                pos--;
                            }
                            if (pos > 0 && (static_cast<unsigned char>(current_line[pos-1]) & 0xC0) != 0x80) {
                                last_char_size = current_line.size() - pos;
                            }
                        }
                    }
                    
                    // Remove the character from the line
                    for (size_t i = 0; i < last_char_size; i++) {
                        if (!current_line.empty()) {
                            current_line.pop_back();
                        }
                    }
                    
                    // Move cursor back and clear the character(s)
                    for (size_t i = 0; i < last_char_size; i++) {
                        std::cout << "\b \b";
                    }
                }
                continue;
            }
            
            // Handle Ctrl+C
            if (ch == 3) {
                std::cout << "^C\n";
                return "/bye";
            }
            
            // Handle Ctrl+D (EOF equivalent on Windows)
            if (ch == 4) {
                if (full_input.empty()) {
                    return "/bye";
                }
                continue_line = false;
                break;
            }
            
            // Handle UTF-8 characters
            if (ch >= 0) {
                // Start of a UTF-8 sequence
                std::string utf8_char;
                utf8_char += static_cast<char>(ch);
                
                // Check if this is a multi-byte UTF-8 sequence
                if ((ch & 0x80) != 0) {
                    // Determine how many continuation bytes we need
                    int continuation_bytes = 0;
                    if ((ch & 0xE0) == 0xC0) continuation_bytes = 1;      // 2-byte sequence
                    else if ((ch & 0xF0) == 0xE0) continuation_bytes = 2; // 3-byte sequence
                    else if ((ch & 0xF8) == 0xF0) continuation_bytes = 3; // 4-byte sequence
                    
                    // Read continuation bytes
                    for (int i = 0; i < continuation_bytes; i++) {
                        int cont_ch = _getch();
                        if (cont_ch >= 0) {
                            utf8_char += static_cast<char>(cont_ch);
                        }
                    }
                }
                
                // Add the complete UTF-8 character to the line
                current_line += utf8_char;
                std::cout << utf8_char;
            }
        }
        
        // If this is the first line and it's empty, continue
        if (is_first_line && current_line.empty()) {
            continue;
        }
        
        // Check if this is a command (starts with /)
        if (!current_line.empty() && current_line[0] == '/') {
            return current_line;  // Commands are always single-line
        }
        
        // Append this line
        if (!is_first_line) full_input += "\n";
        full_input += current_line;
        is_first_line = false;
        
        // Check for backslash continuation
        if (!current_line.empty() && current_line.back() == '\\') {
            full_input.pop_back();  // drop the '\'
            continue;  // go read another line
        }
        
        // If we were waiting for more input, check if it's a paste
        if (waiting_for_more_input) {
            // wait for 30ms
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
            if (_kbhit()) {
                // More input is coming - this is a paste operation
                // Continue to the next iteration to read more lines
                // Ensure we're no longer on the first line
                is_first_line = false;
                continue;
            } else {
                // No more input - this was just a single Enter
                continue_line = false;
                break;
            }
        }
        
        // If we get here and continue_line is still true, we need to break
        if (!continue_line) {
            break;
        }
    }

    return full_input;
}

/// \brief Run the runner
void Runner::run() {
    bool verbose = false;
    std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8conv;
    wstream_buf obuf(std::cout);
    std::ostream ostream(&obuf);
    
    while (true) {
        std::string input = get_interactive_input();
        
        if (input.empty()) {
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
            std::vector<int> tokens = this->chat_engine->tokenize(input);
            std::vector<int> prompts = this->chat_engine->apply_chat_template(tokens, chat_bot::USER, true);
            this->chat_engine->insert(prompts);
            this->chat_engine->stop_ttft_timer();
            this->chat_engine->generate(ostream);
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
        this->downloader.model_not_found(this->tag);
        exit(1);
    }
    nlohmann::json model_info = this->supported_models.get_model_info(this->tag);
    this->chat_engine->load_model(this->supported_models.get_model_path(this->tag), model_info["default_context_length"]);
}

/// \brief Save the history
/// \param input_list, std::vector<std::string>
void Runner::cmd_save(std::vector<std::string>& input_list) {
    std::pair<std::string, std::vector<int>> history = this->chat_engine->get_history();
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

    // 2) build filename
    std::string file_name = "history_" + date_str + ".txt";
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
        this->chat_engine->set_system_prompt(full_input);
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