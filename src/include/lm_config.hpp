/// \file lm_config.hpp
/// \brief lm_config class
/// \author FastFlowLM Team
/// \date 2025-08-05
/// \version 0.9.2
/// \note This class is used to store the model configuration.
#pragma once

#include "typedef.hpp"
#include "nlohmann/json.hpp"

/// \brief LM_Config class
class LM_Config{
    public:
        std::string model_path;
        std::string model_name;
        std::string model_type;
        u32 head_dim;
        u32 hidden_size;
        std::string hidden_act;
        u32 intermediate_size;
        u32 num_attention_heads;
        u32 num_hidden_layers;
        u32 num_key_value_heads;
        u32 pretraining_tp;
        f32 rms_norm_eps;
        f32 rope_theta;
        u32 vocab_size;
        u32 sliding_window;
        u32 sliding_window_pattern;
        u32 addr_qk;
        u32 addr_kv;
        u32 addr_l_begin_mha;
        u32 addr_l_end_mha;
        u32 addr_kk;
        std::string layer_xclbin_name;
        std::string lm_head_xclbin_name;
        std::string dequant_xclbin_name;
        std::string mm_engine_xclbin_name;
        std::string mha_engine_xclbin_name;
        std::string flm_version;
        nlohmann::json _json_config;

        /// \brief from pretrained
        /// \param model_name the model name
        void from_pretrained(std::string model_name){
            this->model_path = model_name;
            this->model_name = model_name;
            std::ifstream file(model_name + "/config.json");
            if (!file.is_open()){
                std::cerr << "Failed to open file: " << model_name << std::endl;
                exit(1);
            }
            // read the json file as a string
            std::string json_str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            this->_json_config = nlohmann::json::parse(json_str);
            JSON_GET(this->sliding_window, this->_json_config, "sliding_window", 0, u32);
            JSON_GET(this->sliding_window_pattern, this->_json_config, "sliding_window_pattern", 0, u32);
            JSON_GET(this->model_type, this->_json_config, "model_type", "", std::string);
            JSON_GET(this->head_dim, this->_json_config, "head_dim", 0, u32);
            JSON_GET(this->hidden_size, this->_json_config, "hidden_size", 0, u32);
            JSON_GET(this->hidden_act, this->_json_config, "hidden_act", "", std::string);
            JSON_GET(this->intermediate_size, this->_json_config, "intermediate_size", 0, u32);
            JSON_GET(this->num_attention_heads, this->_json_config, "num_attention_heads", 0, u32);
            JSON_GET(this->num_hidden_layers, this->_json_config, "num_hidden_layers", 0, u32);
            JSON_GET(this->num_key_value_heads, this->_json_config, "num_key_value_heads", 0, u32);
            JSON_GET(this->pretraining_tp, this->_json_config, "pretraining_tp", 0, u32);
            JSON_GET(this->rms_norm_eps, this->_json_config, "rms_norm_eps", 0.0, f32);
            JSON_GET(this->rope_theta, this->_json_config, "rope_theta", 0.0, f32);
            JSON_GET(this->vocab_size, this->_json_config, "vocab_size", 0, u32);
            JSON_GET(this->addr_qk, this->_json_config, "addr_qk", 0, u32);
            JSON_GET(this->addr_kv, this->_json_config, "addr_kv", 0, u32);
            JSON_GET(this->addr_l_begin_mha, this->_json_config, "addr_l_begin_mha", 0, u32);
            JSON_GET(this->addr_l_end_mha, this->_json_config, "addr_l_end_mha", 0, u32);
            JSON_GET(this->addr_kk, this->_json_config, "addr_kk", 0, u32);
            JSON_GET(this->layer_xclbin_name, this->_json_config, "layer_xclbin_name", "layer.xclbin", std::string);
            JSON_GET(this->lm_head_xclbin_name, this->_json_config, "lm_head_xclbin_name", "lm_head.xclbin", std::string);
            JSON_GET(this->dequant_xclbin_name, this->_json_config, "dequant_xclbin_name", "dequant.xclbin", std::string);
            JSON_GET(this->mm_engine_xclbin_name, this->_json_config, "mm_engine_xclbin_name", "mm.xclbin", std::string);
            JSON_GET(this->mha_engine_xclbin_name, this->_json_config, "mha_engine_xclbin_name", "attn.xclbin", std::string);
            JSON_GET(this->flm_version, this->_json_config, "flm_version", "0.0.0", std::string);
            assert(this->vocab_size > 0);
            assert(this->hidden_size > 0);
            assert(this->intermediate_size > 0);
            assert(this->num_attention_heads > 0);
            assert(this->num_hidden_layers > 0);
            assert(this->num_key_value_heads > 0);
            assert(this->rms_norm_eps > 0);
            assert(this->addr_qk > 0);
            assert(this->addr_kv > 0);
            this->layer_xclbin_name = this->model_path + "/" + this->layer_xclbin_name;
            this->lm_head_xclbin_name = this->model_path + "/" + this->lm_head_xclbin_name;
            this->dequant_xclbin_name = this->model_path + "/" + this->dequant_xclbin_name;
            this->mm_engine_xclbin_name = this->model_path + "/" + this->mm_engine_xclbin_name;
            this->mha_engine_xclbin_name = this->model_path + "/" + this->mha_engine_xclbin_name;
        }
        std::string _str(){
            std::stringstream ss;
            ss << "  Model: "  << std::endl;
            ss << "    model_name:             " << this->model_name << std::endl;
            ss << "    compatible_flm_version: >= " << this->flm_version << std::endl;
            ss << "    head_dim:               " << this->head_dim << std::endl;
            ss << "    hidden_size:            " << this->hidden_size << std::endl;
            ss << "    hidden_act:             " << this->hidden_act << std::endl;
            ss << "    intermediate_size:      " << this->intermediate_size << std::endl;
            ss << "    num_attention_heads:    " << this->num_attention_heads << std::endl;
            ss << "    num_hidden_layers:      " << this->num_hidden_layers << std::endl;
            ss << "    num_key_value_heads:    " << this->num_key_value_heads << std::endl;
            ss << "    pretraining_tp:         " << this->pretraining_tp << std::endl;
            ss << "    rms_norm_eps:           " << this->rms_norm_eps << std::endl;
            if (this->sliding_window > 0){
                ss << "    sliding_window:       " << this->sliding_window << std::endl;
                ss << "    sliding_window_pattern: " << this->sliding_window_pattern << std::endl;
            }
            return ss.str();
        }
        LM_Config(){}
};