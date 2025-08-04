// ===== src/utils.hpp =====
#pragma once

#include <string>
#include <vector>
#include <chrono>

namespace utils {

// Read all lines from a text file into a vector of strings
std::vector<std::string> readLines(const std::string &filepath);

// Escape a CSV field (wraps in quotes and escapes internal quotes)
std::string escapeCsv(const std::string &field);

// Write CSV rows to a file (each row is a vector of fields)
void writeCsv(const std::string &filepath,
              const std::vector<std::vector<std::string>> &rows);

// Timer for measuring durations
class Timer {
public:
    Timer() : start_(std::chrono::steady_clock::now()) {}
    // Reset the timer
    void reset() { start_ = std::chrono::steady_clock::now(); }
    // Return elapsed time in seconds since creation or last reset
    double elapsed() const {
        using namespace std::chrono;
        return duration<double>(steady_clock::now() - start_).count();
    }
private:
    std::chrono::steady_clock::time_point start_;
};

} // namespace utils