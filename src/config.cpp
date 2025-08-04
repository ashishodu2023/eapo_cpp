
// ===== src/config.cpp =====
#include "../header/config.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <stdexcept>

Config Config::load(const std::string &filename) {
    std::ifstream in(filename);
    if (!in.is_open()) {
        throw std::runtime_error("Unable to open config file: " + filename);
    }

    nlohmann::json j;
    in >> j;
    Config cfg;

    cfg.model_path     = j.at("model_path").get<std::string>();
    cfg.tokenizer_path = j.at("tokenizer_path").get<std::string>();
    cfg.dataset_path   = j.at("dataset_path").get<std::string>();
    cfg.results_dir    = j.at("results_dir").get<std::string>();
    cfg.num_trials     = j.at("num_trials").get<int>();

    // Load prompt_space entries correctly
    for (auto &it : j.at("prompt_space").items()) {
        const std::string &key = it.key();
        const auto &val = it.value();
        cfg.prompt_space[key] = val.get<std::vector<std::string>>();
    }

    return cfg;
}
