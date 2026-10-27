#include "GameSession.h"

#include <stdexcept>

namespace hangman {

namespace {

bool isLatinLetter(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

char toLower(char c) {
    return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c;
}

}  // namespace

GameSession::GameSession(std::string secretWord, std::string category, int maxAttempts)
    : secret_(std::move(secretWord)),
      category_(std::move(category)),
      maxAttempts_(maxAttempts),
      wrongCount_(0) {
    if (secret_.empty()) {
        throw std::invalid_argument("Secret word must not be empty");
    }
    if (maxAttempts_ <= 0) {
        throw std::invalid_argument("maxAttempts must be positive");
    }
    for (char& c : secret_) {
        if (!isLatinLetter(c)) {
            throw std::invalid_argument("Secret word must contain only Latin letters");
        }
        c = toLower(c);
    }
    mask_.assign(secret_.size(), '_');
}

GuessResult GameSession::guess(char letter) {
    if (isOver() || !isLatinLetter(letter)) {
        return GuessResult::InvalidInput;
    }
    letter = toLower(letter);

    if (guessed_.count(letter) > 0) {
        return GuessResult::AlreadyGuessed;
    }
    guessed_.insert(letter);

    if (secret_.find(letter) != std::string::npos) {
        revealLetter(letter);
        return GuessResult::Hit;
    }

    ++wrongCount_;
    return GuessResult::Miss;
}

void GameSession::revealLetter(char letter) {
    for (std::size_t i = 0; i < secret_.size(); ++i) {
        if (secret_[i] == letter) {
            mask_[i] = letter;
        }
    }
}

GameState GameSession::state() const {
    if (mask_ == secret_) {
        return GameState::Won;
    }
    if (wrongCount_ >= maxAttempts_) {
        return GameState::Lost;
    }
    return GameState::InProgress;
}

bool GameSession::isOver() const {
    return state() != GameState::InProgress;
}

std::string GameSession::maskForDisplay() const {
    std::string result;
    for (std::size_t i = 0; i < mask_.size(); ++i) {
        if (i > 0) {
            result += ' ';
        }
        result += mask_[i];
    }
    return result;
}

const std::string& GameSession::mask() const { return mask_; }
const std::string& GameSession::secretWord() const { return secret_; }
const std::string& GameSession::category() const { return category_; }
const std::set<char>& GameSession::guessedLetters() const { return guessed_; }

std::set<char> GameSession::wrongLetters() const {
    std::set<char> wrong;
    for (char c : guessed_) {
        if (secret_.find(c) == std::string::npos) {
            wrong.insert(c);
        }
    }
    return wrong;
}

int GameSession::maxAttempts() const { return maxAttempts_; }
int GameSession::wrongCount() const { return wrongCount_; }
int GameSession::attemptsLeft() const { return maxAttempts_ - wrongCount_; }

}  // namespace hangman
