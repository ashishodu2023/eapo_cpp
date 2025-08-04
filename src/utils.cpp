// ===== src/utils.cpp =====
#include "../header/utils.hpp"
#include <fstream>
#include <sstream>

namespace utils {

std::vector<std::string> readLines(const std::string &filepath) {
    std::vector<std::string> lines;
    std::ifstream in(filepath);
    std::string line;
    while (std::getline(in, line)) {
        lines.push_back(line);
    }
    return lines;
}

std::string escapeCsv(const std::string &field) {
    std::string out = "\"";
    for (char c : field) {
        if (c == '"') out += '"';  // double the quote
        out += c;
    }
    out += "\"";
    return out;
}

void writeCsv(const std::string &filepath,
              const std::vector<std::vector<std::string>> &rows) {
    std::ofstream out(filepath);
    for (const auto &row : rows) {
        bool first = true;
        for (const auto &field : row) {
            if (!first) out << ',';
            out << escapeCsv(field);
            first = false;
        }
        out << '\n';
    }
}

} // namespace utils