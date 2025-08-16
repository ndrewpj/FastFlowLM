/// \file image_reader.hpp
/// \brief image_reader class
/// \author FastFlowLM Team
/// \date 2025-08-16
/// \version 0.9.4
/// \note This is a header file for the image_reader functions
#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include "typedef.hpp"
#include "buffer.hpp"
#include "base64.hpp"

// Simple image loading and saving functions
// Load image from file and resize to 896x896 RGB24 format
bytes load_image(const std::string& filename);

bytes load_image_base64(const std::string& base64_string);

// Save RGB24 image as PPM file
bool save_image(const std::string& filename, const bytes& image);


// preprocess the image for gemma3 model
buffer<bf16> preprocess_image(bytes& image);