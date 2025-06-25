/// \file download_model.cpp
/// \brief Download model class
/// \author FastFlowLM Team
/// \date 2025-06-24
/// \version 0.1.0
/// \note This class for curl download
#include "download_model.hpp"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <iomanip>

namespace download_utils {

/// \brief Callback function for libcurl to write data to a file
/// \param ptr the pointer to the data
/// \param size the size of the data
/// \param nmemb the number of items
/// \param stream the stream to write to
/// \return the number of bytes written
size_t write_data_to_file(void* ptr, size_t size, size_t nmemb, FILE* stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

/// \brief Callback function for libcurl to write data to a string
/// \param ptr the pointer to the data
/// \param size the size of the data
/// \param nmemb the number of items
/// \param userdata the string to write to
/// \return the number of bytes written
size_t write_data_to_string(void* ptr, size_t size, size_t nmemb, std::string* userdata) {
    userdata->append((char*)ptr, size * nmemb);
    return size * nmemb;
}

/// \brief Progress callback function
/// \param clientp the client pointer
/// \param dltotal the total download size
/// \param dlnow the current download size
/// \param ultotal the total upload size
/// \param ulnow the current upload size
/// \return the progress
int progress_callback(void* clientp, double dltotal, double dlnow, double ultotal, double ulnow) {
    if (dltotal > 0) {
        double percentage = (dlnow / dltotal) * 100.0;
        std::cout << "\rDownloading: " << std::fixed << std::setprecision(1) 
                  << percentage << "% (" << dlnow / 1024 / 1024 << "MB / " 
                  << dltotal / 1024 / 1024 << "MB)" << std::flush;
    }
    return 0;
}

/// \brief Download a file from URL to a local file
/// \param url the URL to download from
/// \param local_path the local path to save the file
/// \param progress_cb the progress callback
/// \return true if the file is downloaded, false otherwise
bool download_file(const std::string& url, const std::string& local_path, 
                   std::function<void(double)> progress_cb) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initialize CURL" << std::endl;
        return false;
    }

    // Create directory if it doesn't exist
    std::filesystem::path path(local_path);
    std::filesystem::create_directories(path.parent_path());

    FILE* fp = fopen(local_path.c_str(), "wb");
    if (!fp) {
        std::cerr << "Failed to open file for writing: " << local_path << std::endl;
        curl_easy_cleanup(curl);
        return false;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data_to_file);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "FastFlowLM/1.0");
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 300L); // 5 minutes timeout

    // Set progress callback if provided
    if (progress_cb) {
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progress_callback);
    }

    CURLcode res = curl_easy_perform(curl);
    
    fclose(fp);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        std::cerr << "CURL error: " << curl_easy_strerror(res) << std::endl;
        std::filesystem::remove(local_path); // Remove partial download
        return false;
    }

    std::cout << std::endl << "Download completed: " << local_path << std::endl;
    return true;
}

/// \brief Download content from URL to a string
/// \param url the URL to download from
/// \return the downloaded string
std::string download_string(const std::string& url) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initialize CURL" << std::endl;
        return "";
    }

    std::string response;
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data_to_string);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "FastFlowLM/1.0");
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60L); // 1 minute timeout

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        std::cerr << "CURL error: " << curl_easy_strerror(res) << std::endl;
        return "";
    }

    return response;
}

/// \brief Download multiple files with progress tracking
/// \param downloads the downloads
/// \param progress_cb the progress callback
/// \return true if the files are downloaded, false otherwise
bool download_multiple_files(const std::vector<std::pair<std::string, std::string>>& downloads,
                           std::function<void(size_t, size_t)> progress_cb) {
    size_t total_files = downloads.size();
    size_t completed_files = 0;

    for (const auto& [url, local_path] : downloads) {
        std::cout << "Downloading " << (completed_files + 1) << "/" << total_files 
                  << ": " << std::filesystem::path(url).filename().string() << std::endl;

        auto file_progress = [&](double percentage) {
            if (progress_cb) {
                progress_cb(completed_files, total_files);
            }
        };

        if (!download_file(url, local_path, file_progress)) {
            std::cerr << "Failed to download: " << url << std::endl;
            return false;
        }

        completed_files++;
        if (progress_cb) {
            progress_cb(completed_files, total_files);
        }
    }

    std::cout << "All downloads completed successfully!" << std::endl;
    return true;
}

/// \brief Initialize CURL library (call this once at program startup)
/// \return true if the CURL library is initialized, false otherwise
bool init_curl() {
    CURLcode res = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (res != CURLE_OK) {
        std::cerr << "Failed to initialize CURL library: " << curl_easy_strerror(res) << std::endl;
        return false;
    }
    return true;
}

/// \brief Cleanup CURL library (call this once at program shutdown)
void cleanup_curl() {
    curl_global_cleanup();
}

/// \brief RAII wrapper for CURL initialization
/// \return the CURL initializer
CurlInitializer::CurlInitializer() {
    if (!init_curl()) {
        throw std::runtime_error("Failed to initialize CURL");
    }
}

/// \brief Destructor
CurlInitializer::~CurlInitializer() {
    cleanup_curl();
}

} // namespace download_utils 