/// \file tokenizer.cpp
/// \brief Tokenizer implementation for text encoding/decoding
/// \author FastFlowLM Team
/// \date 2025-06-24
/// \version 0.9.4
#include "tokenizer/tokenizer.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstdint>
#include <unordered_map>
#include <regex>

/// \brief Constructor
/// \param model_path the model path
Tokenizer::Tokenizer(const std::string& model_path) {
    std::ifstream fs(model_path + "/tokenizer.json", std::ios::in | std::ios::binary);
    if (fs.fail()) {
        std::cerr << "Cannot open " << model_path + "/tokenizer.json" << std::endl;
        exit(1);
    }
    std::string data;
    fs.seekg(0, std::ios::end);
    size_t size = static_cast<size_t>(fs.tellg());
    fs.seekg(0, std::ios::beg);
    data.resize(size);
    fs.read(data.data(), size);
    this->tokenizer = tokenizers::Tokenizer::FromBlobJSON(data);
    fs.close();
    std::string decoder_type;
    nlohmann::json data_json = nlohmann::json::parse(data);
    JSON_GET(decoder_type, data_json["decoder"], "type", "ByteLevel", std::string);
    if (decoder_type == "ByteLevel") {
        this->is_doubled_encoded = true;
    }
    else {
        this->is_doubled_encoded = false;
    }
    // load tokenizer configurations
    std::ifstream fs_config(model_path + "/tokenizer_config.json", std::ios::in | std::ios::binary);
    if (fs_config.fail()) {
        std::cerr << "Cannot open " << model_path + "/tokenizer_config.json" << std::endl;
        exit(1);
    }
    std::string data_config;
    fs_config.seekg(0, std::ios::end);
    size_t size_config = static_cast<size_t>(fs_config.tellg());
    fs_config.seekg(0, std::ios::beg);
    data_config.resize(size_config);
    fs_config.read(data_config.data(), size_config);
    fs_config.close();
    auto tokenizer_config = nlohmann::json::parse(data_config);
    // check if bos_token is null
    if (tokenizer_config["bos_token"].is_null()) {
        this->has_bos_token = false;
    }
    else {
        this->has_bos_token = true;
    }
    // load chat template
    this->tmpl = std::make_unique<minja::chat_template>(
        tokenizer_config["chat_template"],
        this->has_bos_token ? tokenizer_config["bos_token"] : "",
        tokenizer_config["eos_token"]
    );
    
    if (this->has_bos_token) {
        this->bos_token_id = tokenizer_config["bos_token_id"].get<int>();
    }
    else {
        this->bos_token_id = -1;
    }
    this->eos_token = tokenizer_config["eos_token"].get<std::string>();
    for (auto& token : tokenizer_config["eos_token_id"]) {
        this->eos_token_ids.push_back(token.get<int>());
    }
    this->user_system_prompt = "";
    this->extra_context["user_system_prompt"] = this->user_system_prompt;
    this->extra_context["enable_thinking"] = false;
    JSON_GET(this->think_marker_id, tokenizer_config, "think_marker_id", -1, int);
    JSON_GET(this->boi_token, tokenizer_config, "boi_token", "", std::string);
    JSON_GET(this->eoi_token, tokenizer_config, "eoi_token", "", std::string);
    JSON_GET(this->image_token, tokenizer_config, "image_token", "", std::string);
    if (!this->boi_token.empty()) {
        assert(!this->eoi_token.empty());
        assert(!this->image_token.empty());
        this->extra_context["boi_token"] = this->boi_token;
        this->extra_context["eoi_token"] = this->eoi_token;
        this->extra_context["image_token"] = this->image_token;
    }
}

/// \brief Destructor
Tokenizer::~Tokenizer() = default;

/// \brief Make the inverse byte map
/// \return the inverse byte map
std::unordered_map<uint32_t, uint8_t> Tokenizer::make_inverse_byte_map() {
    // 1) Build the "printable" byte list bs exactly as in GPT-2/LLaMA
    std::vector<uint8_t> bs;
    for (int b = 33; b <= 126; b++) bs.push_back((uint8_t)b);
    for (int b = 161; b <= 172; b++) bs.push_back((uint8_t)b);
    for (int b = 174; b <= 255; b++) bs.push_back((uint8_t)b);

    // 2) Build the codepoint list cs (same length, but extended to 256)
    std::vector<uint32_t> cs(bs.begin(), bs.end());
    int n = 0;
    for (int b = 0; b < 256; b++) {
        if (std::find(bs.begin(), bs.end(), (uint8_t)b) == bs.end()) {
            bs.push_back((uint8_t)b);
            cs.push_back(256 + n);
            n++;
        }
    }

    // 3) Zip them into an inverse map: cp → original byte
    std::unordered_map<uint32_t, uint8_t> inv;
    for (size_t i = 0; i < bs.size(); i++) {
        inv[ cs[i] ] = bs[i];
    }
    return inv;
}

/// \brief Convert the codepoint to utf8
/// \param input the input
/// \return the utf8 string
std::string Tokenizer::cpt_to_utf8(const std::string& input) {
    static auto inv_map = this->make_inverse_byte_map();
    std::string output = "";
    size_t i = 0;
    if (!this->is_doubled_encoded) { // simply do pattern substitution of "▁" to " ", temporary solution
        std::string output = "";
        static constexpr std::string pattern = "▁";
        static constexpr size_t pattern_size = pattern.size();
        for (size_t i = 0; i < input.size(); i++) {
            if (i <= input.size() - pattern_size && input.substr(i, pattern_size) == pattern) {
                output += " ";
                i += pattern_size - 1; // -1 because the loop will increment i by 1
            }
            else {
                output += input[i];
            }
        }
        return output;
    }
    while (i < input.size()) {
        unsigned char c = input[i];
        uint32_t cp;
        size_t width;

        // --- Decode a UTF-8 codepoint ---
        if (c < 0x80) {
            cp = c;
            width = 1;
        }
        else if ((c & 0xE0) == 0xC0) {
            if (i + 1 >= input.size()) throw std::runtime_error("Truncated UTF-8, ll = 2");
            cp  = (uint32_t)(c     & 0x1F) << 6;
            cp |= (uint32_t)(input[i+1] & 0x3F);
            width = 2;
        }
        else if ((c & 0xF0) == 0xE0) {
            if (i + 2 >= input.size()) throw std::runtime_error("Truncated UTF-8, ll = 3");
            cp  = (uint32_t)(c     & 0x0F) << 12;
            cp |= (uint32_t)(input[i+1] & 0x3F) << 6;
            cp |= (uint32_t)(input[i+2] & 0x3F);
            width = 3;
        }
        else if ((c & 0xF8) == 0xF0) {
            if (i + 3 >= input.size()) throw std::runtime_error("Truncated UTF-8, ll = 4");
            cp  = (uint32_t)(c     & 0x07) << 18;
            cp |= (uint32_t)(input[i+1] & 0x3F) << 12;
            cp |= (uint32_t)(input[i+2] & 0x3F) << 6;
            cp |= (uint32_t)(input[i+3] & 0x3F);
            width = 4;
        }
        else {
            std::cout << std::hex <<"cp: " << cp << std::endl;
            throw std::runtime_error("Invalid UTF-8 byte");
        }

        // --- Look up the original byte ---
        auto it = inv_map.find(cp);
        if (it == inv_map.end()) {
            throw std::runtime_error("Codepoint not in LLaMA byte map");
        }
        output.push_back((char)it->second);

        i += width;
    }

    return output;
}

/// \brief Encode the text
/// \param text the text
/// \return the encoded tokens
std::vector<int> Tokenizer::encode(const std::string& text) {
    return this->tokenizer->Encode(text);
}

/// \brief Decode the tokens
/// \param tokens the tokens
/// \return the decoded text
std::string Tokenizer::decode(const std::vector<int>& tokens) {
    return this->tokenizer->Decode(tokens);
}

/// \brief Run time decoder
/// \param answer_token the answer token
/// \return the decoded text
std::string Tokenizer::run_time_decoder(int answer_token) {
    return this->cpt_to_utf8(this->tokenizer->IdToToken(answer_token));
}

/// \brief Apply the chat template
/// \param messages the messages
/// \param add_generation_prompt the add generation prompt
/// \return the chat template
std::string Tokenizer::apply_chat_template(nlohmann::ordered_json& messages, bool add_generation_prompt, bool enable_thinking, bool block_system_prompt) {
    minja::chat_template_inputs inputs;
    inputs.add_generation_prompt = add_generation_prompt;
    inputs.messages = messages;
    this->extra_context["enable_thinking"] = enable_thinking;
    inputs.extra_context = this->extra_context;
    return this->tmpl->apply(inputs);
}

/// \brief Set the user system prompt
/// \param user_system_prompt the user system prompt
void Tokenizer::set_user_system_prompt(const std::string& user_system_prompt) {
    this->user_system_prompt = user_system_prompt;
    this->extra_context["user_system_prompt"] = user_system_prompt;
}