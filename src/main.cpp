#include <SFML/Graphics.hpp>
#include "DesktopGame.hpp"

int main() {
    sf::RenderWindow window(
        sf::VideoMode({static_cast<unsigned>(WIN_W), static_cast<unsigned>(WIN_H)}),
        "尘封的星舰工作站",
        sf::Style::Close | sf::Style::Titlebar);
    window.setFramerateLimit(60);

    Desktop desktop;
    desktop.init(window);
    desktop.run();

    return 0;
}
