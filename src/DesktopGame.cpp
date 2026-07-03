#include "DesktopGame.hpp"
#include <cmath>
#include <cstdint>

// ═══════════════════════════════════════════════════════════════
//  Helpers & Renderers
// ═══════════════════════════════════════════════════════════════
void drawPixelPattern(sf::RenderWindow& win, sf::Vector2f pos, const char* const pattern[16], float pixelSize) {
    sf::VertexArray va(sf::PrimitiveType::Triangles);
    for (int y = 0; y < 16; ++y) {
        for (int x = 0; x < 16; ++x) {
            char c = pattern[y][x];
            if (c == '.') continue;
            sf::Color color = C::charToColor(c);
            
            float px = pos.x + x * pixelSize;
            float py = pos.y + y * pixelSize;
            
            va.append(sf::Vertex{ {px, py}, color });
            va.append(sf::Vertex{ {px + pixelSize, py}, color });
            va.append(sf::Vertex{ {px + pixelSize, py + pixelSize}, color });
            
            va.append(sf::Vertex{ {px, py}, color });
            va.append(sf::Vertex{ {px + pixelSize, py + pixelSize}, color });
            va.append(sf::Vertex{ {px, py + pixelSize}, color });
        }
    }
    win.draw(va);
}

void drawDoubleBevel(sf::RenderWindow& win, sf::FloatRect rect, bool raised) {
    float t = 2.f; // Thickness of each bevel line
    
    // Colors
    sf::Color outerTL = raised ? sf::Color::White : C::BevelDark;
    sf::Color innerTL = raised ? C::BevelLight : C::BevelMedium;
    sf::Color innerBR = raised ? C::BevelMedium : C::BevelLight;
    sf::Color outerBR = raised ? C::BevelDark : sf::Color::White;

    // 1. Outer bevel
    // Top
    sf::RectangleShape oTop({rect.size.x, t});
    oTop.setPosition(rect.position);
    oTop.setFillColor(outerTL);
    win.draw(oTop);
    
    // Left
    sf::RectangleShape oLeft({t, rect.size.y});
    oLeft.setPosition(rect.position);
    oLeft.setFillColor(outerTL);
    win.draw(oLeft);
    
    // Bottom
    sf::RectangleShape oBottom({rect.size.x, t});
    oBottom.setPosition({rect.position.x, rect.position.y + rect.size.y - t});
    oBottom.setFillColor(outerBR);
    win.draw(oBottom);
    
    // Right
    sf::RectangleShape oRight({t, rect.size.y});
    oRight.setPosition({rect.position.x + rect.size.x - t, rect.position.y});
    oRight.setFillColor(outerBR);
    win.draw(oRight);

    // 2. Inner bevel (offset by t)
    sf::Vector2f innerPos = rect.position + sf::Vector2f(t, t);
    sf::Vector2f innerSize = rect.size - sf::Vector2f(2.f * t, 2.f * t);
    if (innerSize.x <= 0 || innerSize.y <= 0) return;

    // Top
    sf::RectangleShape iTop({innerSize.x, t});
    iTop.setPosition(innerPos);
    iTop.setFillColor(innerTL);
    win.draw(iTop);
    
    // Left
    sf::RectangleShape iLeft({t, innerSize.y});
    iLeft.setPosition(innerPos);
    iLeft.setFillColor(innerTL);
    win.draw(iLeft);
    
    // Bottom
    sf::RectangleShape iBottom({innerSize.x, t});
    iBottom.setPosition({innerPos.x, innerPos.y + innerSize.y - t});
    iBottom.setFillColor(innerBR);
    win.draw(iBottom);
    
    // Right
    sf::RectangleShape iRight({t, innerSize.y});
    iRight.setPosition({innerPos.x + innerSize.x - t, innerPos.y});
    iRight.setFillColor(innerBR);
    win.draw(iRight);
}

// ═══════════════════════════════════════════════════════════════
//  DesktopIcon
// ═══════════════════════════════════════════════════════════════
DesktopIcon::DesktopIcon(sf::Vector2f p, sf::Vector2f sz, const std::string& lbl,
                         const std::string& app, const sf::Font& font, const sf::Texture* tex)
    : pos(p), size(sz), label(lbl), appType(app),
      labelText(font, sf::String::fromUtf8(lbl.begin(), lbl.end()), 13),
      texturePtr(tex)
{
    shape.setSize(size);
    shape.setPosition(pos);
    shape.setFillColor(sf::Color::Transparent);

    labelText.setFillColor(sf::Color::White); // Classic Windows desktop text color is white
    // center text under icon
    sf::FloatRect tb = labelText.getLocalBounds();
    labelText.setOrigin({tb.position.x + tb.size.x / 2.f, tb.position.y});
    labelText.setPosition({pos.x + size.x / 2.f, pos.y + size.y + 8.f});
}

bool DesktopIcon::contains(sf::Vector2i mp) const {
    float fx = static_cast<float>(mp.x);
    float fy = static_cast<float>(mp.y);
    return fx >= pos.x && fx <= pos.x + size.x &&
           fy >= pos.y && fy <= pos.y + size.y + ICON_TEXT_H;
}

void DesktopIcon::draw(sf::RenderWindow& win, sf::Vector2f offset) const {
    sf::Vector2f drawPos = pos + offset;
    
    if (texturePtr) {
        sf::Sprite sp(*texturePtr);
        sp.setPosition(drawPos);
        sf::Vector2u texSize = texturePtr->getSize();
        sp.setScale({size.x / texSize.x, size.y / texSize.y});
        win.draw(sp);
    } else {
        // Fallback to pixel art if no texture loaded
        const char* const* pattern = nullptr;
        if (appType == "notepad")         pattern = C::NotepadIconPattern;
        else if (appType == "calculator") pattern = C::CalculatorIconPattern;
        else if (appType == "folder")     pattern = C::FolderIconPattern;
        else if (appType == "recycle")    pattern = C::RecycleIconPattern;
        
        if (pattern) {
            drawPixelPattern(win, drawPos, pattern, 4.f);
        }
    }

    sf::Text shiftedLabel = labelText;
    shiftedLabel.setPosition({drawPos.x + size.x / 2.f, drawPos.y + size.y + 8.f});

    if (selected) {
        // Semi-transparent blue selection box overlay on the icon itself (like standard OS selection)
        sf::RectangleShape selRect({size.x + 8.f, size.y + 8.f});
        selRect.setPosition({drawPos.x - 4.f, drawPos.y - 4.f});
        selRect.setFillColor(sf::Color(49, 106, 197, 60)); // Transparent blue
        selRect.setOutlineColor(C::XpBlueLight);
        selRect.setOutlineThickness(1.f);
        win.draw(selRect);

        // Highlight box for the text label
        sf::FloatRect tb = shiftedLabel.getGlobalBounds();
        sf::RectangleShape textBg({tb.size.x + 8.f, tb.size.y + 4.f});
        textBg.setPosition({tb.position.x - 4.f, tb.position.y - 2.f});
        textBg.setFillColor(sf::Color(49, 106, 197)); // Windows selection blue
        win.draw(textBg);
        
        shiftedLabel.setFillColor(sf::Color::White);
    } else {
        shiftedLabel.setFillColor(sf::Color::White);
    }
    
    win.draw(shiftedLabel);
}

// ═══════════════════════════════════════════════════════════════
//  ContextMenu
// ═══════════════════════════════════════════════════════════════
ContextMenu::ContextMenu(const sf::Font& font) {
    background.setFillColor(C::MenuBg);
    // Bevel will draw the outlines
}

void ContextMenu::show(sf::Vector2f p, const std::vector<MenuItem>& newItems,
                       const sf::Font& font) {
    items = newItems;
    position = p;

    // clamp so menu doesn't go off-screen
    float h = items.size() * itemHeight + 4.f;
    if (position.x + width  > WIN_W) position.x = WIN_W - width;
    if (position.y + h       > DESKTOP_H) position.y = DESKTOP_H - h;
    if (position.x < 0) position.x = 0;
    if (position.y < 0) position.y = 0;

    background.setSize({width, h});
    background.setPosition(position);
    background.setFillColor(C::XpMenuBg); // XP Classic menu gray

    itemTexts.clear();
    for (size_t i = 0; i < items.size(); ++i) {
        sf::Text t(font);
        t.setString("  " + items[i].label);
        t.setCharacterSize(14);
        t.setFillColor(C::XpMenuText); // Black text for XP menu
        t.setPosition({position.x + 4.f, position.y + 3.f + i * itemHeight});
        itemTexts.push_back(t);
    }
    visible = true;
    hoveredIdx = -1;
}

void ContextMenu::hide() {
    visible = false;
    items.clear();
    itemTexts.clear();
    hoveredIdx = -1;
}

bool ContextMenu::contains(sf::Vector2i mp) const {
    float fx = static_cast<float>(mp.x);
    float fy = static_cast<float>(mp.y);
    sf::FloatRect r = background.getGlobalBounds();
    return r.contains({fx, fy});
}

int ContextMenu::itemAt(sf::Vector2i mp) const {
    if (!contains(mp)) return -1;
    float relY = static_cast<float>(mp.y) - position.y;
    int idx = static_cast<int>(relY / itemHeight);
    if (idx < 0 || idx >= static_cast<int>(items.size())) return -1;
    return idx;
}

void ContextMenu::updateHover(sf::Vector2i mp) {
    hoveredIdx = itemAt(mp);
}

void ContextMenu::draw(sf::RenderWindow& win) const {
    if (!visible) return;
    
    // Draw background
    win.draw(background);
    
    // Draw classic Windows XP single-pixel gray border around the menu
    sf::RectangleShape borderRect(background.getSize());
    borderRect.setPosition(background.getPosition());
    borderRect.setFillColor(sf::Color::Transparent);
    borderRect.setOutlineColor(C::XpMenuBorder);
    borderRect.setOutlineThickness(1.f);
    win.draw(borderRect);
    
    for (size_t i = 0; i < itemTexts.size(); ++i) {
        sf::Text itemText = itemTexts[i];
        if (static_cast<int>(i) == hoveredIdx) {
            // Draw Luna Selection Blue block
            sf::RectangleShape hoverBg({width - 6.f, itemHeight - 2.f});
            hoverBg.setPosition({position.x + 3.f, position.y + 1.f + i * itemHeight});
            hoverBg.setFillColor(C::XpMenuHover); // Luna blue
            win.draw(hoverBg);
            
            // Text color changes to white on hover
            itemText.setFillColor(sf::Color::White);
        } else {
            itemText.setFillColor(C::XpMenuText); // Black text
        }
        win.draw(itemText);
    }
}

// ═══════════════════════════════════════════════════════════════
//  GameWindow
// ═══════════════════════════════════════════════════════════════
GameWindow::GameWindow(const sf::Font& font, const std::string& type)
    : appType(type),
      titleText(font, "", 14),
      bodyText(font, "", 14),
      bodyText2(font, "", 14)
{
    float w = 340.f, h = 250.f;
    if (appType == "calculator") { w = 240.f; h = 280.f; }
    if (appType == "folder")     { w = 300.f; h = 220.f; }

    titleBar.setSize({w, 26.f});
    titleBar.setFillColor(C::WindowTitle);

    body.setSize({w, h - 26.f});
    body.setPosition({0.f, 26.f});
    body.setFillColor(C::WindowBody);

    closeBtn.setSize({20.f, 18.f});
    closeBtn.setFillColor(C::CloseBtn);

    maxBtn.setSize({20.f, 18.f});
    maxBtn.setFillColor(C::WindowTitle);

    minBtn.setSize({20.f, 18.f});
    minBtn.setFillColor(C::WindowTitle);

    titleText.setFillColor(C::WinTitleText);
    titleText.setPosition({6.f, 4.f});
    if      (appType == "notepad")    titleText.setString("Notepad");
    else if (appType == "calculator") titleText.setString("Calculator");
    else if (appType == "folder")     titleText.setString("Folder");

    bodyText.setFillColor(C::TextWhite);
    bodyText.setPosition({10.f, 34.f});

    bodyText2.setFillColor(C::TextWhite);
    bodyText2.setPosition({10.f, 54.f});

    updateContent();
}

void GameWindow::setPos(sf::Vector2f p) {
    titleBar.setPosition(p);
    body.setPosition({p.x, p.y + 26.f});
    
    float w = titleBar.getSize().x;
    closeBtn.setPosition({p.x + w - 24.f, p.y + 4.f});
    maxBtn.setPosition({p.x + w - 46.f, p.y + 4.f});
    minBtn.setPosition({p.x + w - 68.f, p.y + 4.f});
    
    titleText.setPosition({p.x + 6.f, p.y + 4.f});
    bodyText.setPosition({p.x + 10.f, p.y + 34.f});
    bodyText2.setPosition({p.x + 10.f, p.y + 54.f});
}

void GameWindow::setSize(sf::Vector2f sz) {
    titleBar.setSize({sz.x, 26.f});
    body.setSize({sz.x, sz.y - 26.f});
    setPos(titleBar.getPosition()); // Recalculate button positions
}

sf::FloatRect GameWindow::bounds() const {
    sf::Vector2f tp = titleBar.getPosition();
    sf::Vector2f bs = body.getSize();
    return sf::FloatRect(tp, {titleBar.getSize().x, 26.f + bs.y});
}

sf::FloatRect GameWindow::titleBarBounds() const {
    sf::Vector2f tp = titleBar.getPosition();
    return sf::FloatRect(tp, {titleBar.getSize().x, 26.f});
}

sf::FloatRect GameWindow::closeBtnBounds() const {
    return closeBtn.getGlobalBounds();
}

sf::FloatRect GameWindow::maxBtnBounds() const {
    return maxBtn.getGlobalBounds();
}

sf::FloatRect GameWindow::minBtnBounds() const {
    return minBtn.getGlobalBounds();
}

ResizeDir GameWindow::hitResizeBorder(sf::Vector2i mp) const {
    if (!isOpen || isMinimized || isMaximized) return ResizeDir::None;
    sf::FloatRect r = bounds();
    float border = 8.f;
    
    bool nearRight = (mp.x >= r.position.x + r.size.x - border && mp.x <= r.position.x + r.size.x + 2.f);
    bool nearBottom = (mp.y >= r.position.y + r.size.y - border && mp.y <= r.position.y + r.size.y + 2.f);
    bool insideX = (mp.x >= r.position.x && mp.x <= r.position.x + r.size.x);
    bool insideY = (mp.y >= r.position.y && mp.y <= r.position.y + r.size.y);
    
    if (nearRight && nearBottom) return ResizeDir::BottomRight;
    if (nearRight && insideY) return ResizeDir::Right;
    if (nearBottom && insideX) return ResizeDir::Bottom;
    return ResizeDir::None;
}

bool GameWindow::containsTitleBar(sf::Vector2i mp) const {
    return titleBarBounds().contains({static_cast<float>(mp.x), static_cast<float>(mp.y)});
}

bool GameWindow::containsCloseBtn(sf::Vector2i mp) const {
    return closeBtnBounds().contains({static_cast<float>(mp.x), static_cast<float>(mp.y)});
}

bool GameWindow::containsMaxBtn(sf::Vector2i mp) const {
    return maxBtnBounds().contains({static_cast<float>(mp.x), static_cast<float>(mp.y)});
}

bool GameWindow::containsMinBtn(sf::Vector2i mp) const {
    return minBtnBounds().contains({static_cast<float>(mp.x), static_cast<float>(mp.y)});
}

void GameWindow::handleTextInput(uint32_t unicode) {
    // Backspace
    if (unicode == 8) {
        if (!userInput.empty()) userInput.pop_back();
        updateContent();
        return;
    }
    // Enter — submit
    if (unicode == 13) {
        // Notepad riddle
        if (appType == "notepad" && puzzleStep == 0) {
            if (userInput == "echo" || userInput == "Echo" || userInput == "ECHO") {
                puzzleStep = 1;
                puzzleSolved = true;
            } else {
                bodyText2.setString("Wrong! Try again...");
                bodyText2.setFillColor(sf::Color(200, 0, 0));
            }
        }
        // Calculator code
        else if (appType == "calculator" && puzzleStep == 0) {
            if (userInput == "1337") {
                puzzleStep = 1;
                puzzleSolved = true;
            } else {
                bodyText2.setString("Wrong code! Try again...");
                bodyText2.setFillColor(sf::Color(200, 0, 0));
            }
        }
        // Folder final answer
        else if (appType == "folder") {
            if (userInput == "77") {
                puzzleStep = 1;
                puzzleSolved = true;
            } else {
                bodyText2.setString("That's not right...");
                bodyText2.setFillColor(sf::Color(200, 0, 0));
            }
        }
        userInput.clear();
        updateContent();
        return;
    }
    // Printable ASCII range
    if (unicode >= 32 && unicode < 127 && userInput.size() < 40) {
        userInput += static_cast<char>(unicode);
        updateContent();
    }
}

void GameWindow::updateContent() {
    bodyText2.setFillColor(C::TextWhite);

    if (appType == "notepad") {
        if (puzzleStep == 0) {
            bodyText.setString(
                "RIDDLE:\n"
                "I speak without a mouth and hear\n"
                "without ears. I have no body, but\n"
                "I come alive with wind. What am I?\n\n"
                "Your answer: " + userInput + "_");
            bodyText2.setString("(Type and press Enter)");
        } else {
            bodyText.setString("CORRECT! The answer is \"echo\".\n\n"
                               "The password for the\n"
                               "Calculator is: 1337");
            bodyText2.setString("Puzzle solved!");
            bodyText2.setFillColor(sf::Color(0, 150, 0));
        }
    }
    else if (appType == "calculator") {
        if (puzzleStep == 0) {
            bodyText.setString(
                "ENTER CODE:\n\n"
                "   [" + userInput + "_]\n\n"
                "Type the code and press Enter.");
            bodyText2.setString("");
        } else {
            bodyText.setString(
                "CODE ACCEPTED!\n\n"
                "UNLOCKED:\n"
                "Key fragment A = 7\n\n"
                "Hint: The Folder needs\n"
                "A x 11 = ?");
            bodyText2.setString("Calculator unlocked!");
            bodyText2.setFillColor(sf::Color(0, 150, 0));
        }
    }
    else if (appType == "folder") {
        if (puzzleStep == 0) {
            bodyText.setString(
                "CLUE FOLDER\n"
                "----------------------------\n"
                "Solve the other puzzles to\n"
                "find the final code.\n\n"
                "Enter final code: " + userInput + "_");
            bodyText2.setString("Hint: Calculator holds a key...");
        } else {
            bodyText.setString(
                "PUZZLE SOLVED!\n\n"
                "7 x 11 = 77\n\n"
                "You have mastered the desktop!\n"
                "Congratulations!");
            bodyText2.setString("ALL PUZZLES COMPLETE!");
            bodyText2.setFillColor(sf::Color(0, 120, 180));
        }
    }
}

void GameWindow::draw(sf::RenderWindow& win) const {
    if (!isOpen || isMinimized) return;
    
    // Draw the overall window frame with a raised 3D pixel bevel
    sf::FloatRect winBounds = bounds();
    sf::RectangleShape frameBg(winBounds.size);
    frameBg.setPosition(winBounds.position);
    frameBg.setFillColor(C::WindowBody);
    win.draw(frameBg);
    drawDoubleBevel(win, winBounds, true); // Raised border for window
    
    // Draw Titlebar background
    win.draw(titleBar);
    
    // Draw Sunken Window Body content area
    sf::FloatRect bodyBounds = body.getGlobalBounds();
    sf::RectangleShape bodyBg(bodyBounds.size);
    bodyBg.setPosition(bodyBounds.position);
    bodyBg.setFillColor(C::WindowBody);
    win.draw(bodyBg);
    drawDoubleBevel(win, bodyBounds, false); // Sunken border for body content

    // Draw close button as a raised button
    win.draw(closeBtn);
    drawDoubleBevel(win, closeBtn.getGlobalBounds(), true);

    // Draw maximize button as a raised button
    win.draw(maxBtn);
    drawDoubleBevel(win, maxBtn.getGlobalBounds(), true);

    // Draw minimize button as a raised button
    win.draw(minBtn);
    drawDoubleBevel(win, minBtn.getGlobalBounds(), true);

    win.draw(titleText);
    win.draw(bodyText);
    win.draw(bodyText2);

    // Draw "X" text inside the close button
    sf::Text xText(titleText);
    xText.setString("X");
    xText.setCharacterSize(11);
    xText.setFillColor(sf::Color::White);
    
    // Center the X text in the close button
    sf::FloatRect xb = xText.getLocalBounds();
    xText.setOrigin({xb.position.x + xb.size.x / 2.f, xb.position.y + xb.size.y / 2.f});
    sf::Vector2f cbPos = closeBtn.getPosition();
    sf::Vector2f cbSize = closeBtn.getSize();
    xText.setPosition({cbPos.x + cbSize.x / 2.f, cbPos.y + cbSize.y / 2.f - 1.f});
    win.draw(xText);

    // Draw square icon on maximize button
    sf::Vector2f mbPos = maxBtn.getPosition();
    sf::Vector2f mbSize = maxBtn.getSize();
    sf::RectangleShape maxBox({8.f, 8.f});
    maxBox.setPosition({mbPos.x + mbSize.x / 2.f - 4.f, mbPos.y + mbSize.y / 2.f - 4.f});
    maxBox.setFillColor(sf::Color::Transparent);
    maxBox.setOutlineColor(sf::Color::White);
    maxBox.setOutlineThickness(1.5f);
    win.draw(maxBox);

    // Draw line icon on minimize button
    sf::Vector2f minbPos = minBtn.getPosition();
    sf::Vector2f minbSize = minBtn.getSize();
    sf::RectangleShape minLine({8.f, 2.f});
    minLine.setPosition({minbPos.x + minbSize.x / 2.f - 4.f, minbPos.y + minbSize.y / 2.f + 2.f});
    minLine.setFillColor(sf::Color::White);
    win.draw(minLine);
}

// ═══════════════════════════════════════════════════════════════
//  Taskbar
// ═══════════════════════════════════════════════════════════════
Taskbar::Taskbar(const sf::Font& font, const sf::Texture* startTex)
    : clockText(font, "", 14),
      startTexPtr(startTex)
{
    bg.setSize({WIN_W, h});
    bg.setPosition({0.f, DESKTOP_H});
    bg.setFillColor(C::XpBlueDark);

    clockText.setFillColor(sf::Color(220, 240, 255)); // XP Clock text color (light blue-white)
}

void Taskbar::update(std::vector<GameWindow>& windows, const sf::Font& font) {
    // Clock
    auto t = clock.getElapsedTime();
    int totalSec = static_cast<int>(t.asSeconds());
    int hours   = (totalSec / 3600) % 24;
    int minutes = (totalSec / 60) % 60;
    char buf[16];
    snprintf(buf, sizeof(buf), "%02d:%02d", hours, minutes);
    clockText.setString(buf);
    
    // Position clock text inside the system tray box on the right
    sf::FloatRect ctBounds = clockText.getLocalBounds();
    clockText.setOrigin({0.f, ctBounds.size.y / 2.f});
    clockText.setPosition({WIN_W - 75.f, DESKTOP_H + h / 2.f - 2.f});

    // Window buttons - starting after the green "start" button (at x = 120)
    winBtns.clear();
    winBtnLabels.clear();
    float bx = 125.f;
    for (size_t i = 0; i < windows.size(); ++i) {
        if (!windows[i].isOpen) continue;
        sf::RectangleShape btn({110.f, 28.f});
        btn.setPosition({bx, DESKTOP_H + (h - 28.f) / 2.f});
        
        // XP Luna active (dark blue) vs inactive (lighter blue-gray) colors
        btn.setFillColor(windows[i].isMinimized
            ? sf::Color(60, 130, 226)  // Inactive XP blue
            : sf::Color(25, 82, 171)); // Active dark XP blue
            
        winBtns.push_back(btn);

        sf::Text lbl(font);
        std::string title;
        if (windows[i].appType == "notepad")    title = "Notepad";
        if (windows[i].appType == "calculator") title = "Calculator";
        if (windows[i].appType == "folder")     title = "Folder";
        lbl.setString(title);
        lbl.setCharacterSize(12);
        lbl.setFillColor(sf::Color::White);
        
        sf::FloatRect lblB = lbl.getLocalBounds();
        lbl.setOrigin({0.f, lblB.size.y / 2.f});
        lbl.setPosition({bx + 8.f, DESKTOP_H + h / 2.f - 2.f});
        winBtnLabels.push_back(lbl);

        bx += 118.f;
    }
}

int Taskbar::windowBtnAt(sf::Vector2i mp) const {
    float fx = static_cast<float>(mp.x);
    float fy = static_cast<float>(mp.y);
    for (size_t i = 0; i < winBtns.size(); ++i) {
        sf::FloatRect r = winBtns[i].getGlobalBounds();
        if (r.contains({fx, fy})) return static_cast<int>(i);
    }
    return -1;
}

void Taskbar::draw(sf::RenderWindow& win) const {
    // 1. Draw smooth vertical gradient for the Windows XP Luna taskbar
    sf::VertexArray grad(sf::PrimitiveType::TriangleStrip, 4);
    grad[0] = sf::Vertex{ {0.f, DESKTOP_H}, C::XpBlueLight };
    grad[1] = sf::Vertex{ {WIN_W, DESKTOP_H}, C::XpBlueLight };
    grad[2] = sf::Vertex{ {0.f, WIN_H}, C::XpBlueDark };
    grad[3] = sf::Vertex{ {WIN_W, WIN_H}, C::XpBlueDark };
    win.draw(grad);
    
    // Draw classic XP taskbar top highlight line
    sf::RectangleShape topHighlight({WIN_W, 2.f});
    topHighlight.setPosition({0.f, DESKTOP_H});
    topHighlight.setFillColor(sf::Color(100, 160, 255));
    win.draw(topHighlight);

    // 2. Draw Start Button on the left
    if (startTexPtr) {
        sf::Sprite startSp(*startTexPtr);
        float startBtnH = h - 16.f; // 48px
        startSp.setPosition({8.f, DESKTOP_H + (h - startBtnH) / 2.f});
        sf::Vector2u texSize = startTexPtr->getSize();
        startSp.setScale({110.f / texSize.x, startBtnH / texSize.y});
        win.draw(startSp);
    } else {
        // Fallback start button (green block)
        sf::RectangleShape startFallback({110.f, h - 16.f});
        startFallback.setPosition({8.f, DESKTOP_H + 8.f});
        startFallback.setFillColor(sf::Color(50, 180, 50));
        win.draw(startFallback);
        drawDoubleBevel(win, startFallback.getGlobalBounds(), true);
    }
    
    // 3. Draw System Tray (Clock box) on the right
    sf::FloatRect clockRect({WIN_W - 100.f, DESKTOP_H + 6.f}, {92.f, h - 12.f});
    sf::RectangleShape clockBg(clockRect.size);
    clockBg.setPosition(clockRect.position);
    clockBg.setFillColor(C::XpBlueTray); // Windows XP dark blue tray
    win.draw(clockBg);
    
    // Add vertical divider line on the left of system tray
    sf::RectangleShape trayDivider({2.f, h});
    trayDivider.setPosition({WIN_W - 102.f, DESKTOP_H});
    trayDivider.setFillColor(sf::Color(16, 50, 150));
    win.draw(trayDivider);
    
    win.draw(clockText);
    
    // 4. Draw Window buttons
    for (size_t i = 0; i < winBtns.size(); ++i) {
        win.draw(winBtns[i]);
        
        bool isSunken = winBtns[i].getFillColor() == sf::Color(25, 82, 171); // Active state
        // For XP Luna buttons, draw 3D bevels
        drawDoubleBevel(win, winBtns[i].getGlobalBounds(), !isSunken);
        
        // Draw label (shift text slightly down-right if sunken for tactile effect)
        sf::Text lbl = winBtnLabels[i];
        if (isSunken) {
            lbl.move({1.f, 1.f});
        }
        win.draw(lbl);
    }
}

// ═══════════════════════════════════════════════════════════════
//  Helpers
// ═══════════════════════════════════════════════════════════════
std::string timeString() {
    return "12:00"; // simple placeholder; the taskbar clock uses sf::Clock
}

sf::Vector2f clampWindowPos(const GameWindow& w, sf::Vector2f desired) {
    float winW = w.titleBar.getSize().x;
    if (desired.x < 0) desired.x = 0;
    if (desired.x + winW > WIN_W) desired.x = WIN_W - winW;
    if (desired.y < 0) desired.y = 0;
    if (desired.y > DESKTOP_H - 26.f) desired.y = DESKTOP_H - 26.f;
    return desired;
}
