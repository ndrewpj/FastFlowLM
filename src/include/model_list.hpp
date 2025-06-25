/// \file model_list.hpp
/// \brief model_list class
/// \author FastFlowLM Team
/// \date 2025-06-24
/// \version 1.0
/// \note This class is used to manage the model list.
#pragma once
#include "nlohmann/json.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "utils/utils.hpp"

/// \brief model_list class
/// \author FastFlowLM Team
/// \date 2025-06-24
/// \version 1.0
/// \note This class is used to manage the model list.
class model_list {
    public:
        /// \brief files required for the model
        static constexpr const char* model_files[] = {
            "config.json",
            "tokenizer.json",
            "attn.xclbin",
            "mm.xclbin",
            "dequant.xclbin",
            "layer.xclbin",
            "lm_head.xclbin",
            "model.q4nx"
        };
        /// \brief number of model files
        static constexpr int model_files_count = sizeof(model_files) / sizeof(model_files[0]);

        /// \brief constructor
        model_list(){}

        /// \brief constructor
        /// \param list_path the path to the model list
        /// \param exe_dir the executable directory for resolving relative paths
        model_list(std::string& list_path, std::string& exe_dir){
            this->list_path = list_path;
            std::ifstream config_file(list_path);
            if (!config_file.is_open()) {
                std::cerr << "Failed to open config file: " << list_path << std::endl;
                exit(1);
            }
            this->config = nlohmann::json::parse(config_file);
            // Resolve model_root_path relative to executable directory
            std::string relative_model_path = this->config["model_path"];
            this->model_root_path = exe_dir + "/" + relative_model_path;
            config_file.close();
        }

        /// \brief get the model info
        /// \param tag the tag of the model
        /// \return the model info
        nlohmann::json get_model_info(const std::string& tag){
            bool model_found = false;
            // get model type, the string before ':' in the tag
            std::string model_type;
            std::string model_size;

            if (tag.find(':') != std::string::npos) {
                model_type = tag.substr(0, tag.find(':'));
                model_size = tag.substr(tag.find(':') + 1);
            }
            else {
                model_type = tag;
                model_size = "";
            }
            
            // find the model subset first, compare with the key of the model
            bool model_subset_found = false;
            for (const auto& [key, model] : this->config["models"].items()) {
                if (key == model_type) {
                    model_subset_found = true;
                    break;
                }
            } 
            if (model_subset_found) {
                // find the model in the subset
                // check if a size is specified in the tag
                // if not use the first model in the subset
                if (model_size.empty()) {
                    model_size = this->config["models"][model_type].begin().key();
                    return this->config["models"][model_type][model_size];
                }
                bool model_found = false;
                for (const auto& [key, model] : this->config["models"][model_type].items()) {
                    if (key == model_size) { // if the size is found, return the model
                        model_found = true;
                        return model;
                    }
                } 
                if (!model_found) {
                    header_print("ERROR", "Model not found: " + model_size + " in subset " + model_type);
                    header_print("ERROR", "Using default model: llama3.2-1B");
                    return this->config["models"]["llama3.2"]["1B"];
                }
            }
            else{
                header_print("ERROR", "Model subset not found: " + model_type);
                exit(1);
                return this->config["models"]["llama3.2"]["1B"];
            }
        }

        /// \brief get the model root path
        /// \return the model root path, string
        std::string get_model_root_path(){
            return this->model_root_path;
        }

        /// \brief get all the models
        /// \return all the models in json
        nlohmann::json get_all_models(){
            nlohmann::json response = {
                {"models", nlohmann::json::array()}
            };

            for (const auto& [model_type, model_subset] : this->config["models"].items()) {
                for (const auto& [size, model_info] : model_subset.items()) {
                    nlohmann::json model_entry = {
                        {"name", model_type + ":" + size},
                        {"model", model_type + ":" + size},
                        {"modified_at", "2024-03-28T00:00:00Z"},
                        {"details", {
                            {"format", "gguf"},
                            {"family", model_type},
                            {"parameter_size", size},
                            {"quantization_level", "Q4_0"}
                        }}
                    };
                    response["models"].push_back(model_entry);
                }
            }
            return response;
        }

        /// \brief get the model path
        /// \param tag the tag of the model
        /// \return the model path, string
        std::string get_model_path(const std::string& tag){
            std::string model_name = this->get_model_info(tag)["name"];
            std::string model_path = this->model_root_path + "/" + model_name;
            return model_path;
        }
    private:
        std::string list_path;
        nlohmann::json config;
        std::string model_root_path;
};