#pragma once

#include <string>
#include <map>
#include <vector>
#include <sstream>

// PromptGenerator: builds instruction prompts based on configuration
// cfg keys: "style", "reasoning", "format", "brevity"
// doc: the input document text
class PromptGenerator {
public:
    // Render a prompt given the document and config map
    static std::string renderPrompt(
        const std::string &doc,
        const std::map<std::string, std::string> &cfg
    ) {
        std::vector<std::string> fragments;
        auto get = [&](const std::string &key) {
            auto it = cfg.find(key);
            return it != cfg.end() ? it->second : std::string();
        };
        std::string style     = get("style");
        std::string reasoning = get("reasoning");
        std::string fmt       = get("format");
        std::string brevity   = get("brevity");

        // Style instructions
        if (style == "concise") {
            fragments.push_back("Please summarize the following text.");
        } else if (style == "role") {
            fragments.push_back("You are a summarization expert. Please summarize.");
        } else if (style == "stepwise") {
            fragments.push_back("Summarize step by step:");
        } else if (style == "few-shot") {
            fragments.push_back("Example:\nText: ... Summary: ...\nNow you: summarize the following text.");
        } else if (style == "chain-of-thought") {
            fragments.push_back("Think step by step, then summarize:");
        }
        // Reasoning cues
        if (reasoning == "brief") {
            fragments.push_back("Provide a brief rationale.");
        } else if (reasoning == "bounded") {
            fragments.push_back("Explain concisely why you chose this summary.");
        } else if (reasoning == "detailed") {
            fragments.push_back("Provide a detailed explanation of your reasoning.");
        }
        // Format directives
        if (fmt == "bullets") {
            fragments.push_back("Use bullet points.");
        } else if (fmt == "json") {
            fragments.push_back("Output in valid JSON format.");
        } else if (fmt == "table") {
            fragments.push_back("Present results in a table.");
        }
        // Brevity constraints
        if (brevity == "1sent") {
            fragments.push_back("Limit your summary to exactly one sentence.");
        } else if (brevity == "3sent") {
            fragments.push_back("Limit your summary to up to three sentences.");
        } else if (brevity == "word50") {
            fragments.push_back("Limit your summary to 50 words or fewer.");
        } else if (brevity == "token50") {
            fragments.push_back("Limit your summary to 50 tokens or fewer.");
        }

        // Join fragments into instruction
        std::ostringstream instr;
        for (size_t i = 0; i < fragments.size(); ++i) {
            instr << fragments[i];
            if (i + 1 < fragments.size()) instr << ' ';
        }

        // Build final prompt
        std::ostringstream prompt;
        prompt << instr.str()
               << "\n\nInput: " << doc
               << "\nOutput:";
        return prompt.str();
    }
};
