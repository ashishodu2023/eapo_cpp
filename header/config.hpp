// ===== src/config.hpp =====
#pragma once

#include <string>
#include <map>
#include <vector>

// Config struct: loads JSON configuration
// from a file using nlohmann::json
struct Config {
    std::string model_path;
    std::string tokenizer_path;
    std::string dataset_path;
    std::string results_dir;
    int num_trials;
    // Prompt space definitions
    std::map<std::string, std::vector<std::string>> prompt_space;

    // Load configuration from JSON file
    static Config load(const std::string &filename);
};

