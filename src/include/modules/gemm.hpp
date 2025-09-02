/// \file gemm.hpp
/// \brief gemm class
/// \author FastFlowLM Team
/// \date 2025-06-24
/// \version 0.9.7
/// \note This is a header file for the gemm class
#pragma once
#include "lm_config.hpp"
#include "npu_utils/npu_utils.hpp"

/// \brief gemm class
/// \note This is a class for the gemm layer
class Gemm{
public:
    Gemm(){}

    /// \brief Constructor
    /// \param config the configuration
    /// \param xclbin_name the xclbin name
    /// \param npu the npu manager
    Gemm(LM_Config& config);
    ~Gemm();

    /// \brief Generate the sequence
    void generate_seq(npu_sequence* seq, const uint32_t M, const uint32_t K, const uint32_t N, const uint32_t weight_offset);

    /// \brief get y_group_id
    /// \return the y group id
    int y_id();
    /// \brief get x_group_id
    /// \return the x group id
    int x_id();
    /// \brief get w_group_id
    /// \return the w group id
    int w_id();

private:
    struct Impl;
    Impl* _impl;

};

