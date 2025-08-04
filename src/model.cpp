// ===== src/model.cpp =====
#include "../header/model.hpp"
#include <new>
#include <torch/torch.h>
#include <stdexcept>

Model::Model(const std::string &model_path) {
    try {
        // Deserialize the ScriptModule from file
        module_ = torch::jit::load(model_path);
        // Move to GPU if available
        if (torch::cuda::is_available()) {
            module_.to(torch::kCUDA);
        }
        module_.eval();
    } catch (const c10::Error &e) {
        throw std::runtime_error("Error loading the model from " + model_path + ": " + e.what());
    }
}

std::vector<int64_t> Model::generate(
    const std::vector<int64_t> &input_ids,
    int max_new_tokens
) {
    // Prepare input tensor [1, seq_len]
    auto options = torch::TensorOptions().dtype(torch::kInt64);
    if (torch::cuda::is_available()) {
        options = options.device(torch::kCUDA);
    }
    torch::Tensor ids = torch::tensor(input_ids, options).unsqueeze(0);

    std::vector<int64_t> output_ids = input_ids;
    for (int i = 0; i < max_new_tokens; ++i) {
        // Forward pass
        std::vector<torch::IValue> inputs;
        inputs.push_back(ids);
        at::Tensor logits = module_.forward(inputs).toTensor();
        // logits shape [1, seq_len, vocab_size]
        at::Tensor next_token_logits = logits[0][-1]; // last timestep
        int64_t next_id = next_token_logits.argmax().item<int64_t>();

        // Append to outputs and input tensor
        output_ids.push_back(next_id);
        torch::Tensor next_tensor = torch::tensor({next_id}, options);
        ids = torch::cat({ids, next_tensor.unsqueeze(0)}, /*dim=*/1);
    }
    return output_ids;
}
