/// \file model_downloader.cpp
/// \brief Model downloader class
/// \author FastFlowLM Team
/// \date 2025-06-24
/// \version 0.1.0
/// \note This class is used to download models from the huggingface
#include "model_downloader.hpp"
#include "utils/utils.hpp"
#include "download_model.hpp"
#include <sstream>
#include <iomanip>

/// \brief Constructor
/// \param models the model list
/// \return the model downloader
ModelDownloader::ModelDownloader(model_list& models) 
    : supported_models(models), curl_init() {
}

/// \brief Check if the model is downloaded
/// \param model_tag the model tag
/// \return true if the model is downloaded, false otherwise
bool ModelDownloader::is_model_downloaded(const std::string& model_tag) {
    auto missing_files = get_missing_files(model_tag);
    return missing_files.empty();
}

/// \brief Pull the model
/// \param model_tag the model tag
/// \param force_redownload true if the model should be downloaded even if it is already downloaded
/// \return true if the model is downloaded, false otherwise
bool ModelDownloader::pull_model(const std::string& model_tag, bool force_redownload) {
    try {
        // Get model info
        auto model_info = supported_models.get_model_info(model_tag);
        std::string model_name = model_info["name"];
        std::string base_url = model_info["url"];
        
        header_print("FLM", "Model: " + model_tag);
        header_print("FLM", "Name: " + model_name);
        
        // Check if model is already downloaded
        if (!force_redownload && is_model_downloaded(model_tag)) {
            header_print("FLM", "Model already downloaded. Use --force to re-download.");
            return true;
        }
        
        // Get missing files
        auto missing_files = get_missing_files(model_tag);
        if (missing_files.empty() && !force_redownload) {
            header_print("FLM", "All files already present.");
            return true;
        }
        
        if (!missing_files.empty()) {
            header_print("FLM", "Missing files (" + std::to_string(missing_files.size()) + "):");
            for (const auto& file : missing_files) {
                std::cout << "  - " << file << std::endl;
            }
        } else {
            header_print("FLM", "All required files are present.");
        }
        
        // Show present files if any
        auto present_files = get_present_files(model_tag);
        if (!present_files.empty()) {
            header_print("FLM", "Present files (" + std::to_string(present_files.size()) + "):");
            for (const auto& file : present_files) {
                std::cout << "  - " << file << std::endl;
            }
        }
        
        // Build download list
        auto downloads = build_download_list(model_tag);
        if (downloads.empty()) {
            header_print("FLM", "No files to download for model: " + model_tag);
            return true; // Return true since all files are already present
        }
        
        header_print("FLM", "Downloading " + std::to_string(downloads.size()) + " missing files...");
        
        // Show which files will be downloaded
        header_print("FLM", "Files to download:");
        for (const auto& download : downloads) {
            std::string filename = std::filesystem::path(download.second).filename().string();
            std::cout << "  - " << filename << std::endl;
        }
        
        // Download files with progress
        bool success = download_utils::download_multiple_files(downloads, get_progress_callback());
        
        if (success) {
            header_print("FLM", "Model downloaded successfully!");
            
            // Verify download
            auto final_missing = get_missing_files(model_tag);
            if (final_missing.empty()) {
                header_print("FLM", "All files verified successfully.");
            } else {
                header_print("WARNING", "Some files may be missing after download:");
                for (const auto& file : final_missing) {
                    std::cout << "  - " << file << std::endl;
                }
            }
            return true;
        } else {
            header_print("ERROR", "Failed to download model files.");
            return false;
        }
        
    } catch (const std::exception& e) {
        header_print("ERROR", "Exception during download: " + std::string(e.what()));
        return false;
    }
}

/// \brief Model not found
/// \param model_tag the model tag
void ModelDownloader::model_not_found(const std::string& model_tag) {
    header_print("ERROR", "Model not found: " + model_tag);
    header_print("ERROR", "Supported models: ");
    nlohmann::json models = supported_models.get_all_models();
    for (const auto& model : models["models"]) {
        header_print("ERROR", "  - " + model["name"].get<std::string>());
    }
}

/// \brief Get missing files
/// \param model_tag the model tag
/// \return the missing files
std::vector<std::string> ModelDownloader::get_missing_files(const std::string& model_tag) {
    std::vector<std::string> missing_files;
    
    try {
        auto model_info = supported_models.get_model_info(model_tag);
        std::string model_name = model_info["name"];
        
        // Check each required file
        for (int i = 0; i < model_list::model_files_count; ++i) {
            std::string filename = model_list::model_files[i];
            std::string file_path = get_model_file_path(model_tag, filename);
            if (!file_exists(file_path)) {
                missing_files.push_back(filename);
            }
        }
        
    } catch (const std::exception& e) {
        header_print("ERROR", "Error checking missing files: " + std::string(e.what()));
    }
    
    return missing_files;
}

/// \brief Get present files
/// \param model_tag the model tag
/// \return the present files
std::vector<std::string> ModelDownloader::get_present_files(const std::string& model_tag) {
    std::vector<std::string> present_files;
    
    try {
        auto model_info = supported_models.get_model_info(model_tag);
        std::string model_name = model_info["name"];
        
        // Check each required file
        for (int i = 0; i < model_list::model_files_count; ++i) {
            std::string filename = model_list::model_files[i];
            std::string file_path = get_model_file_path(model_tag, filename);
            if (file_exists(file_path)) {
                present_files.push_back(filename);
            }
        }
        
    } catch (const std::exception& e) {
        header_print("ERROR", "Error checking present files: " + std::string(e.what()));
    }
    
    return present_files;
}

/// \brief Get progress callback
/// \return the progress callback
std::function<void(size_t, size_t)> ModelDownloader::get_progress_callback() {
    return [](size_t completed, size_t total) {
        if (total > 0) {
            double percentage = (static_cast<double>(completed) / total) * 100.0;
            std::cout << "\r[FLM]  Overall progress: " << std::fixed << std::setprecision(1) 
                      << percentage << "% (" << completed << "/" << total << " files)" << std::flush;
            
            std::cout << std::endl;
        }
    };
}

/// \brief Check if the file exists
/// \param file_path the file path
/// \return true if the file exists, false otherwise
bool ModelDownloader::file_exists(const std::string& file_path) {
    return std::filesystem::exists(file_path) && std::filesystem::is_regular_file(file_path);
}

/// \brief Get the model file path
/// \param model_tag the model tag
/// \param filename the filename
/// \return the model file path
std::string ModelDownloader::get_model_file_path(const std::string& model_tag, const std::string& filename) {
    std::string model_path = supported_models.get_model_path(model_tag);
    return model_path + "/" + filename;
}

/// \brief Build the download list
/// \param model_tag the model tag
/// \return the download list
std::vector<std::pair<std::string, std::string>> ModelDownloader::build_download_list(const std::string& model_tag) {
    std::vector<std::pair<std::string, std::string>> downloads;
    
    try {
        auto model_info = supported_models.get_model_info(model_tag);
        std::string base_url = model_info["url"];
        std::string model_name = model_info["name"];
        
        // Create model directory
        std::string model_path = supported_models.get_model_path(model_tag);
        std::filesystem::create_directories(model_path);
        
        // Build download list only for missing files
        for (int i = 0; i < model_list::model_files_count; ++i) {
            std::string filename = model_list::model_files[i];
            std::string local_path = get_model_file_path(model_tag, filename);
            
            // Only add to download list if file doesn't exist
            if (!file_exists(local_path)) {
                std::string url = base_url + "/resolve/main/" + filename + "?download=true";
                downloads.emplace_back(url, local_path);
            }
        }
        
    } catch (const std::exception& e) {
        header_print("ERROR", "Error building download list: " + std::string(e.what()));
    }
    
    return downloads;
}

/// \brief Remove a model and all its files
/// \param model_tag the model tag
/// \return true if the model was successfully removed, false otherwise
bool ModelDownloader::remove_model(const std::string& model_tag) {
    try {
        // Check if model exists in supported models by trying to get its info
        try {
            supported_models.get_model_info(model_tag);
        } catch (const std::exception& e) {
            header_print("ERROR", "Model not found: " + model_tag);
            model_not_found(model_tag);
            return false;
        }
        
        // Get model path
        std::string model_path = supported_models.get_model_path(model_tag);
        
        // Check if model directory exists
        if (!std::filesystem::exists(model_path)) {
            header_print("FLM", "Model directory does not exist: " + model_path);
            return true; // Consider it already removed
        }
        
        header_print("FLM", "Removing model: " + model_tag);
        header_print("FLM", "Path: " + model_path);
        
        // Remove all files in the model directory
        size_t removed_files = 0;
        for (const auto& entry : std::filesystem::directory_iterator(model_path)) {
            if (entry.is_regular_file()) {
                std::filesystem::remove(entry.path());
                removed_files++;
            }
        }
        
        // Remove the model directory itself
        if (std::filesystem::remove(model_path)) {
            header_print("FLM", "Successfully removed " + std::to_string(removed_files) + " files and model directory.");
            return true;
        } else {
            header_print("ERROR", "Failed to remove model directory: " + model_path);
            return false;
        }
        
    } catch (const std::exception& e) {
        header_print("ERROR", "Exception during model removal: " + std::string(e.what()));
        return false;
    }
} 