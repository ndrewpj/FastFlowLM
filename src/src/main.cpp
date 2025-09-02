/// \file main.cpp
/// \brief Main entry point for the FLM application
/// \author FastFlowLM Team
/// \date 2025-08-05
/// \version 0.9.7
/// \note This is a source file for the main entry point
#pragma once
#include "runner.hpp"
#include "server.hpp"
#include "model_list.hpp"
#include "model_downloader.hpp"
#include "utils/utils.hpp"
#include "minja/chat-template.hpp"
#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <windows.h>
#include <filesystem>
#include <shlobj.h>
#include <cstdlib>
#include <shellapi.h>


// Global variables
///@brief should_exit is used to control the server thread
std::atomic<bool> should_exit(false);

///@brief get_unicode_command_line_args gets Unicode command line arguments
///@param argc_out reference to store argument count
///@return vector of UTF-8 encoded argument strings
std::vector<std::string> get_unicode_command_line_args(int& argc_out) {
    std::vector<std::string> args;
    
    // Get the Unicode command line
    LPWSTR* szArglist;
    int nArgs;
    
    szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
    if (szArglist == nullptr) {
        argc_out = 0;
        return args;
    }
    
    argc_out = nArgs;
    
    // Convert each argument from wide string to UTF-8
    for (int i = 0; i < nArgs; i++) {
        std::wstring warg(szArglist[i]);
        
        // Convert to UTF-8
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, warg.c_str(), (int)warg.size(), nullptr, 0, nullptr, nullptr);
        if (size_needed > 0) {
            std::string utf8_arg(size_needed, 0);
            WideCharToMultiByte(CP_UTF8, 0, warg.c_str(), (int)warg.size(), &utf8_arg[0], size_needed, nullptr, nullptr);
            args.push_back(utf8_arg);
        } else {
            args.push_back(""); // fallback for conversion error
        }
    }
    
    // Free memory allocated by CommandLineToArgvW
    LocalFree(szArglist);
    
    return args;
}

///@brief get_executable_directory gets the directory where the executable is located
///@return the executable directory path
std::string get_executable_directory() {
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    std::string exe_path(buffer);
    size_t last_slash = exe_path.find_last_of("/\\");
    if (last_slash != std::string::npos) {
        return exe_path.substr(0, last_slash);
    }
    return ".";
}

///@brief get_user_documents_directory gets the user's Documents directory
///@return the user's Documents directory path
std::string get_user_documents_directory() {
    char buffer[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_PERSONAL, NULL, 0, buffer))) {
        return std::string(buffer);
    }
    // Fallback to executable directory if Documents folder cannot be found
    return get_executable_directory();
}

///@brief ensure_models_directory creates the models directory if it doesn't exist
///@param exe_dir the executable directory
void ensure_models_directory(const std::string& exe_dir) {
    // Use Documents directory for models instead of executable directory
    std::string documents_dir = get_user_documents_directory();
    std::string models_dir = documents_dir + "/flm/models";
    if (!std::filesystem::exists(models_dir)) {
        std::filesystem::create_directories(models_dir);
    }
}

///@brief handle_user_input is used to handle the user input
void handle_user_input() {
    std::string input;
    while (!should_exit) {
        header_print("FLM", "Enter 'exit' to stop the server: ");
        std::getline(std::cin, input);
        if (input == "exit") {
            should_exit = true;
            break;
        }
    }
}

///@brief create_lm_server is used to create the ollama server
///@param models the model list
///@param default_tag the default tag
///@param port the port to listen on, default is 11434, same with the ollama server
///@return the server
std::unique_ptr<WebServer> create_lm_server(model_list& models, ModelDownloader& downloader, const std::string& default_tag, int port);

///@brief print_usage prints the usage information for the program
///@param program_name the name of the program
void print_usage(const std::string& program_name);

///@brief get_server_port gets the server port from environment variable FLM_SERVE_PORT
///@return the server port, default is 11434 if environment variable is not set
int get_server_port() {
    char* port_env = nullptr;
    size_t len = 0;
    if (_dupenv_s(&port_env, &len, "FLM_SERVE_PORT") == 0 && port_env != nullptr) {
        try {
            int port = std::stoi(port_env);
            free(port_env);
            if (port > 0 && port <= 65535) {
                return port;
            }
        } catch (const std::exception&) {
            free(port_env);
            // Invalid port number, use default
        }
    }
    return 11434; // Default port
}

///@brief get_models_directory gets the models directory from environment variable or defaults to Documents
///@return the models directory path
std::string get_models_directory() {
    char* model_path_env = nullptr;
    size_t len = 0;
    if (_dupenv_s(&model_path_env, &len, "FLM_MODEL_PATH") == 0 && model_path_env != nullptr) {
        std::string custom_path(model_path_env);
        free(model_path_env);
        if (!custom_path.empty()) {
            return custom_path;
        }
    }
    // Fallback to Documents directory if environment variable is not set
    std::string documents_dir = get_user_documents_directory();
    return documents_dir + "/flm/models";
}

///@brief main function
///@param argc the number of arguments
///@param argv the arguments
///@return the exit code
int main(int argc, char* argv[]) {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    
    // Get Unicode command line arguments
    int unicode_argc;
    std::vector<std::string> unicode_argv = get_unicode_command_line_args(unicode_argc);
    
    std::string command;
    std::string tag;
    // std::wstring filename = L"";
    // int allowed_length = -1;
    bool force_redownload = false;
    std::string power_mode = "performance"; // Default power mode
    bool got_power_mode = false;
    
    // Parse the command line arguments
    if (unicode_argc < 2) {
        print_usage(unicode_argv[0]);
        return 1;
    }
    
    // Parse arguments for each command
    command = unicode_argv[1];
    if (command == "run") {
        if (unicode_argc < 3) {
            print_usage(unicode_argv[0]);
            return 1;
        }
        tag = unicode_argv[2];
        
        // Parse optional arguments
        for (int i = 3; i < unicode_argc; i++) {
            if (unicode_argv[i] == "--pmode" && i + 1 < unicode_argc) {
                power_mode = unicode_argv[i + 1];
                got_power_mode = true;
                i++; // Skip the next argument since we consumed it
            }
        }
    }
    else if (command == "serve") {
        // Check if the second argument is --pmode (no tag provided)
        if (unicode_argc >= 3 && unicode_argv[2] == "--pmode") {
            tag = "llama3.2:1b"; // Use default tag
        } else if (unicode_argc >= 3) {
            tag = unicode_argv[2]; // Use provided tag
        } else {
            tag = "llama3.2:1b"; // No arguments, use default tag
        }
        
        // Parse --pmode option for serve command
        // Start parsing from index 2 and look for --pmode
        for (int i = 2; i < unicode_argc; i++) {
            if (unicode_argv[i] == "--pmode" && i + 1 < unicode_argc) {
                power_mode = unicode_argv[i + 1];
                got_power_mode = true;
                i++; // Skip the next argument since we consumed it
            }
        }
    }
    else if (command == "pull") {
        if (unicode_argc < 3) {
            std::cout << "Usage: " << unicode_argv[0] << " pull <model_tag> [--force]" << std::endl;
            return 1;
        }
        tag = unicode_argv[2];
        // Check for force flag, if true, the model will be re-downloaded
        if (unicode_argc == 4 && unicode_argv[3] == "--force") {
            force_redownload = true;
        }

    }
    else if (command == "version") {
        std::cout << "FLM v" << __FLM_VERSION__ << std::endl;
        return 0;
    }
    else if (command == "help") {
        print_usage(unicode_argv[0]);
        return 0;
    }
    else if (command == "remove") {
        if (unicode_argc < 3) {
            std::cout << "Usage: " << unicode_argv[0] << " remove <model_tag>" << std::endl;
            return 1;
        }
        tag = unicode_argv[2];
    }
    else if (command == "list") {
        if (unicode_argc < 2) {
            std::cout << "Usage: " << unicode_argv[0] << " list" << std::endl;
            return 1;
        }
    }

    // Set process priority to high for better performance
    SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
    
    if (command == "serve" || command == "run"){
        // Configure AMD XRT for the specified power mode
        if (power_mode == "default" || power_mode == "powersaver" || power_mode == "balanced" || 
            power_mode == "performance" || power_mode == "turbo") {
            std::string xrt_cmd = "cd \"C:\\Windows\\System32\\AMD\" && .\\xrt-smi.exe configure --pmode " + power_mode + " > NUL 2>&1";
            header_print("FLM", "Configuring NPU Power Mode to " + power_mode + (got_power_mode ? "" : " (flm default)"));
            system(xrt_cmd.c_str());
        }
        else{
            std::cout << "Invalid power mode: " << power_mode << std::endl;
            std::cout << "Valid power modes: default, powersaver, balanced, performance, turbo" << std::endl;
            return 1;
        }
    }
    // Get the command, model tag, and force flag
    std::string exe_dir = get_executable_directory();
    std::string config_path = exe_dir + "/model_list.json";

    try {
        // Get the models directory from environment variable or default
        std::string models_dir = get_models_directory();
        
        // Load the model list with the models directory as the base
        model_list supported_models(config_path, models_dir);
        ModelDownloader downloader(supported_models);

        // Ensure models directory exists
        if (!std::filesystem::exists(models_dir)) {
            std::filesystem::create_directories(models_dir);
        }

        if (command == "run") {
            Runner runner(supported_models, downloader, tag);
            runner.run();
            // else{
            //     chat_bot chat(0);
            //     if (!downloader.is_model_downloaded(tag)) {
            //         downloader.pull_model(tag);
            //     }
            //     nlohmann::json model_info = supported_models.get_model_info(tag);
            //     chat.load_model(supported_models.get_model_path(tag), model_info);
            //     std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8conv;
            //     // read the file
            //     std::wifstream file(filename);
            //     if (!file.is_open()) {
            //         header_print("FLM", "Error: Could not open file: " << utf8conv.to_bytes(filename));
            //         header_print("FLM", "Please check if the file exists and is readable.");
            //         return 1;
            //     }
            //     file.imbue(std::locale(file.getloc(), new std::codecvt_utf8<wchar_t>));  // treat file content as UTF-8
            //     std::wstring file_content_original((std::istreambuf_iterator<wchar_t>(file)), std::istreambuf_iterator<wchar_t>());
            //     std::string file_content = utf8conv.to_bytes(file_content_original);
            //     file.close();
            //     // close the file
            //     chat.start_ttft_timer();
            //     chat.start_total_timer();
            //     std::vector<int> prompts = chat.tokenize(file_content, true, "user", true);
            //     header_print("FLM", "Prefill starts, " << prompts.size() << " tokens");
            //     std::cout << std::endl;
            //     chat_meta_info meta_info;
            //     bool success = chat.insert(meta_info, prompts);
            //     if (!success){
            //         return 1;
            //     }
            //     chat.stop_ttft_timer();
            //     chat.generate(meta_info, allowed_length, std::cout);
            //     chat.stop_total_timer();
            //     std::cout << std::endl;
            //     chat.verbose();
            // }
        } else if (command == "serve") {
            // Create the server
            int port = get_server_port();
            auto server = create_lm_server(supported_models, downloader, tag, port);
            server->set_max_connections(5);           // Allow up to 2000 concurrent connections
            server->set_io_threads(5);          // Allow up to 5 io threads
            server->set_request_timeout(std::chrono::seconds(600)); // 10 minute timeout for long requests
            // Start the server
            header_print("FLM", "Starting server on port " << port << "...");
            server->start();

            // Start a thread to handle user input, this thread will be used to handle the user input
            std::thread input_thread(handle_user_input);

            // Wait for exit command, this thread will be used to wait for the user to exit the server
            while (!should_exit) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }

            // Cleanup, this will be used to stop the server
            header_print("FLM", "Stopping server...");
            server->stop();
            input_thread.join();
        }
        else if (command == "pull") {
            // Check if the model is already downloaded, if true, the model will not be downloaded
            // Check if model is already downloaded
            if (!force_redownload && downloader.is_model_downloaded(tag)) {
                header_print("FLM", "Model is already downloaded.");
                // Show missing files if any, this will be used to show the missing files
                auto missing_files = downloader.get_missing_files(tag);
                if (!missing_files.empty()) {
                    header_print("FLM", "Missing files:");
                    for (const auto& file : missing_files) {
                        std::cout << "  - " << file << std::endl;
                    }
                } else {
                    header_print("FLM", "All required files are present.");
                }
            } else {
                // Download the model, this will be used to download the model
                bool success = downloader.pull_model(tag, force_redownload);
                if (!success) {
                    header_print("ERROR", "Failed to pull model: " + tag);
                    return 1;
                }
            }
        }
        else if (command == "remove") {
            // Remove the model, this will be used to remove the model
            downloader.remove_model(tag);
        }
        else if (command == "list") {
            // List the models, this will be used to list the models
            std::cout << "Models:" << std::endl;
            nlohmann::json models = supported_models.get_all_models();
            for (const auto& model : models["models"]) {
                bool is_present = downloader.is_model_downloaded(model["name"].get<std::string>());
                std::cout << "  - " << model["name"].get<std::string>();
                if (is_present){
                    std::cout << " ✅";
                }
                else{
                    std::cout << " ⏬";
                }
                std::cout << std::endl;
            }
        }
        else {
            // Invalid command, this will be used to show the invalid command
            std::cerr << "Invalid command: " << command << std::endl;
            print_usage(unicode_argv[0]);
            return 1;
        }
        // Return 0 if the command is valid
        return 0;
    } catch (const std::exception& e) {
        // If an error occurs, this will be used to show the error
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}

void print_usage(const std::string& program_name) {
    int server_port = get_server_port();
    std::cout << "Usage: " << program_name << " run <model_tag> [--pmode <mode>]" << std::endl;
    std::cout << "Usage: " << program_name << " serve <model_tag> [--pmode <mode>]" << std::endl;
    std::cout << "Usage: " << program_name << " pull <model_tag> [--force]" << std::endl;
    std::cout << "Usage: " << program_name << " help" << std::endl;
    std::cout << "Usage: " << program_name << " remove <model_tag>" << std::endl;
    std::cout << "Usage: " << program_name << " list" << std::endl;
    std::cout << "Usage: " << program_name << " version" << std::endl;
    std::cout << "Commands:" << std::endl;
    std::cout << "  run     - Run the model interactively" << std::endl;
    std::cout << "  serve   - Start the Ollama-compatible server" << std::endl;
    std::cout << "  pull    - Download model files if not present" << std::endl;
    std::cout << "  help    - Show the help" << std::endl;
    std::cout << "  list    - List all the models" << std::endl;
    std::cout << "  version - Show the version" << std::endl;
    std::cout << "  remove  - Remove a model" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  --force - Force re-download even if model exists (for pull command)" << std::endl;
    std::cout << "  --pmode - Set power mode: default, powersaver, balanced, performance, turbo (for run/serve commands)" << std::endl;
    std::cout << "Notes:" << std::endl;
    std::cout << "  - The server port is set with environment variable FLM_SERVE_PORT, current value is " << server_port << std::endl;
    std::cout << "  - The models directory is set with environment variable FLM_MODEL_PATH, current value is " << get_models_directory() << std::endl;
}