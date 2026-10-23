#pragma once

#include <set>
#include <string>

namespace hangman {

enum class GameState {
    InProgress,
    Won,
    Lost
};

enum class GuessResult {
    Hit,            // буква есть в слове
    Miss,           // буквы нет, попытка потрачена
    AlreadyGuessed, // буква уже называлась, попытка не тратится
    InvalidInput    // не буква, попытка не тратится
};

// Один игровой сеанс: загаданное слово, маска, названные буквы, попытки.
class GameSession {
public:
    GameSession(std::string secretWord, std::string category, int maxAttempts = 6);

    // Обрабатывает ввод буквы (регистр не важен).
    GuessResult guess(char letter);

    GameState state() const;
    bool isOver() const;

    // Маска слова для вывода, например "_ a _ g m a _" (раскрытые позиции видны).
    std::string maskForDisplay() const;
    const std::string& mask() const;          // "_a_gma_" без пробелов
    const std::string& secretWord() const;
    const std::string& category() const;
    const std::set<char>& guessedLetters() const;
    std::set<char> wrongLetters() const;

    int maxAttempts() const;
    int wrongCount() const;
    int attemptsLeft() const;

private:
    void revealLetter(char letter);   // открывает все позиции буквы в маске

    std::string secret_;
    std::string category_;
    std::string mask_;
    std::set<char> guessed_;
    int maxAttempts_;
    int wrongCount_;
};

}  // namespace hangman
