// ===== src/metrics.cpp =====
#include "../header/metrics.hpp"
#include <vector>
#include <sstream>
#include <algorithm>

// Split a string into tokens by whitespace
static std::vector<std::string> tokenize(const std::string &s) {
    std::istringstream iss(s);
    std::vector<std::string> tokens;
    std::string t;
    while (iss >> t) tokens.push_back(t);
    return tokens;
}

// Compute length of LCS between two token sequences
static int lcs_length(const std::vector<std::string> &a,
                      const std::vector<std::string> &b) {
    size_t n = a.size(), m = b.size();
    std::vector<std::vector<int>> dp(n+1, std::vector<int>(m+1, 0));
    for (size_t i = 1; i <= n; ++i) {
        for (size_t j = 1; j <= m; ++j) {
            if (a[i-1] == b[j-1])
                dp[i][j] = dp[i-1][j-1] + 1;
            else
                dp[i][j] = std::max(dp[i-1][j], dp[i][j-1]);
        }
    }
    return dp[n][m];
}

// Rouge-L F1 computation
double computeRougeL(const std::string &pred, const std::string &ref) {
    auto p_tokens = tokenize(pred);
    auto r_tokens = tokenize(ref);
    if (p_tokens.empty() || r_tokens.empty()) return 0.0;
    int lcs = lcs_length(p_tokens, r_tokens);
    double prec = static_cast<double>(lcs) / p_tokens.size();
    double rec  = static_cast<double>(lcs) / r_tokens.size();
    if (prec + rec == 0.0) return 0.0;
    return 2.0 * prec * rec / (prec + rec);
}