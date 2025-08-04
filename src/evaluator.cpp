// ===== src/evaluator.cpp =====
#include "../header/evaluator.hpp"

#include <nvml.h>
#include <fstream>
#include <chrono>
#include <map>

Evaluator::Evaluator(const Tokenizer &tokenizer,
                     Model &model,
                     const Config &config)
  : tokenizer_(tokenizer)
  , model_(model)
  , config_(config)
{
    // Initialize NVML library
    nvmlInit();
}

void Evaluator::run(const std::string &prompt_cfg_json,
                    const std::string &dataset_path,
                    const std::string &results_dir)
{
    // 1) Parse JSON prompt configuration
    auto jcfg = nlohmann::json::parse(prompt_cfg_json);

    // 2) Convert to std::map<string,string> for PromptGenerator
    std::map<std::string, std::string> cfg_map;
    for (auto& [key, val] : jcfg.items()) {
        if (val.is_string()) {
            cfg_map[key] = val.get<std::string>();
        }
    }

    // Open input JSONL dataset
    std::ifstream fin(dataset_path);
    if (!fin) {
        throw std::runtime_error("Failed to open dataset file: " + dataset_path);
    }

    // Prepare output CSV
    std::ofstream fout(results_dir + "/eval_per_example.csv");
    if (!fout) {
        throw std::runtime_error("Failed to open output CSV: " + results_dir + "/eval_per_example.csv");
    }
    fout << "doc,prompt,generated,rougeL,energy_J,latency_s,tokens,tpj\n";

    // Initialize NVML device handle
    nvmlDevice_t device;
    nvmlDeviceGetHandleByIndex(0, &device);

    // Helper to escape quotes in CSV fields
    auto escape_csv = [&](const std::string &s) {
        std::string out;
        out.reserve(s.size());
        for (char c : s) {
            if (c == '"') out += "\"\"";
            else           out += c;
        }
        return out;
    };

    std::string line;
    while (std::getline(fin, line)) {
        auto rec = nlohmann::json::parse(line);
        std::string doc = rec["doc"].get<std::string>();

        // Render prompt via your prompts.hpp
        std::string prompt = PromptGenerator::renderPrompt(doc, cfg_map);

        // Tokenize (int) → convert to int64_t
        std::vector<int> tmp_in = tokenizer_.encode(prompt);
        std::vector<int64_t> input_ids(tmp_in.begin(), tmp_in.end());

        // Sample power & timestamp before generation
        unsigned int power_mw;
        nvmlDeviceGetPowerUsage(device, &power_mw);
        double p0 = power_mw / 1000.0;
        auto t0 = std::chrono::steady_clock::now();

        // Model generates int64_t IDs
        auto output_ids = model_.generate(input_ids);

        // Sample power & timestamp after
        auto t1 = std::chrono::steady_clock::now();
        nvmlDeviceGetPowerUsage(device, &power_mw);
        double p1 = power_mw / 1000.0;

        // Compute latency & energy & tokens-per-joule
        double latency = std::chrono::duration<double>(t1 - t0).count();
        double energy  = ((p0 + p1) / 2.0) * latency;
        int    tokens  = static_cast<int>(output_ids.size());
        double tpj     = tokens / energy;

        // Convert back to int for decoding
        std::vector<int> tmp_out(output_ids.begin(), output_ids.end());
        std::string gen_text = tokenizer_.decode(tmp_out);

        // Compute Rouge-L against reference
        double rougeL = computeRougeL(
            gen_text,
            rec["ref"].get<std::string>()
        );

        // Write CSV row (fields quoted & escaped)
        fout
          << '"' << escape_csv(doc)      << "\","
          << '"' << escape_csv(prompt)   << "\","
          << '"' << escape_csv(gen_text) << "\","
          << rougeL  << ","
          << energy  << ","
          << latency << ","
          << tokens  << ","
          << tpj     << "\n";
    }

    // Clean up
    fin.close();
    fout.close();

    // TODO: summarize results (e.g., write summary.json)
}
