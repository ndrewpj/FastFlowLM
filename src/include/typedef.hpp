/// \file typedef.hpp
/// \brief typedef file for the FastFlowLM project
/// \author FastFlowLM Team
/// \date 2025-06-24
/// \version 0.9.7
/// \note This file contains the typedefs for the FastFlowLM project

#pragma once
#include <cstdint>
#include <iostream>
#include "buffer.hpp"
#include <immintrin.h>
#include <math.h>
#include <codecvt>

typedef float f32;
typedef std::int8_t i8;
typedef std::int16_t i16;
typedef std::int32_t i32;
typedef std::int64_t i64;
typedef std::uint8_t u8;
typedef std::uint16_t u16;
typedef std::uint32_t u32;
typedef std::uint64_t u64;

/// \brief accdtype is commonly used in LLM models
typedef f32 accdtype;

typedef enum: u8 {cpu, npu} device_t;

/// \brief JSON GET macro
/// \param output the output variable
/// \param json_handler the json handler
/// \param key the key to get the value from
/// \param default_value the default value to assign if the key does not exist
/// \param data_type the data type of the output
/// \note if the key exists, assign the value to the output
/// \note otherwise, assign the default value to the output
/// \note Usage: JSON_GET(output, json_handler, key, default_value, data_type)
/// \note   output: the output variable
//   json_handler: the json handler
//   key: the key to get the value from
//   default_value: the default value to assign if the key does not exist
//   data_type: the data type of the output
// Example: JSON_GET(this->model_desc.vocab_size, json_handler["config"], "vocab_size", 0, i32);
#define JSON_GET(output, json_handler, key, default_value, data_type) if (json_handler.contains(key) && !json_handler[key].is_null()) { \
    output = data_type(json_handler[key]); \
} else { \
    output = default_value; \
}

/// \brief bf16_t class
/// \note This class is used to store bfloat16 values, and provide some arithmetic operations.
/// \note C++17 does not support bfloat16, so we use this class to store bfloat16 values.
class bf16_t {
public:
    u16 value;

    bf16_t(u16 v = 0) : value(v) {}
    // Constructor from float (implicitly used with static_cast)

    explicit bf16_t(float v) {
        u32 f_bits = std::bit_cast<u32>(v);
        u32 rounding_bias = 0x7FFF + ((f_bits >> 16) & 1);
        value = static_cast<u16>((f_bits + rounding_bias) >> 16);
    }    
    bf16_t(int v)  {
        float f = v;
        u32 f_bits = *reinterpret_cast<u32*>(&f);
        this->value = static_cast<u16>(f_bits >> 16); 
    }

    // Implicit conversion to u16
    operator u16() const {
        return this->value;
    }
    // Explicit conversion to float for static_cast
    explicit operator float() const {
        return this->as_float();
    }
    
    // Helper function to get raw pointer for AVX operations
    u16* raw_ptr() {
        return &this->value;
    }
    
    const u16* raw_ptr() const {
        return &this->value;
    }
    // Convert to float
    float as_float() const {
        u32 f_bits = static_cast<u32>(this->value) << 16;
        return *reinterpret_cast<float*>(&f_bits);
    }

    // Print as float
    friend std::ostream& operator<<(std::ostream& os, const bf16_t& bf16) {
        os << bf16.as_float();
        return os;
    }

    // Create bf16_t from float
    static bf16_t from_float(float f) {
        u32 f_bits = *reinterpret_cast<u32*>(&f);
        return bf16_t(static_cast<u16>(f_bits >> 16));
    }

    // For memory reinterpretation (similar to reinterpret_cast)
    static bf16_t* reinterpret_from_u16(u16* ptr) {
        return reinterpret_cast<bf16_t*>(ptr);
    }

    static const bf16_t* reinterpret_from_u16(const u16* ptr) {
        return reinterpret_cast<const bf16_t*>(ptr);
    }

    static u16* reinterpret_to_u16(bf16_t* ptr) {
        return reinterpret_cast<u16*>(ptr);
    }

    static const u16* reinterpret_to_u16(const bf16_t* ptr) {
        return reinterpret_cast<const u16*>(ptr);
    }

    // Get raw memory address
    void* address() {
        return static_cast<void*>(&value);
    }

    const void* address() const {
        return static_cast<const void*>(&value);
    }

    // Assignment from another bf16_t
    bf16_t& operator=(bf16_t other) {
        this->value = other.value;
        return *this;
    }

    // Assignment from float
    bf16_t& operator=(float val) {
        u32 f_bits = *reinterpret_cast<u32*>(&val);
        this->value = static_cast<u16>(f_bits >> 16);
        return *this;
    }

    // Arithmetic operators
    float operator+(const bf16_t& other) const {
        return this->as_float() + other.as_float();
    }

    float operator-(const bf16_t& other) const {
        return this->as_float() - other.as_float();
    }

    float operator*(const bf16_t& other) const {
        return this->as_float() * other.as_float();
    }

    float operator/(const bf16_t& other) const {
        return this->as_float() / other.as_float();
    }

    float operator+(const float& other) const {
        return this->as_float() + other;
    }   

    float operator-(const float& other) const {
        return this->as_float() - other;
    }

    float operator*(const float& other) const {
        return this->as_float() * other;
    }

    float operator/(const float& other) const {
        return this->as_float() / other;
    }

    float operator*(const int& other) const {
        return this->as_float() * other;
    }

    float operator/(const int& other) const {
        return this->as_float() / other;
    }

    // bf16 * float
    friend bf16_t operator*(const bf16_t& lhs, float rhs) {
        return bf16_t::from_float(lhs.as_float() * rhs);
    }
    // float * bf16
    friend bf16_t operator*(float lhs, const bf16_t& rhs) {
        return bf16_t::from_float(lhs * rhs.as_float());
    }

    // bf16 * int
    friend bf16_t operator*(const bf16_t& lhs, int rhs) {
        return bf16_t::from_float(lhs.as_float() * rhs);
    }

    // int * bf16
    friend bf16_t operator*(int lhs, const bf16_t& rhs) {
        return bf16_t::from_float(lhs * rhs.as_float());
    }

    // bf16 + float
    friend bf16_t operator+(const bf16_t& lhs, float rhs) {
        return bf16_t::from_float(lhs.as_float() + rhs);
    }

    // float + bf16
    friend bf16_t operator+(float lhs, const bf16_t& rhs) {
        return bf16_t::from_float(lhs + rhs.as_float());
    }

    // bf16 + int
    friend bf16_t operator+(const bf16_t& lhs, int rhs) {
        return bf16_t::from_float(lhs.as_float() + rhs);
    }

    // int + bf16
    friend bf16_t operator+(int lhs, const bf16_t& rhs) {
        return bf16_t::from_float(lhs + rhs.as_float());
    }

    // Compound assignment operators
    bf16_t& operator+=(const bf16_t& other) {
        *this = *this + other;
        return *this;
    }

    bf16_t& operator-=(const bf16_t& other) {
        *this = *this - other;
        return *this;
    }

    bf16_t& operator*=(const bf16_t& other) {
        *this = *this * other;
        return *this;
    }

    bf16_t& operator/=(const bf16_t& other) {
        *this = *this / other;
        return *this;
    }

    // Comparison operators
    bool operator==(const bf16_t& other) const {
        return this->as_float() == other.as_float();
    }

    bool operator!=(const bf16_t& other) const {
        return !(*this == other);
    }

    bool operator<(const bf16_t& other) const {
        return this->as_float() < other.as_float();
    }

    bool operator<=(const bf16_t& other) const {
        return this->as_float() <= other.as_float();
    }

    bool operator>(const bf16_t& other) const {
        return this->as_float() > other.as_float();
    }

    bool operator>=(const bf16_t& other) const {
        return this->as_float() >= other.as_float();
    }

    
};

typedef bf16_t bf16;
typedef bf16 dtype;
typedef buffer<bf16> vdtype;

/// \brief Helper functions for bf16 array reinterpretation
/// \param u16_arr u16 array
/// \param size size of the array
/// \return bf16_t array view (does not own memory)
inline bf16_t* reinterpret_u16_as_bf16(u16* u16_arr, size_t size) {
    return reinterpret_cast<bf16_t*>(u16_arr);
}

/// \brief Helper functions for bf16 array reinterpretation (const version)
/// \param u16_arr const u16 array
/// \param size size of the array
/// \return const bf16_t array view (does not own memory)
inline const bf16_t* reinterpret_u16_as_bf16(const u16* u16_arr, size_t size) {
    return reinterpret_cast<const bf16_t*>(u16_arr);
}

/// \brief Helper functions for bf16 array reinterpretation
/// \param bf16_arr bf16_t array
/// \param size size of the array
/// \return u16 array view (does not own memory)
inline u16* reinterpret_bf16_as_u16(bf16_t* bf16_arr, size_t size) {
    return reinterpret_cast<u16*>(bf16_arr);
}

/// \brief Helper functions for bf16 array reinterpretation (const version)
/// \param bf16_arr const bf16_t array
/// \param size size of the array
/// \return const u16 array view (does not own memory)
inline const u16* reinterpret_bf16_as_u16(const bf16_t* bf16_arr, size_t size) {
    return reinterpret_cast<const u16*>(bf16_arr);
}

/// \brief AVX2 Convert bfloat16 to float32
/// \param bf16_vals the bfloat16 values, 128-bits, 8xbf16
/// \return the float32 values, 256-bits, 8xfloat32
inline __m256 bf16o_fp32(__m128i bf16_vals) {
    __m256i expanded = _mm256_cvtepu16_epi32(bf16_vals); // Extend to 32-bit
    return _mm256_castsi256_ps(_mm256_slli_epi32(expanded, 16));  // Shift to float position
}

/// \brief AVX2 Convert float32 to bfloat16
/// \param fp32_vals the float32 values, 256-bits, 8xfloat32
/// \return the bfloat16 values, 128-bits, 8xbf16
inline __m128i f32o_bf16(__m256 fp32_vals) {
    __m256i rounded = _mm256_srli_epi32(_mm256_castps_si256(fp32_vals), 16);  // Truncate lower bits
    return _mm_packus_epi32(_mm256_extracti128_si256(rounded, 0),
                            _mm256_extracti128_si256(rounded, 1));
}

/// \brief Convert UTF-8 string to wide string
/// \param str the UTF-8 string
/// \return the wide string
/// \note Usage: std::wstring wstr = utf8_to_wstring(str);
#ifdef _IS_WINDOWS_
inline std::wstring utf8_to_wstring(const std::string& str) {
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), nullptr, 0);
    std::wstring wstr(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &wstr[0], size_needed);
    return wstr;
}
#endif