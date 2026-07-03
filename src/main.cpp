#include <SFML/Graphics.hpp>
#include "DesktopGame.hpp"
#include <algorithm>
#include <iostream>
#include <optional>

int main() {
    // ── Window ─────────────────────────────────────────────
    sf::RenderWindow window(sf::VideoMode({static_cast<unsigned>(WIN_W), static_cast<unsigned>(WIN_H)}),
                            "Desktop Puzzle Game", sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(60);
    window.setMouseCursorVisible(false); // Hide the native OS cursor for retro style

    // ── Font ───────────────────────────────────────────────
    sf::Font font;
    bool hasFont = font.openFromFile("C:/Windows/Fonts/consola.ttf");
    if (!hasFont) hasFont = font.openFromFile("C:/Windows/Fonts/arial.ttf");
    if (!hasFont) hasFont = font.openFromFile("C:/Windows/Fonts/segoeui.ttf");
    if (!hasFont) {
        std::cerr << "Warning: could not load any font. Text will not render.\n";
    }

    // ── Background Wallpaper ───────────────────────────────
    sf::Texture bgTexture;
    std::optional<sf::Sprite> bgSprite;
    if (bgTexture.loadFromFile("assets/wallpaper.jpg")) {
        bgSprite.emplace(bgTexture);
        sf::Vector2u texSize = bgTexture.getSize();
        bgSprite->setScale({WIN_W / texSize.x, WIN_H / texSize.y});
    }

    // ── Windows XP Luna Textures ───────────────────────────
    sf::Texture startTex;
    sf::Texture notepadTex;
    sf::Texture calcTex;
    sf::Texture folderTex;
    sf::Texture recycleTex;

    auto loadXpTexture = [](sf::Texture& tex, const std::string& path) -> bool {
        sf::Image img;
        if (img.loadFromFile(path)) {
            // BFS Flood Fill to mask out outer blackish background
            sf::Vector2u size = img.getSize();
            std::vector<bool> visited(size.x * size.y, false);
            std::vector<sf::Vector2i> q;
            q.reserve(size.x * size.y);

            auto isBlackish = [](sf::Color c) {
                // Threshold of 55: anything darker than RGB(55,55,55) is blackish background
                return (c.r < 55 && c.g < 55 && c.b < 55);
            };

            // 1. Add all border pixels to queue
            for (unsigned int x = 0; x < size.x; ++x) {
                q.push_back({static_cast<int>(x), 0});
                q.push_back({static_cast<int>(x), static_cast<int>(size.y - 1)});
                visited[0 * size.x + x] = true;
                visited[(size.y - 1) * size.x + x] = true;
            }
            for (unsigned int y = 0; y < size.y; ++y) {
                q.push_back({0, static_cast<int>(y)});
                q.push_back({static_cast<int>(size.x - 1), static_cast<int>(y)});
                visited[y * size.x + 0] = true;
                visited[y * size.x + (size.x - 1)] = true;
            }

            // 2. BFS Flood Fill
            size_t head = 0;
            while (head < q.size()) {
                sf::Vector2i curr = q[head++];
                sf::Color c = img.getPixel(sf::Vector2u(curr.x, curr.y));

                if (isBlackish(c)) {
                    // Set transparent
                    img.setPixel(sf::Vector2u(curr.x, curr.y), sf::Color(c.r, c.g, c.b, 0));

                    const int dx[] = {0, 0, 1, -1};
                    const int dy[] = {1, -1, 0, 0};
                    for (int i = 0; i < 4; ++i) {
                        int nx = curr.x + dx[i];
                        int ny = curr.y + dy[i];
                        if (nx >= 0 && nx < static_cast<int>(size.x) &&
                            ny >= 0 && ny < static_cast<int>(size.y)) {
                            size_t idx = static_cast<size_t>(ny) * size.x + static_cast<size_t>(nx);
                            if (!visited[idx]) {
                                visited[idx] = true;
                                q.push_back({nx, ny});
                            }
                        }
                    }
                }
            }

            return tex.loadFromImage(img);
        }
        return false;
    };

    bool startLoaded = loadXpTexture(startTex, "assets/start.jpg");
    bool notepadLoaded = loadXpTexture(notepadTex, "assets/notepad.jpg");
    bool calcLoaded = loadXpTexture(calcTex, "assets/calculator.jpg");
    bool folderLoaded = loadXpTexture(folderTex, "assets/folder.jpg");
    bool recycleLoaded = loadXpTexture(recycleTex, "assets/recycle.jpg");

    // ── Desktop icons ──────────────────────────────────────
    std::vector<DesktopIcon> icons;
    icons.emplace_back(sf::Vector2f{30.f, 30.f},  sf::Vector2f{ICON_W, ICON_H},
                       "Notepad",    "notepad",    font, notepadLoaded ? &notepadTex : nullptr);
    icons.emplace_back(sf::Vector2f{30.f, 130.f}, sf::Vector2f{ICON_W, ICON_H},
                       "Calculator", "calculator", font, calcLoaded ? &calcTex : nullptr);
    icons.emplace_back(sf::Vector2f{30.f, 230.f}, sf::Vector2f{ICON_W, ICON_H},
                       "Folder",     "folder",     font, folderLoaded ? &folderTex : nullptr);
    icons.emplace_back(sf::Vector2f{30.f, 330.f}, sf::Vector2f{ICON_W, ICON_H},
                       "Recycle",    "recycle",    font, recycleLoaded ? &recycleTex : nullptr);

    // ── Taskbar ───────────────────────────────────────────
    Taskbar taskbar(font, startLoaded ? &startTex : nullptr);

    // ── Windows container ──────────────────────────────────
    std::vector<GameWindow> windows;
    GameWindow* focusedWindow = nullptr;
    int zOrderCounter = 0;

    // ── Context menu ──────────────────────────────────────
    ContextMenu ctxMenu(font);

    // ── Double-click state ────────────────────────────────
    sf::Clock dblClock;
    int lastClickedIconIdx = -1;

    // ── Refresh animation state ───────────────────────────
    float refreshTimer = 0.f;
    sf::Clock refreshClock;

    // ── Window spawn offset ───────────────────────────────
    auto openWindow = [&](const std::string& appType) {
        static int stagger = 0;
        GameWindow gw(font, appType);
        sf::Vector2f base = {100.f + stagger * 30.f, 40.f + stagger * 30.f};
        if (base.x > 450.f || base.y > 300.f) { stagger = 0; base = {100.f, 40.f}; }
        gw.setPos(base);
        gw.zOrder = ++zOrderCounter;
        windows.push_back(std::move(gw));
        focusedWindow = &windows.back();
        ++stagger;
    };

    // ── Helper: bring window to front ─────────────────────
    auto bringToFront = [&](GameWindow* gw) {
        if (!gw) return;
        gw->zOrder = ++zOrderCounter;
        focusedWindow = gw;
    };

    // ── Helper: hit-test windows from top z-order ─────────
    auto windowAt = [&](sf::Vector2i mp) -> GameWindow* {
        // iterate a sorted copy of indices by zOrder desc
        std::vector<size_t> idxs;
        for (size_t i = 0; i < windows.size(); ++i) idxs.push_back(i);
        std::sort(idxs.begin(), idxs.end(), [&](size_t a, size_t b) {
            return windows[a].zOrder > windows[b].zOrder;
        });
        for (size_t i : idxs) {
            if (!windows[i].isOpen || windows[i].isMinimized) continue;
            sf::FloatRect r = windows[i].bounds();
            if (r.contains({static_cast<float>(mp.x), static_cast<float>(mp.y)}))
                return &windows[i];
        }
        return nullptr;
    };

    // ── Helper: icon at position ──────────────────────────
    auto iconAt = [&](sf::Vector2i mp) -> int {
        for (int i = 0; i < static_cast<int>(icons.size()); ++i) {
            if (icons[i].contains(mp)) return i;
        }
        return -1;
    };

    // ── Main loop ─────────────────────────────────────────
    std::cout << "Desktop Puzzle Game started!\n";
    std::cout << "  - Double-click icons to open apps\n";
    std::cout << "  - Right-click for context menu\n";
    std::cout << "  - Drag windows by title bar\n";
    std::cout << "  - Solve the Notepad, Calculator, and Folder puzzles\n";

    while (window.isOpen()) {
        // ── Event handling ────────────────────────────────
        while (const std::optional event = window.pollEvent()) {
            // 1. Close
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            // 2. Text input → focused window
            if (event->is<sf::Event::TextEntered>() && focusedWindow) {
                uint32_t uc = event->getIf<sf::Event::TextEntered>()->unicode;
                focusedWindow->handleTextInput(uc);
            }

            // 3. Mouse button pressed
            if (event->is<sf::Event::MouseButtonPressed>()) {
                auto* mb = event->getIf<sf::Event::MouseButtonPressed>();
                sf::Vector2i mp = mb->position;
                auto btn = mb->button;

                // ── LEFT CLICK ────────────────────────────
                if (btn == sf::Mouse::Button::Left) {
                    // a) Context menu item click?
                    if (ctxMenu.visible) {
                        int idx = ctxMenu.itemAt(mp);
                        if (idx >= 0 && idx < static_cast<int>(ctxMenu.items.size())) {
                            ctxMenu.items[idx].action();
                            ctxMenu.hide();
                        } else {
                            ctxMenu.hide();
                        }
                        continue;
                    }

                    // b) Window buttons (close / max / min)?
                    GameWindow* gw = windowAt(mp);
                    if (gw && gw->containsCloseBtn(mp)) {
                        gw->isOpen = false;
                        if (focusedWindow == gw) focusedWindow = nullptr;
                        continue;
                    }
                    if (gw && gw->containsMaxBtn(mp)) {
                        if (!gw->isMaximized) {
                            // Save current state then maximize
                            gw->preMaxPos  = gw->titleBar.getPosition();
                            gw->preMaxSize = gw->bounds().size;
                            gw->isMaximized = true;
                            gw->setPos({0.f, 0.f});
                            gw->setSize({WIN_W, DESKTOP_H});
                        } else {
                            // Restore
                            gw->isMaximized = false;
                            gw->setPos(gw->preMaxPos);
                            gw->setSize(gw->preMaxSize);
                        }
                        bringToFront(gw);
                        continue;
                    }
                    if (gw && gw->containsMinBtn(mp)) {
                        gw->isMinimized = true;
                        if (focusedWindow == gw) focusedWindow = nullptr;
                        continue;
                    }

                    // c) Window resize border?
                    // Check all windows for resize border hit (highest z first)
                    bool startedResize = false;
                    {
                        std::vector<size_t> idxs;
                        for (size_t i = 0; i < windows.size(); ++i) idxs.push_back(i);
                        std::sort(idxs.begin(), idxs.end(), [&](size_t a, size_t b){
                            return windows[a].zOrder > windows[b].zOrder;
                        });
                        for (size_t i : idxs) {
                            if (!windows[i].isOpen || windows[i].isMinimized) continue;
                            ResizeDir rd = windows[i].hitResizeBorder(mp);
                            if (rd != ResizeDir::None) {
                                bringToFront(&windows[i]);
                                windows[i].isResizing = true;
                                windows[i].resizeDir = rd;
                                windows[i].resizeStartMouse = mp;
                                windows[i].resizeStartSize = windows[i].bounds().size;
                                startedResize = true;
                                break;
                            }
                        }
                    }
                    if (startedResize) continue;


                    // d) Window title bar? (start drag + focus)
                    gw = windowAt(mp);
                    if (gw && gw->containsTitleBar(mp)) {
                        bringToFront(gw);
                        if (!gw->isMaximized) {
                            gw->isDragging = true;
                            sf::Vector2f tp = gw->titleBar.getPosition();
                            gw->dragOffset = {static_cast<float>(mp.x) - tp.x,
                                              static_cast<float>(mp.y) - tp.y};
                        }
                        continue;
                    }

                    // e) Window body click? (focus only)
                    if (gw) {
                        bringToFront(gw);
                        continue;
                    }

                    // f) Taskbar button click? (minimize/restore)
                    int tbi = taskbar.windowBtnAt(mp);
                    if (tbi >= 0) {
                        int wi = 0;
                        for (auto& w : windows) {
                            if (!w.isOpen) continue;
                            if (wi == tbi) {
                                w.isMinimized = !w.isMinimized;
                                if (w.isMinimized && focusedWindow == &w)
                                    focusedWindow = nullptr;
                                else if (!w.isMinimized)
                                    bringToFront(&w);
                                break;
                            }
                            ++wi;
                        }
                        continue;
                    }

                    // g) Desktop icon click?
                    int ii = iconAt(mp);
                    if (ii >= 0) {
                        int now = dblClock.getElapsedTime().asMilliseconds();
                        if (lastClickedIconIdx == ii && now < DOUBLE_CLICK_MS) {
                            openWindow(icons[ii].appType);
                            for (auto& ic : icons) ic.selected = false;
                            lastClickedIconIdx = -1;
                        } else {
                            for (auto& ic : icons) ic.selected = false;
                            icons[ii].selected = true;
                            lastClickedIconIdx = ii;
                        }
                        dblClock.restart();
                        continue;
                    }

                    // h) Empty desktop click → deselect
                    for (auto& ic : icons) ic.selected = false;
                    lastClickedIconIdx = -1;
                }

                // ── RIGHT CLICK ───────────────────────────
                if (btn == sf::Mouse::Button::Right) {
                    ctxMenu.hide();

                    // a) Taskbar button right-click → window control menu
                    int tbi = taskbar.windowBtnAt(mp);
                    if (tbi >= 0) {
                        int wi = 0;
                        GameWindow* captured = nullptr;
                        for (auto& w : windows) {
                            if (!w.isOpen) continue;
                            if (wi == tbi) { captured = &w; break; }
                            ++wi;
                        }
                        if (captured) {
                            ctxMenu.show({static_cast<float>(mp.x), static_cast<float>(mp.y)},
                            {
                                {"Restore", [captured, &bringToFront]() {
                                    captured->isMinimized = false;
                                    bringToFront(captured);
                                }},
                                {"Minimize", [captured, &focusedWindow]() {
                                    captured->isMinimized = true;
                                    if (focusedWindow == captured) focusedWindow = nullptr;
                                }},
                                {"Close Window", [captured, &focusedWindow]() {
                                    captured->isOpen = false;
                                    if (focusedWindow == captured) focusedWindow = nullptr;
                                }},
                            }, font);
                        }
                        continue;
                    }

                    // b) Window title bar right-click?
                    GameWindow* gw = windowAt(mp);
                    if (gw && gw->containsTitleBar(mp)) {
                        GameWindow* captured = gw;
                        ctxMenu.show({static_cast<float>(mp.x), static_cast<float>(mp.y)},
                        {
                            {"Close", [captured, &focusedWindow]() {
                                captured->isOpen = false;
                                if (focusedWindow == captured) focusedWindow = nullptr;
                            }},
                            {"Minimize", [captured, &focusedWindow]() {
                                captured->isMinimized = true;
                                if (focusedWindow == captured) focusedWindow = nullptr;
                            }},
                            {"Bring to Front", [captured, &zOrderCounter]() {
                                captured->zOrder = ++zOrderCounter;
                            }},
                        }, font);
                        continue;
                    }

                    // c) Icon right-click?
                    int ii = iconAt(mp);
                    if (ii >= 0) {
                        int captured = ii;
                        ctxMenu.show({static_cast<float>(mp.x), static_cast<float>(mp.y)},
                        {
                            {"Properties", [&openWindow]() {
                                openWindow("notepad");
                            }},
                            {"Delete", [&icons, captured, &lastClickedIconIdx]() {
                                icons.erase(icons.begin() + captured);
                                lastClickedIconIdx = -1;
                            }},
                        }, font);
                        continue;
                    }

                    // d) Empty desktop right-click
                    ctxMenu.show({static_cast<float>(mp.x), static_cast<float>(mp.y)},
                    {
                        {"New Folder", []() { /* no-op */ }},
                        {"Refresh", [&refreshTimer, &refreshClock]() {
                            refreshTimer = 0.2f;
                            refreshClock.restart();
                        }},
                    }, font);
                    continue;
                }
            }

            // 4. Mouse released → stop dragging + resizing
            if (event->is<sf::Event::MouseButtonReleased>()) {
                for (auto& w : windows) {
                    w.isDragging = false;
                    w.isResizing = false;
                    w.resizeDir = ResizeDir::None;
                }
            }

            // 5. Mouse moved → drag + resize + context menu hover
            if (event->is<sf::Event::MouseMoved>()) {
                sf::Vector2i mp = event->getIf<sf::Event::MouseMoved>()->position;

                for (auto& w : windows) {
                    if (w.isDragging) {
                        sf::Vector2f newPos = {
                            static_cast<float>(mp.x) - w.dragOffset.x,
                            static_cast<float>(mp.y) - w.dragOffset.y
                        };
                        newPos = clampWindowPos(w, newPos);
                        w.setPos(newPos);
                    }
                    if (w.isResizing) {
                        float dx = static_cast<float>(mp.x - w.resizeStartMouse.x);
                        float dy = static_cast<float>(mp.y - w.resizeStartMouse.y);
                        float newW = w.resizeStartSize.x;
                        float newH = w.resizeStartSize.y;
                        if (w.resizeDir == ResizeDir::Right || w.resizeDir == ResizeDir::BottomRight)
                            newW = std::max(200.f, w.resizeStartSize.x + dx);
                        if (w.resizeDir == ResizeDir::Bottom || w.resizeDir == ResizeDir::BottomRight)
                            newH = std::max(120.f, w.resizeStartSize.y + dy);
                        w.setSize({newW, newH});
                    }
                }

                if (ctxMenu.visible) {
                    ctxMenu.updateHover(mp);
                }
            }

        }

        // ── Clean up closed windows ───────────────────────
        windows.erase(std::remove_if(windows.begin(), windows.end(),
            [](const GameWindow& w) { return !w.isOpen; }), windows.end());

        // ── Update taskbar ────────────────────────────────
        taskbar.update(windows, font);

        // ── Render ────────────────────────────────────────
        window.clear(C::DesktopBg);
        if (bgSprite) {
            window.draw(*bgSprite);
        }

        // ── Compute refresh animation ─────────────────────
        if (refreshTimer > 0.f) {
            refreshTimer -= 1.f / 60.f;
        }

        // 1. Icons (with momentary Windows XP refresh blink - hide during first 0.1s of refresh)
        bool hideIcons = (refreshTimer > 0.1f);
        if (!hideIcons) {
            for (const auto& ic : icons) ic.draw(window);
        }

        // 2. Windows (sorted by zOrder)
        std::vector<GameWindow*> sortedWin;
        for (auto& w : windows) sortedWin.push_back(&w);
        std::sort(sortedWin.begin(), sortedWin.end(),
            [](const GameWindow* a, const GameWindow* b) {
                return a->zOrder < b->zOrder;
            });
        for (const auto* w : sortedWin) w->draw(window);

        // 3. Taskbar
        taskbar.draw(window);

        // 4. Context menu (topmost)
        ctxMenu.draw(window);

        // 5. Dynamic cursor selection based on resize hover/dragging
        const char* const* activeCursor = C::CursorPattern;
        ResizeDir hoverDir = ResizeDir::None;
        
        // Check if any window is actively resizing
        for (const auto& w : windows) {
            if (w.isResizing) {
                hoverDir = w.resizeDir;
                break;
            }
        }
        
        // If not resizing, check if hover is over a resize border of the topmost window at mouse position
        if (hoverDir == ResizeDir::None) {
            sf::Vector2i mPos = sf::Mouse::getPosition(window);
            std::vector<size_t> idxs;
            for (size_t i = 0; i < windows.size(); ++i) idxs.push_back(i);
            std::sort(idxs.begin(), idxs.end(), [&](size_t a, size_t b) {
                return windows[a].zOrder > windows[b].zOrder;
            });
            for (size_t i : idxs) {
                if (!windows[i].isOpen || windows[i].isMinimized) continue;
                ResizeDir rd = windows[i].hitResizeBorder(mPos);
                if (rd != ResizeDir::None) {
                    hoverDir = rd;
                    break;
                }
            }
        }
        
        // Select cursor pattern
        if      (hoverDir == ResizeDir::Right)       activeCursor = C::CursorResizeH;
        else if (hoverDir == ResizeDir::Bottom)      activeCursor = C::CursorResizeV;
        else if (hoverDir == ResizeDir::BottomRight) activeCursor = C::CursorResizeDiag;

        // 6. Custom cursor drawing (above everything else)
        sf::Vector2i mPos = sf::Mouse::getPosition(window);
        if (mPos.x >= 0 && mPos.x < WIN_W && mPos.y >= 0 && mPos.y < WIN_H) {
            drawPixelPattern(window, sf::Vector2f(static_cast<float>(mPos.x), static_cast<float>(mPos.y)), activeCursor, 1.5f);
        }

        window.display();
    }

    std::cout << "Game exited.\n";
    return 0;
}
