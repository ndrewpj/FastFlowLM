/// \file main.cpp
/// \brief Main entry point for the FLM application
/// \author FastFlowLM Team
/// \date 2025-06-24
/// \version 1.0
/// \note This is a header file for the main entry point
#pragma once
#include "runner.hpp"
#include "server.hpp"
#include "model_list.hpp"
#include "model_downloader.hpp"
#include "utils/utils.hpp"
#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <windows.h>
#include <filesystem>

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

///@brief ensure_models_directory creates the models directory if it doesn't exist
///@param exe_dir the executable directory
void ensure_models_directory(const std::string& exe_dir) {
    std::string models_dir = exe_dir + "/models";
    if (!std::filesystem::exists(models_dir)) {
        std::filesystem::create_directories(models_dir);
    }
}

///@brief handle_user_input is used to handle the user input
void handle_user_input() {
    std::string input;
    while (!should_exit) {
        std::cout << "Enter 'exit' to stop the server: ";
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
    // Initialize Unicode support for the console
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    if (argc < 3 || argc > 4) {
        std::cout << "Usage: " << argv[0] << " <command: run | serve | pull> <model_tag> [--force]" << std::endl;
        std::cout << "Commands:" << std::endl;
        std::cout << "  run   - Run the model interactively" << std::endl;
        std::cout << "  serve - Start the Ollama-compatible server" << std::endl;
        std::cout << "  pull  - Download model files if not present" << std::endl;
        std::cout << "Options:" << std::endl;
        std::cout << "  --force - Force re-download even if model exists (for pull command)" << std::endl;
        return 1;
    }
    // Get the command, model tag, and force flag
    std::string exe_dir = get_executable_directory();
    std::string config_path = exe_dir + "/model_list.json";
    std::string command = argv[1];
    std::string tag = argv[2];
    bool force_redownload = false;
    
    // Check for force flag, if true, the model will be re-downloaded
    if (argc == 4 && std::string(argv[3]) == "--force") {
        force_redownload = true;
    }

    try {
        // Load the model list
        model_list supported_models(config_path, exe_dir);
        ModelDownloader downloader(supported_models);

        // Ensure models directory exists
        ensure_models_directory(exe_dir);

        if (command == "run") {
            Runner runner(supported_models, downloader, tag);
            runner.run();
        } else if (command == "serve") {
            // Create the server
            auto server = create_lm_server(supported_models, downloader, tag, 11434);
            // Start the server
            std::cout << "Starting server on port 11434..." << std::endl;
            server->start();

            // Start a thread to handle user input, this thread will be used to handle the user input
            std::thread input_thread(handle_user_input);

            // Wait for exit command, this thread will be used to wait for the user to exit the server
            while (!should_exit) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }

            // Cleanup, this will be used to stop the server
            std::cout << "Stopping server..." << std::endl;
            server->stop();
            input_thread.join();
        }
        else if (command == "pull") {
            // Check if the model is already downloaded, if true, the model will not be downloaded
            // Check if model is already downloaded
            if (!force_redownload && downloader.is_model_downloaded(tag)) {
                header_print("PULL", "Model is already downloaded.");
                // Show missing files if any, this will be used to show the missing files
                auto missing_files = downloader.get_missing_files(tag);
                if (!missing_files.empty()) {
                    header_print("PULL", "Missing files:");
                    for (const auto& file : missing_files) {
                        std::cout << "  - " << file << std::endl;
                    }
                } else {
                    header_print("PULL", "All required files are present.");
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