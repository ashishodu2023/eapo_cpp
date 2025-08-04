#pragma once

#include <string>
#include <nlohmann/json.hpp>
#ifdef USE_NVML
#include <nvml.h>
#endif

#include "tokenizer.hpp"
#include "model.hpp"
#include "config.hpp"
#include "metrics.hpp"
#include "prompts.hpp"

/**
 * Evaluator: runs inference over a JSONL dataset, logs energy & latency,
 * computes per-example metrics (CSV) and can also aggregate summary metrics.
 */
class Evaluator {
public:
    /// Construct with tokenizer, model, and config
    Evaluator(const Tokenizer &tokenizer,
              Model &model,
              const Config &config);

    /**
     * Run detailed evaluation for a given prompt config.
     * Writes per-example CSV (`eval_per_example.csv`) under `results_dir`.
     */
    void run(const std::string &prompt_cfg_json,
             const std::string &dataset_path,
             const std::string &results_dir);

    /// Aggregated metrics over the dataset for one prompt config
    struct SummaryMetrics {
        double rougeL;         ///< average Rouge-L F1 score
        double energyTotalJ;   ///< total energy consumed (J)
        double latencyS;       ///< total latency (seconds)
        double tokensPerJoule; ///< tokens generated per joule
    };

    /**
     * Evaluate and return summary metrics without writing per-example output.
     */
    SummaryMetrics evaluateSummary(const std::string &prompt_cfg_json);

private:
    const Tokenizer &tokenizer_;  ///< tokenizer for encode/decode
    Model           &model_;      ///< model for generation
    Config           config_;     ///< configuration (paths, prompt space)
};
