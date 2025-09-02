/// \file image_reader.cpp
/// \brief image_reader class
/// \author FastFlowLM Team
/// \date 2025-08-16
/// \version 0.9.7
/// \note This is a source file for the image_reader functions

#include "image/image_reader.hpp"
#include "typedef.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <immintrin.h>

// FFmpeg includes for image processing only
extern "C" {
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/frame.h>
#include <libavutil/pixfmt.h>
}

// Initialize FFmpeg once
static bool ffmpeg_initialized = false;
static void initialize_ffmpeg() {
    if (!ffmpeg_initialized) {
        ffmpeg_initialized = true;
    }
}

// Helper to map deprecated YUVJ pixel formats and determine source range
static inline void resolve_source_format_and_range(AVPixelFormat input_format,
                                                  AVPixelFormat &resolved_format,
                                                  int &src_full_range,
                                                  AVColorRange frame_color_range,
                                                  AVCodecID codec_id) {
    resolved_format = input_format;
    src_full_range = 0; // default: limited range

    // If decoder reports full-range, honor it
    if (frame_color_range == AVCOL_RANGE_JPEG) {
        src_full_range = 1;
    }

    // JPEG is typically full range
    if (codec_id == AV_CODEC_ID_MJPEG) {
        src_full_range = 1;
    }

    switch (input_format) {
        case AV_PIX_FMT_YUVJ420P:
            resolved_format = AV_PIX_FMT_YUV420P;
            src_full_range = 1;
            break;
        case AV_PIX_FMT_YUVJ422P:
            resolved_format = AV_PIX_FMT_YUV422P;
            src_full_range = 1;
            break;
        case AV_PIX_FMT_YUVJ444P:
            resolved_format = AV_PIX_FMT_YUV444P;
            src_full_range = 1;
            break;
        case AV_PIX_FMT_YUVJ440P:
            resolved_format = AV_PIX_FMT_YUV440P;
            src_full_range = 1;
            break;
        default:
            break;
    }
}

bytes load_image(const std::string& filename) {
    initialize_ffmpeg();
 
    const int target_width = 896;
    const int target_height = 896;
    
    try {
        // Check if file exists
        if (!std::filesystem::exists(filename)) {
            std::cerr << "Error: File does not exist: " << filename << std::endl;
            return bytes();
        }

        // Read file into memory
        std::ifstream file(filename, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file: " << filename << std::endl;
            return bytes();
        }
        
        std::streamsize file_size = file.tellg();
        file.seekg(0, std::ios::beg);
        
        std::vector<uint8_t> file_data(file_size);
        if (!file.read(reinterpret_cast<char*>(file_data.data()), file_size)) {
            std::cerr << "Error: Could not read file data" << std::endl;
            return bytes();
        }
        file.close();

        // Create memory context for decoding
        AVCodecContext* codecContext = nullptr;
        AVFrame* frame = nullptr;
        AVPacket* packet = nullptr;
        
        try {
            // Find decoder based on file header
            const AVCodec* codec = nullptr;
            if (file_data.size() >= 8) {
                // Check for JPEG
                if (file_data[0] == 0xFF && file_data[1] == 0xD8) {
                    codec = avcodec_find_decoder(AV_CODEC_ID_MJPEG);
                }
                // Check for PNG
                else if (file_data.size() >= 8 && 
                         file_data[0] == 0x89 && file_data[1] == 0x50 && 
                         file_data[2] == 0x4E && file_data[3] == 0x47) {
                    codec = avcodec_find_decoder(AV_CODEC_ID_PNG);
                }
            }
            
            if (!codec) {
                std::cerr << "Error: Unsupported image format (only JPEG and PNG supported)" << std::endl;
                return bytes();
            }

            // Create codec context
            codecContext = avcodec_alloc_context3(codec);
            if (!codecContext) {
                std::cerr << "Error: Could not allocate codec context" << std::endl;
                return bytes();
            }

            // Open codec
            if (avcodec_open2(codecContext, codec, nullptr) < 0) {
                std::cerr << "Error: Could not open codec" << std::endl;
                return bytes();
            }

            // Allocate frame and packet
            frame = av_frame_alloc();
            packet = av_packet_alloc();
            if (!frame || !packet) {
                std::cerr << "Error: Could not allocate frame or packet" << std::endl;
                return bytes();
            }

            // Set packet data
            packet->data = file_data.data();
            packet->size = static_cast<int>(file_data.size());

            // Send packet for decoding
            int response = avcodec_send_packet(codecContext, packet);
            if (response < 0) {
                std::cerr << "Error: Error sending packet for decoding" << std::endl;
                return bytes();
            }

            // Receive decoded frame
            response = avcodec_receive_frame(codecContext, frame);
            if (response < 0) {
                std::cerr << "Error: Error during decoding" << std::endl;
                return bytes();
            }

            // Resolve deprecated pixel formats and color range
            AVPixelFormat srcFmtResolved = AV_PIX_FMT_NONE;
            int srcIsFullRange = 0;
            resolve_source_format_and_range(static_cast<AVPixelFormat>(frame->format),
                                            srcFmtResolved,
                                            srcIsFullRange,
                                            frame->color_range,
                                            codecContext->codec_id);

            // Convert to RGB24 and resize to target dimensions
            SwsContext* swsContext = sws_getContext(
                frame->width, frame->height, srcFmtResolved,
                target_width, target_height, AV_PIX_FMT_RGB24,
                SWS_BILINEAR, nullptr, nullptr, nullptr
            );
            
            if (!swsContext) {
                std::cerr << "Error: Could not create scaling context" << std::endl;
                return bytes();
            }

            // Set colorspace details to respect full/limited range
            const int* srcCoeffs = sws_getCoefficients(SWS_CS_DEFAULT);
            const int* dstCoeffs = sws_getCoefficients(SWS_CS_DEFAULT);
            int dstIsFullRange = 1; // RGB is full range 0..255
            sws_setColorspaceDetails(swsContext, srcCoeffs, srcIsFullRange,
                                     dstCoeffs, dstIsFullRange,
                                     0, 1 << 16, 1 << 16);

            // Allocate RGB24 frame
            AVFrame* rgbFrame = av_frame_alloc();
            int rgbBufferSize = av_image_get_buffer_size(AV_PIX_FMT_RGB24, target_width, target_height, 1);
            bytes result(rgbBufferSize);
            
            if (result.size() > 0) {
                av_image_fill_arrays(rgbFrame->data, rgbFrame->linesize, result.data(), AV_PIX_FMT_RGB24, target_width, target_height, 1);
                
                // Convert and resize to RGB24
                sws_scale(swsContext, frame->data, frame->linesize, 0, frame->height, rgbFrame->data, rgbFrame->linesize);
                
                av_frame_free(&rgbFrame);
                sws_freeContext(swsContext);
                // Cleanup decoder resources before returning
                if (frame) av_frame_free(&frame);
                if (packet) av_packet_free(&packet);
                if (codecContext) {
                    avcodec_close(codecContext);
                    avcodec_free_context(&codecContext);
                }
                return result;
            }

            av_frame_free(&rgbFrame);
            sws_freeContext(swsContext);
            return bytes();

        } catch (...) {
            // Cleanup on error
            if (frame) av_frame_free(&frame);
            if (packet) av_packet_free(&packet);
            if (codecContext) avcodec_free_context(&codecContext);
            throw;
        }

        if (frame) {
            av_frame_free(&frame);
        }
        if (packet) {
            av_packet_free(&packet);
        }
        if (codecContext) {
            avcodec_close(codecContext);
            avcodec_free_context(&codecContext);
        }

        return bytes();
    }
    catch (const std::exception& e) {
        std::cerr << "Error loading image: " << e.what() << std::endl;
        return bytes();
    }
}

bytes load_image_base64(const std::string& base64_string) {
    initialize_ffmpeg();
    
    const int target_width = 896;
    const int target_height = 896;
    
    try {
        // Decode base64 to binary data
        std::string decoder_bytes = base64::from_base64(base64_string);
        
        // Check for valid image formats
        if (decoder_bytes.size() < 8) {
            std::cerr << "Error: Invalid image data (too small)" << std::endl;
            return bytes();
        }
        
        // Convert string to vector<uint8_t> for processing
        std::vector<uint8_t> file_data(decoder_bytes.begin(), decoder_bytes.end());
        
        // Create memory context for decoding
        AVCodecContext* codecContext = nullptr;
        AVFrame* frame = nullptr;
        AVPacket* packet = nullptr;
        
        try {
            // Find decoder based on file header
            const AVCodec* codec = nullptr;
            if (file_data.size() >= 8) {
                // Check for JPEG
                if (file_data[0] == 0xFF && file_data[1] == 0xD8) {
                    codec = avcodec_find_decoder(AV_CODEC_ID_MJPEG);
                }
                // Check for PNG
                else if (file_data[0] == 0x89 && file_data[1] == 0x50 && 
                         file_data[2] == 0x4E && file_data[3] == 0x47) {
                    codec = avcodec_find_decoder(AV_CODEC_ID_PNG);
                }
            }
            
            if (!codec) {
                std::cerr << "Error: Unsupported image format (only JPEG and PNG supported)" << std::endl;
                return bytes();
            }

            // Create codec context
            codecContext = avcodec_alloc_context3(codec);
            if (!codecContext) {
                std::cerr << "Error: Could not allocate codec context" << std::endl;
                return bytes();
            }

            // Open codec
            if (avcodec_open2(codecContext, codec, nullptr) < 0) {
                std::cerr << "Error: Could not open codec" << std::endl;
                return bytes();
            }

            // Allocate frame and packet
            frame = av_frame_alloc();
            packet = av_packet_alloc();
            if (!frame || !packet) {
                std::cerr << "Error: Could not allocate frame or packet" << std::endl;
                return bytes();
            }

            // Set packet data
            packet->data = file_data.data();
            packet->size = static_cast<int>(file_data.size());

            // Send packet for decoding
            int response = avcodec_send_packet(codecContext, packet);
            if (response < 0) {
                std::cerr << "Error: Error sending packet for decoding" << std::endl;
                return bytes();
            }

            // Receive decoded frame
            response = avcodec_receive_frame(codecContext, frame);
            if (response < 0) {
                std::cerr << "Error: Error during decoding" << std::endl;
                return bytes();
            }

            // Resolve deprecated pixel formats and color range
            AVPixelFormat srcFmtResolved = AV_PIX_FMT_NONE;
            int srcIsFullRange = 0;
            resolve_source_format_and_range(static_cast<AVPixelFormat>(frame->format),
                                            srcFmtResolved,
                                            srcIsFullRange,
                                            frame->color_range,
                                            codecContext->codec_id);

            // Convert to RGB24 and resize to target dimensions
            SwsContext* swsContext = sws_getContext(
                frame->width, frame->height, srcFmtResolved,
                target_width, target_height, AV_PIX_FMT_RGB24,
                SWS_BILINEAR, nullptr, nullptr, nullptr
            );
            
            if (!swsContext) {
                std::cerr << "Error: Could not create scaling context" << std::endl;
                return bytes();
            }

            // Set colorspace details to respect full/limited range
            const int* srcCoeffs = sws_getCoefficients(SWS_CS_DEFAULT);
            const int* dstCoeffs = sws_getCoefficients(SWS_CS_DEFAULT);
            int dstIsFullRange = 1; // RGB is full range 0..255
            sws_setColorspaceDetails(swsContext, srcCoeffs, srcIsFullRange,
                                     dstCoeffs, dstIsFullRange,
                                     0, 1 << 16, 1 << 16);

            // Allocate RGB24 frame
            AVFrame* rgbFrame = av_frame_alloc();
            int rgbBufferSize = av_image_get_buffer_size(AV_PIX_FMT_RGB24, target_width, target_height, 1);
            bytes result(rgbBufferSize);
            
            if (result.size() > 0) {
                av_image_fill_arrays(rgbFrame->data, rgbFrame->linesize, result.data(), AV_PIX_FMT_RGB24, target_width, target_height, 1);
                
                // Convert and resize to RGB24
                sws_scale(swsContext, frame->data, frame->linesize, 0, frame->height, rgbFrame->data, rgbFrame->linesize);
                
                av_frame_free(&rgbFrame);
                sws_freeContext(swsContext);
                // Cleanup decoder resources before returning
                if (frame) av_frame_free(&frame);
                if (packet) av_packet_free(&packet);
                if (codecContext) {
                    avcodec_close(codecContext);
                    avcodec_free_context(&codecContext);
                }
                return result;
            }

            av_frame_free(&rgbFrame);
            sws_freeContext(swsContext);
            return bytes();

        } catch (...) {
            // Cleanup on error
            if (frame) av_frame_free(&frame);
            if (packet) av_packet_free(&packet);
            if (codecContext) avcodec_free_context(&codecContext);
            throw;
        }

        // Cleanup
        if (frame) av_frame_free(&frame);
        if (packet) av_packet_free(&packet);
        if (codecContext) avcodec_free_context(&codecContext);

        return bytes();
    }
    catch (const std::exception& e) {
        std::cerr << "Error loading base64 image: " << e.what() << std::endl;
        return bytes();
    }
}

bool save_image(const std::string& filename, const bytes& image) {
    try {
        // Calculate dimensions from image size (RGB24 = 3 bytes per pixel)
        if (image.size() % 3 != 0) {
            std::cerr << "Error: Invalid image data size (not divisible by 3)" << std::endl;
            return false;
        }
        
        int totalPixels = static_cast<int>(image.size() / 3);
        int width = static_cast<int>(sqrt(totalPixels));
        int height = totalPixels / width;
        
        if (width * height != totalPixels) {
            std::cerr << "Error: Image data does not represent a square image" << std::endl;
            return false;
        }

        // Write PPM format (Portable Pixmap)
        std::ofstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file for writing: " << filename << std::endl;
            return false;
        }

        // Write PPM header: "P6\nwidth height\n255\n"
        file << "P6\n" << width << " " << height << "\n255\n";
        
        // Write RGB data
        file.write(reinterpret_cast<const char*>(image.data()), image.size());
        file.close();

        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error saving image: " << e.what() << std::endl;
        return false;
    }
}

///@brief: preprocess the image for gemma3 model
///@note: 1. Reorder: 896x896x3 -> 3x896x896
///@param: image: the image to preprocess
///@return: the preprocessed image
buffer<bf16> preprocess_image(bytes& image) {
    buffer<bf16> result(3 * 896 * 896);
    uint8_t* image_ptr = image.data();
    
    // AVX2 constants for normalization
    const __m256 scale_factor = _mm256_set1_ps(0.00392156862745098f); // 1.0f / 255.0f
    const __m256 subtract_factor = _mm256_set1_ps(0.5f);
    const __m256 multiply_factor = _mm256_set1_ps(2.0f);
    
    // Process 8 pixels at a time using AVX2
    const int pixels_per_vector = 8;
    const int total_pixels = 896 * 896;
    const int vector_pixels = total_pixels - (total_pixels % pixels_per_vector);

    // Shuffle masks for R, G, B channels
    const __m128i mask_lo_r = _mm_setr_epi8(
        0,  3,  6,  9, 12, 15, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1
    );
    const __m128i mask_hi_r = _mm_setr_epi8(
        2,  5, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1
    );

    const __m128i mask_lo_g = _mm_setr_epi8(
        1,  4,  7, 10, 13, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1
    );
    const __m128i mask_hi_g = _mm_setr_epi8(
        0,  3,  6, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1
    );

    const __m128i mask_lo_b = _mm_setr_epi8(
        2,  5,  8, 11, 14, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1
    );
    const __m128i mask_hi_b = _mm_setr_epi8(
        1,  4,  7, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1
    );

    for (int idx = 0; idx < vector_pixels; idx += pixels_per_vector) {
        // Load 8 pixels (24 bytes) into a 256-bit register
        __m256i pix = _mm256_loadu_si256((__m256i*)(image_ptr + idx * 3));

        // Split into low/high 128-bit lanes
        __m128i lo = _mm256_castsi256_si128(pix);
        __m128i hi = _mm256_extracti128_si256(pix, 1);

        // R channel
        __m128i r_lo   = _mm_shuffle_epi8(lo, mask_lo_r);
        __m128i r_hi   = _mm_shuffle_epi8(hi, mask_hi_r);
        __m128i r_hi_s = _mm_bslli_si128(r_hi, 6);          // shift R6->byte6, R7->byte7
        __m128i r_full = _mm_or_si128(r_lo, r_hi_s);
        __m256  r_f    = _mm256_cvtepi32_ps(_mm256_cvtepu8_epi32(r_full));

        // G channel
        __m128i g_lo   = _mm_shuffle_epi8(lo, mask_lo_g);
        __m128i g_hi   = _mm_shuffle_epi8(hi, mask_hi_g);
        __m128i g_hi_s = _mm_bslli_si128(g_hi, 5);          // shift G5->byte5, G6->6, G7->7
        __m128i g_full = _mm_or_si128(g_lo, g_hi_s);
        __m256  g_f    = _mm256_cvtepi32_ps(_mm256_cvtepu8_epi32(g_full));

        // B channel
        __m128i b_lo   = _mm_shuffle_epi8(lo, mask_lo_b);
        __m128i b_hi   = _mm_shuffle_epi8(hi, mask_hi_b);
        __m128i b_hi_s = _mm_bslli_si128(b_hi, 5);          // shift B5->byte5, B6->6, B7->7
        __m128i b_full = _mm_or_si128(b_lo, b_hi_s);
        __m256  b_f    = _mm256_cvtepi32_ps(_mm256_cvtepu8_epi32(b_full));

        // Normalize each channel: (x/255 - 0.5) * 2
        r_f = _mm256_mul_ps(_mm256_sub_ps(_mm256_mul_ps(r_f, scale_factor), subtract_factor), multiply_factor);
        g_f = _mm256_mul_ps(_mm256_sub_ps(_mm256_mul_ps(g_f, scale_factor), subtract_factor), multiply_factor);
        b_f = _mm256_mul_ps(_mm256_sub_ps(_mm256_mul_ps(b_f, scale_factor), subtract_factor), multiply_factor);

        // Convert to BF16
        __m128i r_b = f32o_bf16(r_f);
        __m128i g_b = f32o_bf16(g_f);
        __m128i b_b = f32o_bf16(b_f);

        // Extract 8 bf16 values
        union { __m128i v; uint16_t a[8]; } ru{r_b}, gu{g_b}, bu{b_b};

        // Store into result buffer
        for (int i = 0; i < pixels_per_vector; i++) {
            int row = (idx + i) / 896;
            int col = (idx + i) % 896;
            size_t base = row * 896 + col;
            result[0 * total_pixels + base] = bf16_t(ru.a[i]);
            result[1 * total_pixels + base] = bf16_t(gu.a[i]);
            result[2 * total_pixels + base] = bf16_t(bu.a[i]);
        }
    }

    
    image.release();
    return result;
}