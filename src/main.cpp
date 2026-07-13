#include <SFML/Graphics.hpp>
#include <cstring>
#include "DesktopGame.hpp"

int main() {
    const char* title = "尘封的星舰工作站";
    sf::RenderWindow window(
        sf::VideoMode({static_cast<unsigned>(WIN_W), static_cast<unsigned>(WIN_H)}),
        sf::String::fromUtf8(title, title + std::strlen(title)),
        sf::Style::Close | sf::Style::Titlebar);
    window.setFramerateLimit(60);

    Desktop desktop;
    desktop.init(window);
    desktop.run();

    return 0;
}
