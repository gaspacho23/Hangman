#pragma once

#include <cstddef>
#include <map>
#include <random>
#include <string>
#include <vector>

namespace hangman {

// Словарь слов, разбитых по тематическим категориям.
// Одна категория = один текстовый файл (одно слово в строке).
class Dictionary {
public:
    Dictionary();

    // Загружает слова категории из файла. Возвращает false, если файл
    // не открылся или в нём нет ни одного корректного слова.
    bool loadCategory(const std::string& category, const std::string& filePath);

    // Загружает все файлы *.txt из каталога; имя файла = название категории.
    // Возвращает количество успешно загруженных категорий.
    std::size_t loadFromDirectory(const std::string& dirPath);

    bool hasCategory(const std::string& category) const;
    std::vector<std::string> categories() const;
    std::size_t wordCount(const std::string& category) const;
    bool empty() const;

    // Случайное слово из категории. Бросает std::out_of_range,
    // если категории нет или она пуста.
    std::string randomWord(const std::string& category) const;

private:
    std::map<std::string, std::vector<std::string>> words_;
    mutable std::mt19937 rng_;
};

}  // namespace hangman
