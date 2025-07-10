/// \file main.cpp
/// \brief Main entry point for the FLM application
/// \author FastFlowLM Team
/// \date 2025-06-24
/// \version 0.1.0
/// \note This is a header file for the main entry point
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


// Global variables
///@brief should_exit is used to control the server thread
std::atomic<bool> should_exit(false);

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

///@brief main function
///@param argc the number of arguments
///@param argv the arguments
///@return the exit code
int main(int argc, char* argv[]) {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    std::string command;
    std::string tag;
    std::string filename = "";
    bool force_redownload = false;
    // Parse the command line arguments
    if (argc < 2 || argc > 4) {
        std::cout << "Usage: " << argv[0] << " <command: run <model_tag> <file_name>" << std::endl;
        std::cout << "Usage: " << argv[0] << " <command: serve <model_tag>" << std::endl;
        std::cout << "Usage: " << argv[0] << " <command: pull <model_tag> [--force]" << std::endl;
        std::cout << "Commands:" << std::endl;
        std::cout << "  run    - Run the model interactively" << std::endl;
        std::cout << "  serve  - Start the Ollama-compatible server" << std::endl;
        std::cout << "  pull   - Download model files if not present" << std::endl;
        std::cout << "  help   - Show the help" << std::endl;
        std::cout << "  remove - Remove a model" << std::endl;
        std::cout << "  list   - List all the models" << std::endl;
        std::cout << "Options:" << std::endl;
        std::cout << "  --force - Force re-download even if model exists (for pull command)" << std::endl;
        return 1;
    }
    command = argv[1];
    if (command == "run") {
        if (argc < 3) {
            std::cout << "Usage: " << argv[0] << " run <model_tag>" << std::endl;
            return 1;
        }
        if (argc == 4){
            filename = argv[3];
        }
        tag = argv[2];
    }
    else if (command == "serve") {
        if (argc < 3) {
            tag = "llama3.2:1b";
        }
        else {
            tag = argv[2];
        }
    }
    else if (command == "pull") {
        if (argc < 3) {
            std::cout << "Usage: " << argv[0] << " pull <model_tag>" << std::endl;
            return 1;
        }
        tag = argv[2];
        // Check for force flag, if true, the model will be re-downloaded
        if (argc == 4 && std::string(argv[3]) == "--force") {
            force_redownload = true;
        }

    }
    else if (command == "help") {
        std::cout << "Usage: " << argv[0] << " <command: run <model_tag> <file_name>" << std::endl;
        std::cout << "Usage: " << argv[0] << " <command: serve <model_tag>" << std::endl;
        std::cout << "Usage: " << argv[0] << " <command: pull <model_tag> [--force]" << std::endl;
        std::cout << "Usage: " << argv[0] << " <command: help" << std::endl;
        std::cout << "Usage: " << argv[0] << " <command: remove <model_tag>" << std::endl;
        std::cout << "Usage: " << argv[0] << " <command: list" << std::endl;
        std::cout << "Commands:" << std::endl;
        std::cout << "  run    - Run the model interactively" << std::endl;
        std::cout << "  serve  - Start the Ollama-compatible server" << std::endl;
        std::cout << "  pull   - Download model files if not present" << std::endl;
        std::cout << "  help   - Show the help" << std::endl;
        std::cout << "  remove - Remove a model" << std::endl;
        return 0;
    }
    else if (command == "remove") {
        if (argc < 3) {
            std::cout << "Usage: " << argv[0] << " remove <model_tag>" << std::endl;
            return 1;
        }
        tag = argv[2];
    }
    else if (command == "list") {
        if (argc < 2) {
            std::cout << "Usage: " << argv[0] << " list" << std::endl;
            return 1;
        }
    }

    // Get the command, model tag, and force flag
    std::string exe_dir = get_executable_directory();
    std::string config_path = exe_dir + "/model_list.json";

    try {
        // Get the Documents directory for models
        std::string documents_dir = get_user_documents_directory();
        std::string models_dir = documents_dir + "/flm/models";
        
        // Load the model list with Documents directory as the base
        model_list supported_models(config_path, documents_dir);
        ModelDownloader downloader(supported_models);

        // Ensure models directory exists in Documents
        if (!std::filesystem::exists(models_dir)) {
            std::filesystem::create_directories(models_dir);
        }

        if (command == "run") {
            if (filename == ""){          
                Runner runner(supported_models, downloader, tag);
                runner.run();
            }
            else{
                chat_bot chat(0);
                if (!downloader.is_model_downloaded(tag)) {
                    downloader.pull_model(tag);
                }
                nlohmann::json model_info = supported_models.get_model_info(tag);
                chat.load_model(supported_models.get_model_path(tag), model_info);
                // read the file
                std::ifstream file(filename);
                if (!file.is_open()) {
                    std::cerr << "Error: Could not open file: " << filename << std::endl;
                    return 1;
                }
                // get file size
                file.seekg(0, std::ios::end);
                size_t file_size = file.tellg();
                file.seekg(0, std::ios::beg);
                // read the file
                std::string file_content(file_size, '\0');
                file.read(file_content.data(), file_size);
                // close the file
                chat.start_ttft_timer();
                chat.start_total_timer();
                std::vector<int> prompts = chat.tokenize(file_content, true, "user", true);
                header_print("FLM", "Prefill starts, " << prompts.size() << " tokens");
                std::cout << std::endl;
                chat_meta_info meta_info;
                chat.insert(meta_info, prompts);
                chat.stop_ttft_timer();
                chat.generate(meta_info, -1, std::cout);
                chat.stop_total_timer();
                std::cout << std::endl;
                chat.verbose();
            }
        } else if (command == "serve") {
            // Create the server
            auto server = create_lm_server(supported_models, downloader, tag, 11434);
            server->set_max_connections(5);           // Allow up to 2000 concurrent connections
            server->set_io_threads(5);          // Allow up to 5 io threads
            server->set_request_timeout(std::chrono::seconds(600)); // 10 minute timeout for long requests
            // Start the server
            header_print("FLM", "Starting server on port 11434...");
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
            std::cout << "Valid commands: run, serve, pull" << std::endl;
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