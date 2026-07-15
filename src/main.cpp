#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <SFML/Graphics.hpp>
#include <array>
#include <cstring>
#include <filesystem>
#include "DesktopGame.hpp"

int main() {
    // 使用 Windows 宽字符 API 获取程序目录。argv[0] 是窄字符串，在包含
    // 中文的解压路径下会产生编码错误并导致程序启动阶段异常退出。
    std::array<wchar_t, 32768> exePathBuffer{};
    const DWORD pathLength = GetModuleFileNameW(
        nullptr, exePathBuffer.data(), static_cast<DWORD>(exePathBuffer.size()));
    if (pathLength > 0 && pathLength < exePathBuffer.size()) {
        std::error_code ec;
        const std::filesystem::path exePath(
            std::wstring(exePathBuffer.data(), static_cast<std::size_t>(pathLength)));
        if (exePath.has_parent_path())
            std::filesystem::current_path(exePath.parent_path(), ec);
    }

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
