#include "AsciiRenderer.h"

#include <algorithm>

namespace hangman {

namespace {

constexpr int kStageCount = 7;

const char* const kStages[kStageCount] = {
    "  +---+\n"
    "  |   |\n"
    "      |\n"
    "      |\n"
    "      |\n"
    "      |\n"
    "=========\n",

    "  +---+\n"
    "  |   |\n"
    "  O   |\n"
    "      |\n"
    "      |\n"
    "      |\n"
    "=========\n",

    "  +---+\n"
    "  |   |\n"
    "  O   |\n"
    "  |   |\n"
    "      |\n"
    "      |\n"
    "=========\n",

    "  +---+\n"
    "  |   |\n"
    "  O   |\n"
    " /|   |\n"
    "      |\n"
    "      |\n"
    "=========\n",

    "  +---+\n"
    "  |   |\n"
    "  O   |\n"
    " /|\\  |\n"
    "      |\n"
    "      |\n"
    "=========\n",

    "  +---+\n"
    "  |   |\n"
    "  O   |\n"
    " /|\\  |\n"
    " /    |\n"
    "      |\n"
    "=========\n",

    "  +---+\n"
    "  |   |\n"
    "  O   |\n"
    " /|\\  |\n"
    " / \\  |\n"
    "      |\n"
    "=========\n",
};

}  // namespace

std::string renderGallows(int wrongCount, int maxAttempts) {
    if (maxAttempts <= 0) {
        return kStages[0];
    }
    wrongCount = std::max(0, std::min(wrongCount, maxAttempts));
    int stage = wrongCount * (kStageCount - 1) / maxAttempts;
    return kStages[stage];
}

}  // namespace hangman
