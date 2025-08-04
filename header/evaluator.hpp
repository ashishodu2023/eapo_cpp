#pragma once

#include <string>
#include <nlohmann/json.hpp>
#include <nvml.h>

#include "tokenizer.hpp"   // Tokenizer class
#include "model.hpp"       // Model class/interface
#include "config.hpp"      // Config struct/class
#include "metrics.hpp"     // computeRougeL(...)
#include "prompts.hpp"     // PromptGenerator

/// Evaluator: runs inference over a JSONL dataset, logs energy & latency, computes metrics
class Evaluator {
public:
    /// Constructor takes references to your tokenizer & model, plus a copy of config
    Evaluator(const Tokenizer &tokenizer,
              Model &model,
              const Config &config);

    /**
     * Run evaluation for a given prompt configuration and dataset.
     *
     * @param prompt_cfg_json JSON string specifying prompt parameters
     * @param dataset_path    Path to a JSONL file, one record per line: { "doc": "...", "ref": "..." }
     * @param results_dir     Directory to write:
     *                        - eval_per_example.csv
     *                        - (later) summary.json
     */
    void run(const std::string &prompt_cfg_json,
             const std::string &dataset_path,
             const std::string &results_dir);

private:
    const Tokenizer &tokenizer_;  ///< for encode/decode
    Model           &model_;      ///< for generation
    Config           config_;     ///< prompt settings
};
