#include <SFML/Graphics.hpp>
#include <iostream>

int main() {
    // 构建窗口，大小800*600，名称SFML 3.0......
    sf::RenderWindow window(sf::VideoMode({800, 600}), "SFML 3.0 Game Demo");
    window.setFramerateLimit(60);//最大帧率限制

    sf::CircleShape player(30.f);
    player.setFillColor(sf::Color::Green);
    
    // 设置初始位置，隐式构造vectorf类
    player.setPosition({370.f, 270.f});

    float moveSpeed = 5.f;

    std::cout << "游戏主循环启动！使用 WASD 或方向键控制小球移动。" << std::endl;

    while (window.isOpen()) {
        
        // 💡 pollEvent从窗口的事件队列取出一个事件，返回optional对象，赋值给event
        while (const std::optional event = window.pollEvent()) {
            // 💡 检查事件是否是“关闭窗口”
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            if (event->is<sf::Event::MouseButtonPressed>()) {
                // 获取点击坐标
                sf::Vector2i mousePos = event->getIf<sf::Event::MouseButtonPressed>()->position;
                // 计算圆心位置（左上角 + 半径偏移）
                sf::Vector2f playerCenter = player.getPosition() + sf::Vector2f(30.f, 30.f);
                // 距离平方判断
                float dx = mousePos.x - playerCenter.x;
                float dy = mousePos.y - playerCenter.y;
                if (dx * dx + dy * dy <= 30.f * 30.f) {
                    player.setFillColor(sf::Color(rand() % 256, rand() % 256, rand() % 256));
                }
            }
            if (event->is<sf::Event::KeyPressed>()) {
                if (event->getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::Escape) {
                    window.close();
                }
                if (event->getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::Space) {
                    player.setFillColor(sf::Color(rand() % 256, rand() % 256, rand() % 256));
                }
            }
        }

        // 💡 SFML 3.0 改变：按键枚举从 sf::Keyboard::W 变成了 sf::Keyboard::Key::W
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) {
            player.move({0.f, -moveSpeed}); // 💡 move 也改用大括号传递二维向量
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) {
            player.move({0.f, moveSpeed});
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) {
            player.move({-2 * moveSpeed, 0.f});
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) {
            player.move({2 * moveSpeed, 0.f});
        }

        sf::Vector2f pos = player.getPosition();
        
        float radius = 30.f;
        float winWidth = 800.f;
        float winHeight = 600.f;

        if (pos.x < 0)                    pos.x = 0;
        if (pos.x > winWidth  - radius*2) pos.x = winWidth  - radius*2;
        if (pos.y < 0)                    pos.y = 0;
        if (pos.y > winHeight - radius*2) pos.y = winHeight - radius*2;
        
        player.setPosition(pos);

        window.clear(sf::Color(30, 30, 30));//用指定颜色清空整个窗口的帧缓冲
        window.draw(player);//把 player（绿色圆形）绘制到窗口的帧缓冲中
        window.display();//将后台缓冲区的内容交换到屏幕上（双缓冲交换，swap buffers）
    }

    std::cout << "游戏已安全退出。" << std::endl;
    return 0;
}
