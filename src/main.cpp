#include "../header/config.hpp"
#include "../header/tokenizer.hpp"
#include "../header/model.hpp"
#include "../header/evaluator.hpp"
#include "../header/prompts.hpp"
#include <boost/program_options.hpp>
#include <nlohmann/json.hpp>
#include <iostream>

namespace po = boost::program_options;

int main(int argc, char* argv[]) {
    try {
        // CLI options
        po::options_description desc("EAPO_Cpp Options");
        desc.add_options()
            ("help,h", "Print help messages")
            ("config,c", po::value<std::string>()->required(), "Path to config JSON file")
            ("mode,m", po::value<std::string>()->required(), "Operation mode: search or evaluate")
            ("prompt,p", po::value<std::string>(), "Prompt config JSON string for evaluation mode")
            ("trials,t", po::value<int>()->default_value(10), "Number of trials for search mode");

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);

        if (vm.count("help")) {
            std::cout << desc << std::endl;
            return 0;
        }
        po::notify(vm);

        // Load configuration
        std::string cfg_path = vm["config"].as<std::string>();
        Config cfg = Config::load(cfg_path);

        std::string mode = vm["mode"].as<std::string>();
        if (mode == "search") {
            int n_trials = vm["trials"].as<int>();
            std::cout << "[Search] Running " << n_trials << " trials...\n";
            // TODO: implement prompt search using cfg.prompt_space
            // For now, stub out
            std::cout << "Search not implemented yet.\n";

        } else if (mode == "evaluate") {
            if (!vm.count("prompt")) {
                std::cerr << "Error: --prompt is required in evaluate mode.\n";
                return 1;
            }
            std::string prompt_cfg_json = vm["prompt"].as<std::string>();
            // Initialize components
            Tokenizer tokenizer(cfg.tokenizer_path);
            Model model(cfg.model_path);
            Evaluator evaluator(tokenizer, model, cfg);

            // Run evaluation
            evaluator.run(prompt_cfg_json, cfg.dataset_path, cfg.results_dir);

        } else {
            std::cerr << "Error: Unknown mode '" << mode << "'. Use 'search' or 'evaluate'.\n";
            return 1;
        }

    } catch (const po::error &ex) {
        std::cerr << "Command line error: " << ex.what() << std::endl;
        return 1;
    } catch (const std::exception &ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
