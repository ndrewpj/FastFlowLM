/*!
 *  Copyright (c) 2023 by Contributors
 * \file main.cpp
 * \brief Main entry point for the FLM application
 * \author FastFlowLM Team
 * \date 2025-06-24
 * \version 0.1.6
 */
#pragma once
#include "utils/utils.hpp"
#include "chat/chat_bot.hpp"
#include "nlohmann/json.hpp"
#include "model_list.hpp"
#include "wstream_buf.hpp"
#include "model_downloader.hpp"
#include "cli_wide.hpp"
#include <codecvt>
#include <vector>

using json = nlohmann::ordered_json;

/// \brief Command types
typedef enum {
    CMD_SET,
    CMD_SHOW,
    CMD_LOAD,
    CMD_SAVE,
    CMD_CLEAR,
    CMD_BYE,
    CMD_PULL,
    CMD_HELP,
    CMD_HELP_SHOTCUT,
    CMD_STATUS
} runner_cmd_t;

/// \brief Runner class
class Runner {
    public: 
        Runner(model_list& supported_models, ModelDownloader& downloader, std::string& tag);
        void run();
    private:
        std::string tag;
        model_list supported_models;
        ModelDownloader& downloader;
        std::unique_ptr<chat_bot> chat_engine;
        int generate_limit;
        std::string system_prompt;
        // CLI instance for interactive input
        CLIWide cli;

        /// \brief Command functions
        void cmd_set(std::vector<std::string>& input_list);
        void cmd_show(std::vector<std::string>& input_list);
        void cmd_load(std::vector<std::string>& input_list);
        void cmd_save(std::vector<std::string>& input_list);
        void cmd_clear(std::vector<std::string>& input_list);
        void cmd_help(std::vector<std::string>& input_list);
        void cmd_help_shotcut(std::vector<std::string>& input_list);
        void cmd_status(std::vector<std::string>& input_list);
};
