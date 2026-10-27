#include "Dictionary.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <stdexcept>

namespace hangman {

namespace {

// Приводит слово к нижнему регистру, убирает пробелы по краям и '\r'.
// Возвращает пустую строку, если слово содержит что-то кроме букв a-z.
std::string normalizeWord(const std::string& raw) {
    std::string word;
    for (char c : raw) {
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            continue;
        }
        if (c >= 'A' && c <= 'Z') {
            c = static_cast<char>(c - 'A' + 'a');
        }
        if (c < 'a' || c > 'z') {
            return "";
        }
        word += c;
    }
    return word;
}

}  // namespace

Dictionary::Dictionary() : rng_(std::random_device{}()) {}

bool Dictionary::loadCategory(const std::string& category, const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return false;
    }

    std::vector<std::string> loaded;
    std::string line;
    while (std::getline(file, line)) {
        std::string word = normalizeWord(line);
        if (!word.empty()) {
            loaded.push_back(word);
        }
    }

    if (loaded.empty()) {
        return false;
    }
    words_[category] = std::move(loaded);
    return true;
}

std::size_t Dictionary::loadFromDirectory(const std::string& dirPath) {
    namespace fs = std::filesystem;

    std::error_code ec;
    if (!fs::is_directory(dirPath, ec)) {
        return 0;
    }

    std::size_t count = 0;
    for (const auto& entry : fs::directory_iterator(dirPath, ec)) {
        if (!entry.is_regular_file() || entry.path().extension() != ".txt") {
            continue;
        }
        if (loadCategory(entry.path().stem().string(), entry.path().string())) {
            ++count;
        }
    }
    return count;
}

bool Dictionary::hasCategory(const std::string& category) const {
    return words_.find(category) != words_.end();
}

std::vector<std::string> Dictionary::categories() const {
    std::vector<std::string> result;
    for (const auto& pair : words_) {
        result.push_back(pair.first);
    }
    return result;
}

std::size_t Dictionary::wordCount(const std::string& category) const {
    auto it = words_.find(category);
    return it == words_.end() ? 0 : it->second.size();
}

bool Dictionary::empty() const {
    return words_.empty();
}

std::string Dictionary::randomWord(const std::string& category) const {
    auto it = words_.find(category);
    if (it == words_.end() || it->second.empty()) {
        throw std::out_of_range("Category not found or empty: " + category);
    }
    std::uniform_int_distribution<std::size_t> dist(0, it->second.size() - 1);
    return it->second[dist(rng_)];
}

}  // namespace hangman
