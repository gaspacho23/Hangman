// Демонстрация пункта 3: загрузка словарей, случайное слово,
// ввод букв и открытие позиций в маске.
// Полноценный консольный MVP (ASCII-виселица и т.д.) будет в пункте 4.
#include <iostream>
#include <string>

#include "Dictionary.h"
#include "GameSession.h"

using namespace hangman;

int main() {
    Dictionary dictionary;
    if (dictionary.loadFromDirectory("data") == 0) {
        std::cout << "Could not load dictionaries from the 'data' folder.\n"
                     "Run the program from the project root folder.\n";
        return 1;
    }

    std::cout << "Categories:\n";
    for (const auto& name : dictionary.categories()) {
        std::cout << "  " << name << " (" << dictionary.wordCount(name) << " words)\n";
    }

    std::string category;
    while (true) {
        std::cout << "Choose a category: ";
        if (!(std::cin >> category)) {
            return 0;
        }
        if (dictionary.hasCategory(category)) {
            break;
        }
        std::cout << "No such category, try again.\n";
    }

    GameSession session(dictionary.randomWord(category), category);

    while (!session.isOver()) {
        std::cout << "\nWord: " << session.maskForDisplay()
                  << "   Attempts left: " << session.attemptsLeft() << '\n';

        std::string input;
        std::cout << "Enter a letter: ";
        if (!(std::cin >> input)) {
            return 0;
        }

        switch (session.guess(input[0])) {
            case GuessResult::Hit:            std::cout << "Yes!\n"; break;
            case GuessResult::Miss:           std::cout << "No such letter.\n"; break;
            case GuessResult::AlreadyGuessed: std::cout << "You already tried that letter.\n"; break;
            case GuessResult::InvalidInput:   std::cout << "Please enter a letter a-z.\n"; break;
        }
    }

    if (session.state() == GameState::Won) {
        std::cout << "\nYou won! The word was: " << session.secretWord() << '\n';
    } else {
        std::cout << "\nYou lost. The word was: " << session.secretWord() << '\n';
    }
    return 0;
}
