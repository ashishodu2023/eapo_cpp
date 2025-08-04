// ===== src/model.hpp =====
#pragma once

#include <string>
#include <vector>
#include <new>
#include <torch/script.h>

// Wrapper around a TorchScript causal language model for generation
class Model {
public:
    // Load a serialized TorchScript model (.pt)
    explicit Model(const std::string &model_path);

    // Generate output token IDs given input IDs
    // - input_ids: vector of token IDs (1D)
    // - max_new_tokens: number of tokens to generate beyond inputs
    // Returns: full sequence of output token IDs (including input prefix)
    std::vector<int64_t> generate(
        const std::vector<int64_t> &input_ids,
        int max_new_tokens = 50
    );

private:
    torch::jit::script::Module module_;
};