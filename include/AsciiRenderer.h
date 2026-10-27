#pragma once

#include <string>

namespace hangman {

// Возвращает ASCII-рисунок виселицы для текущего числа ошибок.
// Этапы масштабируются под любое maxAttempts: 0 ошибок = пустая виселица,
// maxAttempts ошибок = полностью нарисованный человечек.
std::string renderGallows(int wrongCount, int maxAttempts);

}  // namespace hangman
