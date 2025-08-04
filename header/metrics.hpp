// ===== src/metrics.hpp =====
#pragma once
#include <string>

// Compute the Rouge-L F1 score between two texts
// pred: generated text
// ref: reference text
// Returns: Rouge-L F1 value between 0 and 1

double computeRougeL(const std::string &pred, const std::string &ref);