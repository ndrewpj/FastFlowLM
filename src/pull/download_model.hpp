/// \file download_model.hpp
/// \brief Download model class
/// \author FastFlowLM Team
/// \date 2025-06-24
/// \version 0.9.0
/// \note This class for curl download
#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <curl/curl.h>

namespace download_utils {

// Cursor control functions
void hide_cursor();
void show_cursor();

// Callback function for libcurl to write data to a file
size_t write_data_to_file(void* ptr, size_t size, size_t nmemb, FILE* stream);

// Callback function for libcurl to write data to a string
size_t write_data_to_string(void* ptr, size_t size, size_t nmemb, std::string* userdata);

// Progress callback function
int progress_callback(void* clientp, double dltotal, double dlnow, double ultotal, double ulnow);

// Download a file from URL to a local file
bool download_file(const std::string& url, const std::string& local_path, 
                   std::function<void(double)> progress_cb = nullptr);

// Download content from URL to a string
std::string download_string(const std::string& url);

// Download multiple files with progress tracking
bool download_multiple_files(const std::vector<std::pair<std::string, std::string>>& downloads,
                           std::function<void(size_t, size_t)> progress_cb = nullptr);

// Initialize CURL library (call this once at program startup)
bool init_curl();

// Cleanup CURL library (call this once at program shutdown)
void cleanup_curl();

// RAII wrapper for CURL initialization
class CurlInitializer {
public:
    CurlInitializer();
    ~CurlInitializer();
};

} // namespace download_utils 