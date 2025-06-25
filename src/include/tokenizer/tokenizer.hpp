/// \file tokenizer.hpp
/// \brief tokenizer class
/// \author FastFlowLM Team
/// \date 2025-06-24
/// \version 0.1.0
/// \note This class is used to tokenize the text.
#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <codecvt>
#include <locale>
#include "nlohmann/json.hpp"
#include "tokenizers_cpp.h"

/// \brief Token struct
/// \param text the text
/// \param token_id the token id
typedef struct {
    std::string text;
    int token_id;
} Token;

/// \brief TokenPair struct
/// \param text the text
/// \param token_id the token id
typedef std::pair<std::string, std::string> TokenPair;

/// \brief Tokenizer class
class Tokenizer {
public:
    /// \brief Constructor
    /// \param model_path the model path
    Tokenizer(const std::string& model_path);

    /// \brief Destructor
    ~Tokenizer();

    /// \brief Encode the text
    /// \param text the text
    /// \return the encoded tokens
    std::vector<int> encode(const std::string& text);

    /// \brief Decode the tokens
    /// \param tokens the tokens
    /// \return the decoded text
    std::string decode(const std::vector<int>& tokens);

    /// \brief Run time decoder
    /// \param answer_token the answer token
    /// \return the decoded text
    std::string run_time_decoder(int answer_token);

    /// \brief Check if the token is EOS
    /// \param token the token
    /// \return true if the token is EOS, false otherwise
    inline bool is_eos(int token) {return ((token == 128000) || (token == 128001) || (token == 128008) || (token == 128009));}

    /// \brief Check if the token is normal
    /// \param token the token
    /// \return true if the token is normal, false otherwise
    inline bool is_normal_token(int token) {return ((token < 128000) || (token == 128013) || (token == 128014));}

    /// \brief Get the begin of text id
    /// \return the begin of text id
    inline int begin_of_text_id() {return 128000;}

    /// \brief Get the begin of header id
    /// \return the begin of header id
    inline int begin_of_header_id() {return 128006;}

    /// \brief Get the end of header id
    /// \return the end of header id
    inline int end_of_header_id() {return 128007;}

    /// \brief Get the user id
    /// \return the user id
    inline int user_id() {return 882;}

    /// \brief Get the assistant id
    /// \return the assistant id
    inline int assistant_id() {return 78191;}

    /// \brief Get the end of text id
    /// \return the end of text id
    inline int end_of_text_id() {return 128001;}

private:
    std::unique_ptr<tokenizers::Tokenizer> tokenizer;
    std::unordered_map<uint32_t, uint8_t> inv_map;

    /// \brief Convert the cp1252 to utf8
    /// \param input the input string
    /// \return the utf8 string
    std::string cpt_to_utf8(const std::string& input);

    /// \brief Make the inverse byte map
    /// \return the inverse byte map
    std::unordered_map<uint32_t, uint8_t> make_inverse_byte_map();
};
