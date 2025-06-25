/// \file model_downloader.hpp
/// \brief Model downloader class
/// \author FastFlowLM Team
/// \date 2025-06-24
/// \version 1.0
/// \note This class is used to download models from the huggingface
#pragma once

#include "model_list.hpp"
#include "download_model.hpp"
#include "nlohmann/json.hpp"
#include <filesystem>
#include <vector>
#include <string>
#include <iostream>

class ModelDownloader {
public:
    ModelDownloader(model_list& models);
    
    // Check if model is already downloaded
    bool is_model_downloaded(const std::string& model_tag);
    
    // Download model files if not present
    bool pull_model(const std::string& model_tag, bool force_redownload = false);
    
    // Get list of missing files for a model
    std::vector<std::string> get_missing_files(const std::string& model_tag);
    
    // Get download progress callback
    std::function<void(size_t, size_t)> get_progress_callback();

    void model_not_found(const std::string& model_tag);

private:
    model_list& supported_models;
    download_utils::CurlInitializer curl_init;
    
    // Check if a specific file exists
    bool file_exists(const std::string& file_path);
    
    // Get the full path for a model file
    std::string get_model_file_path(const std::string& model_tag, const std::string& filename);
    
    // Build download URLs for model files
    std::vector<std::pair<std::string, std::string>> build_download_list(const std::string& model_tag);
}; 