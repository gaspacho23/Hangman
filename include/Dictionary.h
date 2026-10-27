#pragma once
#include <cstddef>
#include <map>
#include <random>
#include <string>
#include <vector>

namespace hangman {
class Dictionary {
public:
    Dictionary();
    bool loadCategory(const std::string& category, const std::string& filePath);

    // ;загрузка txt из каталога
    std::size_t loadFromDirectory(const std::string& dirPath);

    bool hasCategory(const std::string& category) const;
    std::vector<std::string> categories() const;
    std::size_t wordCount(const std::string& category) const;
    bool empty() const;

    // cлучайное слово из категории
    std::string randomWord(const std::string& category) const;

private:
    std::map<std::string, std::vector<std::string>> words_;
    mutable std::mt19937 rng_;
};
}
