// ===== src/search_and_summary.cpp =====

#include "../header/config.hpp"
#include "../header/tokenizer.hpp"
#include "../header/model.hpp"
#include "../header/evaluator.hpp"

#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <vector>
#include <map>

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: eapo_search <config.json>\n";
        return 1;
    }
    // Load configuration
    Config cfg = Config::load(argv[1]);

    // Build trial list (grid search)
    std::vector<std::map<std::string, std::string>> trials;
    for (const auto &style : cfg.prompt_space.at("style")) {
        for (const auto &reasoning : cfg.prompt_space.at("reasoning")) {
            for (const auto &fmt : cfg.prompt_space.at("format")) {
                for (const auto &brev : cfg.prompt_space.at("brevity")) {
                    trials.push_back({
                        {"style",     style},
                        {"reasoning", reasoning},
                        {"format",    fmt},
                        {"brevity",   brev}
                    });
                    if (trials.size() >= (size_t)cfg.num_trials) break;
                }
                if (trials.size() >= (size_t)cfg.num_trials) break;
            }
            if (trials.size() >= (size_t)cfg.num_trials) break;
        }
        if (trials.size() >= (size_t)cfg.num_trials) break;
    }

    // Open output CSV
    std::ofstream csvOut(cfg.results_dir + "/trials.csv");
    if (!csvOut) {
        std::cerr << "Failed to open output: " << cfg.results_dir << "/trials.csv\n";
        return 1;
    }
    csvOut << "style,reasoning,format,brevity,rougeL,energy_J,latency_s,tpj\n";

    // Initialize evaluator
    Tokenizer   tokenizer(cfg.tokenizer_path);
    Model       model(cfg.model_path);
    Evaluator   evaluator(tokenizer, model, cfg);

    // Loop over trials
    for (const auto &pcfg : trials) {
        // Convert prompt config to JSON string
        nlohmann::json jcfg = pcfg;
        std::string promptJson = jcfg.dump();

        // Evaluate summary metrics for this prompt
        Evaluator::SummaryMetrics summary = evaluator.evaluateSummary(promptJson);

        // Write a row
        csvOut
            << pcfg.at("style")           << ','
            << pcfg.at("reasoning")       << ','
            << pcfg.at("format")          << ','
            << pcfg.at("brevity")         << ','
            << summary.rougeL             << ','
            << summary.energyTotalJ       << ','
            << summary.latencyS           << ','
            << summary.tokensPerJoule     << '\n';
    }

    csvOut.close();
    std::cout << "Search complete. Results in " << cfg.results_dir << "/trials.csv\n";
    return 0;
}
