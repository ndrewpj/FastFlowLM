/// \file tokenizer.hpp
/// \brief tokenizer class
/// \author FastFlowLM Team
/// \date 2025-06-24
/// \version 0.1.0
/// \note This class is used to tokenize the text.
#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include "nlohmann/json.hpp"
#include "tokenizers_cpp.h"
#include "minja/chat-template.hpp"
#include "typedef.hpp"

/// \brief Token struct
/// \param text the text
/// \param token_id the token id
typedef struct {
    std::string text;
    int token_id;
} Token;

/// \brief Role type
/// \param USER the user
/// \param ASSISTANT the assistant
/// \param SYSTEM the system
typedef enum{
    USER,
    ASSISTANT,
    SYSTEM,
    PLAIN_TEXT
} role_type;


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
    /// \param role the role, USER, ASSISTANT, SYSTEM, PLAIN_TEXT
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
    inline bool is_eos(int token) {
        return std::find(eos_token_ids.begin(), eos_token_ids.end(), token) != eos_token_ids.end();
    }

    /// \brief Check if the token is normal
    /// \param token the token
    /// \param is_think_model the is think model
    /// \return true if the token is normal, false otherwise
    inline bool is_normal_token(int token) {
        if (token == this->bos_token_id){
            return false;
        }
        else {
            for (auto& id : this->eos_token_ids){
                if (token == id){
                    return false;
                }
            }
        }
        return true;
    }
    
    /// \brief Apply the chat template
    /// \param tokens the tokens
    /// \param role the role
    /// \return the tokens with template applied
    std::string apply_chat_template(nlohmann::ordered_json& messages, bool add_generation_prompt, bool block_system_prompt = false);

    /// \brief Set the user system prompt
    /// \param user_system_prompt the user system prompt
    void set_user_system_prompt(const std::string& user_system_prompt);

    /// \brief Get the think marker id
    /// \return the think marker id
    int get_think_marker_id() const {
        return this->think_marker_id;
    }

private:
    std::unique_ptr<tokenizers::Tokenizer> tokenizer;
    std::unordered_map<uint32_t, uint8_t> inv_map;
    std::string bos_token;
    std::string eos_token;
    int bos_token_id;
    int think_marker_id;
    std::vector<int> eos_token_ids;
    std::unique_ptr<minja::chat_template> tmpl;
    std::string user_system_prompt;
    nlohmann::json extra_context;

    /// \brief Convert the cp1252 to utf8
    /// \param input the input string
    /// \return the utf8 string
    std::string cpt_to_utf8(const std::string& input);

    /// \brief Make the inverse byte map
    /// \return the inverse byte map
    std::unordered_map<uint32_t, uint8_t> make_inverse_byte_map();
};
