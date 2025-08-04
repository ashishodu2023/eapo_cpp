#pragma once

#ifdef USE_SENTENCEPIECE
# include <sentencepiece_processor.h>
#else
# error "SentencePiece support disabled. Define USE_SENTENCEPIECE and install libsentencepiece-dev."
#endif

#include <stdexcept>
#include <vector>
#include <string>

class Tokenizer {
  sentencepiece::SentencePieceProcessor sp_;
public:
  explicit Tokenizer(const std::string &model_path) {
    // Load() now returns a sentencepiece::util::Status
    if (!sp_.Load(model_path).ok()) {
      throw std::runtime_error("Failed to load SPM model");
    }
  }

  std::vector<int> encode(const std::string &text) const {
    std::vector<int> ids;
    sp_.Encode(text, &ids);
    return ids;
  }

  std::string decode(const std::vector<int> &ids) const {
    std::string out;
    // Decode(ids, &out) is the current API
    sp_.Decode(ids, &out);
    return out;
  }
};
