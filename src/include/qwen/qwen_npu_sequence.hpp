/// \file qwen_npu_sequence.hpp
/// \brief qwen_npu_sequence class
/// \author FastFlowLM Team
/// \date 2025-06-24
/// \version 0.9.2
/// \note This is a header file for the qwen_npu_sequence class
#pragma once
#include "npu_utils/npu_instr_utils.hpp"
#include "lm_config.hpp"

/// \brief qwen_npu_sequence class
/// \note This is a class for the qwen_npu_sequence
class qwen_npu_sequence{
public:
    qwen_npu_sequence(){}

    /// \brief Constructor
    /// \param config the configuration
    /// \param MAX_L the max length
    qwen_npu_sequence(LM_Config config, uint32_t MAX_L);
    ~qwen_npu_sequence();

    /// \brief Generate the rtp sequence
    /// \param seq the sequence
    /// \param L the length
    void gen_rtp_seq(npu_sequence* seq, const uint32_t L);

    /// \brief Generate the layer sequence
    /// \param seq the sequence
    /// \param L the length
    void gen_layer_seq(npu_sequence* seq, const uint32_t L);

    /// \brief Set the max length
    /// \param MAX_L the max length
    void set_max_length(const uint32_t MAX_L);

    /// \brief Generate the mha engine sequence
    /// \param seq the sequence
    /// \param L_begin the begin length
    /// \param L_end the end length
    void gen_mha_engine_seq(npu_sequence* seq, const uint32_t L_begin, const uint32_t L_end);

    /// \brief Get the k03 offset
    size_t get_k03_offset() const;
    size_t get_k47_offset() const;
    size_t get_v03_offset() const;
    size_t get_v47_offset() const;

private:
    struct Impl;
    Impl* _impl;
};
