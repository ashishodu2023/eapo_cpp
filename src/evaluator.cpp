// ===== src/evaluator.cpp =====

#include "../header/evaluator.hpp"
#include <fstream>
#include <chrono>

#ifdef USE_NVML
#include <nvml.h>
#endif

Evaluator::Evaluator(const Tokenizer &tokenizer,
                     Model &model,
                     const Config &config)
  : tokenizer_(tokenizer)
  , model_(model)
  , config_(config)
{
#ifdef USE_NVML
    nvmlInit();
#endif
}

void Evaluator::run(const std::string &prompt_cfg_json,
                    const std::string &dataset_path,
                    const std::string &results_dir)
{
    // 1) Parse JSON prompt configuration
    auto jcfg = nlohmann::json::parse(prompt_cfg_json);

    // 2) Convert to std::map<string,string> for PromptGenerator
    std::map<std::string, std::string> cfg_map;
    for (auto &item : jcfg.items()) {
        if (item.value().is_string()) {
            cfg_map[item.key()] = item.value().get<std::string>();
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

#ifdef USE_NVML
    // Initialize NVML device handle
    nvmlDevice_t device;
    nvmlDeviceGetHandleByIndex(0, &device);
#endif

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

        // Render prompt
        std::string prompt = PromptGenerator::renderPrompt(doc, cfg_map);

        // Tokenize: int → int64_t
        auto tmp_in = tokenizer_.encode(prompt);
        std::vector<int64_t> input_ids(tmp_in.begin(), tmp_in.end());

        // Sample power & timestamp before
#ifdef USE_NVML
        unsigned int p0_mw;
        nvmlDeviceGetPowerUsage(device, &p0_mw);
        double p0 = p0_mw / 1000.0;
#endif
        auto t0 = std::chrono::steady_clock::now();

        // Generate
        auto output_ids = model_.generate(input_ids);

        // Sample power & timestamp after
        auto t1 = std::chrono::steady_clock::now();
#ifdef USE_NVML
        unsigned int p1_mw;
        nvmlDeviceGetPowerUsage(device, &p1_mw);
        double p1 = p1_mw / 1000.0;
#endif

        // Compute metrics
        double latency = std::chrono::duration<double>(t1 - t0).count();
        double energy  =
#ifdef USE_NVML
            ((p0 + p1) / 2.0) * latency;
#else
            0.0;
#endif
        int tokens = static_cast<int>(output_ids.size());
        double tpj = (energy > 0.0 ? tokens / energy : 0.0);

        // Decode
        std::vector<int> tmp_out(output_ids.begin(), output_ids.end());
        std::string gen_text = tokenizer_.decode(tmp_out);

        // Rouge-L
        double rougeL = computeRougeL(gen_text, rec["ref"].get<std::string>());

        // Write CSV row
        fout
          << '"' << escape_csv(doc)      << "\","
          << '"' << escape_csv(prompt)   << "\","
          << '"' << escape_csv(gen_text) << "\","
          << rougeL   << ","
          << energy   << ","
          << latency  << ","
          << tokens   << ","
          << tpj      << "\n";
    }

    fin.close();
    fout.close();

    // TODO: optionally write summary.json here

#ifdef USE_NVML
    nvmlShutdown();
#endif
}

// ---- evaluateSummary implementation ----

Evaluator::SummaryMetrics
Evaluator::evaluateSummary(const std::string &prompt_cfg_json)
{
    // Parse prompt config JSON
    auto jcfg = nlohmann::json::parse(prompt_cfg_json);
    std::map<std::string, std::string> cfg_map;
    for (auto &item : jcfg.items()) {
        if (item.value().is_string()) {
            cfg_map[item.key()] = item.value().get<std::string>();
        }
    }

    std::ifstream fin(config_.dataset_path);
    if (!fin) {
        throw std::runtime_error("Cannot open dataset: " + config_.dataset_path);
    }

#ifdef USE_NVML
    nvmlInit();
    nvmlDevice_t device;
    nvmlDeviceGetHandleByIndex(0, &device);
#endif

    size_t count = 0;
    double sumRouge = 0.0, sumEnergy = 0.0, sumLatency = 0.0;
    size_t sumTokens = 0;
    std::string line;

    while (std::getline(fin, line)) {
        auto rec = nlohmann::json::parse(line);
        std::string doc = rec["doc"].get<std::string>();

        // Build prompt & tokenize
        std::string prompt = PromptGenerator::renderPrompt(doc, cfg_map);
        auto tmp_in = tokenizer_.encode(prompt);
        std::vector<int64_t> input_ids(tmp_in.begin(), tmp_in.end());

        // Power & time before
#ifdef USE_NVML
        unsigned int p0_mw;
        nvmlDeviceGetPowerUsage(device, &p0_mw);
        double p0 = p0_mw / 1000.0;
#endif
        auto t0 = std::chrono::steady_clock::now();

        // Generate
        auto output_ids = model_.generate(input_ids);

        auto t1 = std::chrono::steady_clock::now();
#ifdef USE_NVML
        unsigned int p1_mw;
        nvmlDeviceGetPowerUsage(device, &p1_mw);
        double p1 = p1_mw / 1000.0;
#endif

        double latency = std::chrono::duration<double>(t1 - t0).count();
        double energy  =
#ifdef USE_NVML
            ((p0 + p1) / 2.0) * latency;
#else
            0.0;
#endif
        sumLatency += latency;
        sumEnergy  += energy;
        sumTokens  += output_ids.size();

        // Decode & Rouge
        std::vector<int> tmp_out(output_ids.begin(), output_ids.end());
        std::string gen_text = tokenizer_.decode(tmp_out);
        double rouge   = computeRougeL(gen_text, rec["ref"].get<std::string>());
        sumRouge += rouge;

        ++count;
    }

#ifdef USE_NVML
    nvmlShutdown();
#endif

    Evaluator::SummaryMetrics m;
    m.rougeL         = (count ? sumRouge / count : 0.0);
    m.energyTotalJ   = sumEnergy;
    m.latencyS       = sumLatency;
    m.tokensPerJoule = (sumEnergy>0.0 ? sumTokens / sumEnergy : 0.0);

    return m;
}
