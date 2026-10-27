// Графическая версия игры «Виселица» на SFML 3.
// Логика (словарь, игровой сеанс) берётся из Dictionary и GameSession без изменений.
#include <SFML/Graphics.hpp>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "Dictionary.h"
#include "GameSession.h"

using namespace hangman;

namespace {

constexpr unsigned kWidth = 1000;
constexpr unsigned kHeight = 700;

const sf::Color kBackground(24, 26, 36);
const sf::Color kPanel(48, 53, 74);
const sf::Color kPanelHover(68, 76, 106);
const sf::Color kText(235, 235, 245);
const sf::Color kMuted(140, 145, 165);
const sf::Color kAccent(90, 140, 255);
const sf::Color kHit(60, 170, 100);
const sf::Color kMiss(190, 70, 70);
const sf::Color kGallows(200, 170, 120);
const sf::Color kFigure(235, 235, 245);

struct Button {
    sf::FloatRect rect;
    std::string label;

    bool contains(float x, float y) const { return rect.contains({x, y}); }
};

std::string toUpper(std::string s) {
    for (char& c : s) {
        c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }
    return s;
}

bool loadFont(sf::Font& font) {
    const char* const candidates[] = {
        "C:/Windows/Fonts/arialbd.ttf",
        "C:/Windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/segoeui.ttf",
        "assets/font.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
    };
    for (const char* path : candidates) {
        if (font.openFromFile(path)) {
            return true;
        }
    }
    return false;
}

// Рисует текст. centered = true: pos - центр текста; иначе pos - левый край.
// maxWidth > 0: слишком широкий текст уменьшается, чтобы поместиться.
void drawText(sf::RenderTarget& target, const sf::Font& font, const std::string& str,
              unsigned size, sf::Vector2f pos, const sf::Color& color,
              bool centered = true, float maxWidth = 0.f) {
    sf::Text text(font, str, size);
    text.setFillColor(color);

    const sf::FloatRect bounds = text.getLocalBounds();
    if (centered) {
        text.setOrigin({bounds.position.x + bounds.size.x / 2.f,
                        bounds.position.y + bounds.size.y / 2.f});
    } else {
        text.setOrigin({bounds.position.x, bounds.position.y + bounds.size.y / 2.f});
    }
    if (maxWidth > 0.f && bounds.size.x > maxWidth) {
        const float scale = maxWidth / bounds.size.x;
        text.setScale({scale, scale});
    }
    text.setPosition(pos);
    target.draw(text);
}

void drawButton(sf::RenderTarget& target, const sf::Font& font, const Button& button,
                const sf::Color& fill, unsigned textSize) {
    sf::RectangleShape shape(button.rect.size);
    shape.setPosition(button.rect.position);
    shape.setFillColor(fill);
    target.draw(shape);

    drawText(target, font, button.label, textSize,
             {button.rect.position.x + button.rect.size.x / 2.f,
              button.rect.position.y + button.rect.size.y / 2.f},
             kText);
}

void drawLine(sf::RenderTarget& target, sf::Vector2f a, sf::Vector2f b,
              float thickness, const sf::Color& color) {
    const float dx = b.x - a.x;
    const float dy = b.y - a.y;
    const float length = std::sqrt(dx * dx + dy * dy);

    sf::RectangleShape line({length, thickness});
    line.setOrigin({0.f, thickness / 2.f});
    line.setPosition(a);
    line.setRotation(sf::radians(std::atan2(dy, dx)));
    line.setFillColor(color);
    target.draw(line);
}

// stage: 0..6 - сколько частей человечка уже нарисовано.
void drawGallows(sf::RenderTarget& target, int stage) {
    const float t = 8.f;
    drawLine(target, {70.f, 560.f}, {390.f, 560.f}, t, kGallows);   // основание
    drawLine(target, {160.f, 560.f}, {160.f, 120.f}, t, kGallows);  // столб
    drawLine(target, {156.f, 120.f}, {324.f, 120.f}, t, kGallows);  // перекладина
    drawLine(target, {160.f, 190.f}, {230.f, 120.f}, t, kGallows);  // подпорка
    drawLine(target, {320.f, 120.f}, {320.f, 170.f}, 4.f, kGallows); // верёвка

    const float p = 5.f;
    if (stage >= 1) {  // голова
        sf::CircleShape head(30.f);
        head.setPosition({290.f, 170.f});
        head.setFillColor(sf::Color::Transparent);
        head.setOutlineThickness(p);
        head.setOutlineColor(kFigure);
        target.draw(head);
    }
    if (stage >= 2) drawLine(target, {320.f, 235.f}, {320.f, 380.f}, p, kFigure);  // туловище
    if (stage >= 3) drawLine(target, {320.f, 265.f}, {275.f, 335.f}, p, kFigure);  // левая рука
    if (stage >= 4) drawLine(target, {320.f, 265.f}, {365.f, 335.f}, p, kFigure);  // правая рука
    if (stage >= 5) drawLine(target, {320.f, 380.f}, {282.f, 460.f}, p, kFigure);  // левая нога
    if (stage >= 6) {                                                              // правая нога
        drawLine(target, {320.f, 380.f}, {358.f, 460.f}, p, kFigure);
        // глаза-крестики
        drawLine(target, {306.f, 192.f}, {314.f, 200.f}, 3.f, kMiss);
        drawLine(target, {314.f, 192.f}, {306.f, 200.f}, 3.f, kMiss);
        drawLine(target, {326.f, 192.f}, {334.f, 200.f}, 3.f, kMiss);
        drawLine(target, {334.f, 192.f}, {326.f, 200.f}, 3.f, kMiss);
    }
}

std::vector<Button> makeCategoryButtons(const std::vector<std::string>& categories,
                                        const Dictionary& dictionary) {
    std::vector<Button> buttons;
    const float w = 420.f;
    const float h = 60.f;
    float y = 260.f;
    for (const auto& name : categories) {
        buttons.push_back({sf::FloatRect({(kWidth - w) / 2.f, y}, {w, h}),
                           toUpper(name) + "  (" + std::to_string(dictionary.wordCount(name)) + ")"});
        y += h + 20.f;
    }
    return buttons;
}

std::vector<Button> makeKeyboard() {
    const int rows[3] = {9, 9, 8};
    const float size = 50.f;
    const float gap = 8.f;
    const float left = 445.f;
    const float top = 400.f;

    std::vector<Button> keys;
    int letter = 0;
    for (int row = 0; row < 3; ++row) {
        const float offset = (row == 2) ? (size + gap) / 2.f : 0.f;
        for (int col = 0; col < rows[row]; ++col) {
            keys.push_back({sf::FloatRect({left + offset + col * (size + gap),
                                           top + row * (size + gap)},
                                          {size, size}),
                            std::string(1, static_cast<char>('A' + letter))});
            ++letter;
        }
    }
    return keys;
}

}  // namespace

int main() {
    Dictionary dictionary;
    if (dictionary.loadFromDirectory("data") == 0) {
        std::cerr << "Could not load dictionaries from the 'data' folder.\n"
                     "Run the program from the project root folder.\n";
        return 1;
    }

    sf::Font font;
    if (!loadFont(font)) {
        std::cerr << "Could not load a font. Put any .ttf file at assets/font.ttf.\n";
        return 1;
    }

    sf::RenderWindow window(sf::VideoMode({kWidth, kHeight}), "Hangman",
                            sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(60);

    const std::vector<std::string> categories = dictionary.categories();
    const std::vector<Button> categoryButtons = makeCategoryButtons(categories, dictionary);
    const std::vector<Button> keys = makeKeyboard();
    const Button playAgainButton{sf::FloatRect({280.f, 420.f}, {200.f, 55.f}), "Play again"};
    const Button menuButton{sf::FloatRect({520.f, 420.f}, {200.f, 55.f}), "Menu"};

    std::unique_ptr<GameSession> session;
    std::string currentCategory;

    auto startGame = [&](const std::string& category) {
        currentCategory = category;
        session = std::make_unique<GameSession>(dictionary.randomWord(category), category);
    };

    while (window.isOpen()) {
        const sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        const float mx = static_cast<float>(mousePos.x);
        const float my = static_cast<float>(mousePos.y);

        // ---------- события ----------
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            } else if (const auto* click = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (click->button != sf::Mouse::Button::Left) {
                    continue;
                }
                const float x = static_cast<float>(click->position.x);
                const float y = static_cast<float>(click->position.y);

                if (!session) {
                    for (std::size_t i = 0; i < categoryButtons.size(); ++i) {
                        if (categoryButtons[i].contains(x, y)) {
                            startGame(categories[i]);
                            break;
                        }
                    }
                } else if (!session->isOver()) {
                    for (std::size_t i = 0; i < keys.size(); ++i) {
                        if (keys[i].contains(x, y)) {
                            session->guess(static_cast<char>('a' + i));
                            break;
                        }
                    }
                } else if (playAgainButton.contains(x, y)) {
                    startGame(currentCategory);
                } else if (menuButton.contains(x, y)) {
                    session.reset();
                }
            } else if (const auto* text = event->getIf<sf::Event::TextEntered>()) {
                if (!session) {
                    continue;
                }
                const char32_t ch = text->unicode;
                if (!session->isOver()) {
                    if (ch < 128 && std::isalpha(static_cast<int>(ch))) {
                        session->guess(static_cast<char>(ch));
                    }
                } else if (ch == '\r' || ch == ' ') {
                    startGame(currentCategory);
                }
            } else if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
                if (key->code == sf::Keyboard::Key::Escape) {
                    if (session) {
                        session.reset();
                    } else {
                        window.close();
                    }
                }
            }
        }

        // ---------- отрисовка ----------
        window.clear(kBackground);

        if (!session) {
            drawText(window, font, "HANGMAN", 80, {kWidth / 2.f, 110.f}, kText);
            drawText(window, font, "Choose a category", 26, {kWidth / 2.f, 195.f}, kMuted);
            for (const auto& button : categoryButtons) {
                const bool hover = button.contains(mx, my);
                drawButton(window, font, button, hover ? kAccent : kPanel, 28);
            }
            drawText(window, font, "Esc - quit", 18, {kWidth / 2.f, 660.f}, kMuted);
        } else {
            const int stage = session->wrongCount() * 6 / session->maxAttempts();
            drawGallows(window, stage);

            drawText(window, font, "CATEGORY: " + toUpper(session->category()), 24,
                     {700.f, 60.f}, kMuted);

            std::string mask = toUpper(session->maskForDisplay());
            drawText(window, font, mask, 48, {700.f, 150.f}, kText, true, 520.f);

            std::string wrong = "Wrong: ";
            for (char c : session->wrongLetters()) {
                wrong += static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
                wrong += ' ';
            }
            drawText(window, font, wrong, 26, {700.f, 250.f}, kMiss, true, 520.f);
            drawText(window, font, "Attempts left: " + std::to_string(session->attemptsLeft()),
                     26, {700.f, 300.f}, kText);

            for (std::size_t i = 0; i < keys.size(); ++i) {
                const char letter = static_cast<char>('a' + i);
                const bool guessed = session->guessedLetters().count(letter) > 0;
                const bool hit = guessed && session->secretWord().find(letter) != std::string::npos;

                sf::Color fill = kPanel;
                if (hit) {
                    fill = kHit;
                } else if (guessed) {
                    fill = kMiss;
                } else if (!session->isOver() && keys[i].contains(mx, my)) {
                    fill = kPanelHover;
                }
                drawButton(window, font, keys[i], fill, 26);
            }
            drawText(window, font, "Type a letter or click it  |  Esc - menu", 18,
                     {700.f, 650.f}, kMuted);

            if (session->isOver()) {
                sf::RectangleShape dim({static_cast<float>(kWidth), static_cast<float>(kHeight)});
                dim.setFillColor(sf::Color(0, 0, 0, 190));
                window.draw(dim);

                sf::RectangleShape panel({500.f, 330.f});
                panel.setPosition({250.f, 190.f});
                panel.setFillColor(kBackground);
                panel.setOutlineThickness(3.f);
                panel.setOutlineColor(kAccent);
                window.draw(panel);

                const bool won = session->state() == GameState::Won;
                drawText(window, font, won ? "YOU WON!" : "YOU LOST", 60,
                         {kWidth / 2.f, 260.f}, won ? kHit : kMiss);
                drawText(window, font, "The word was: " + toUpper(session->secretWord()), 28,
                         {kWidth / 2.f, 345.f}, kText, true, 440.f);

                drawButton(window, font, playAgainButton,
                           playAgainButton.contains(mx, my) ? kAccent : kPanel, 24);
                drawButton(window, font, menuButton,
                           menuButton.contains(mx, my) ? kAccent : kPanel, 24);
                drawText(window, font, "Enter - play again  |  Esc - menu", 18,
                         {kWidth / 2.f, 497.f}, kMuted);
            }
        }

        window.display();
    }
    return 0;
}
