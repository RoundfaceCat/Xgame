cpp_content = """#include "DesktopGame.hpp"
#include <iostream>
#include <cstring>
#include <sstream>

// U8 string helper for MSVC
inline sf::String U8(const char* s) { return sf::String::fromUtf8(s, s + std::strlen(s)); }
inline sf::String U8(const std::string& s) { return sf::String::fromUtf8(s.begin(), s.end()); }

Desktop* g_Desktop = nullptr;

// ── Drawing Helpers ─────────────────────────────────────────────
void drawDoubleBevel(sf::RenderWindow& win, sf::FloatRect rect, bool raised) {
    sf::RectangleShape line(sf::Vector2f(rect.width, 1.f));
    // Top outer
    line.setPosition({rect.position.x, rect.position.y});
    line.setFillColor(raised ? C::BevelLight : C::BevelDark);
    win.draw(line);
    // Top inner
    line.setPosition({rect.position.x + 1.f, rect.position.y + 1.f});
    line.setSize({rect.width - 2.f, 1.f});
    line.setFillColor(raised ? C::BevelMedium : C::TextBlack);
    win.draw(line);

    // Left outer
    line.setSize({1.f, rect.height});
    line.setPosition({rect.position.x, rect.position.y});
    line.setFillColor(raised ? C::BevelLight : C::BevelDark);
    win.draw(line);
    // Left inner
    line.setSize({1.f, rect.height - 2.f});
    line.setPosition({rect.position.x + 1.f, rect.position.y + 1.f});
    line.setFillColor(raised ? C::BevelMedium : C::TextBlack);
    win.draw(line);

    // Bottom outer
    line.setSize({rect.width, 1.f});
    line.setPosition({rect.position.x, rect.position.y + rect.height - 1.f});
    line.setFillColor(raised ? C::TextBlack : C::BevelLight);
    win.draw(line);
    // Bottom inner
    line.setSize({rect.width - 2.f, 1.f});
    line.setPosition({rect.position.x + 1.f, rect.position.y + rect.height - 2.f});
    line.setFillColor(raised ? C::BevelDark : C::BevelMedium);
    win.draw(line);

    // Right outer
    line.setSize({1.f, rect.height});
    line.setPosition({rect.position.x + rect.width - 1.f, rect.position.y});
    line.setFillColor(raised ? C::TextBlack : C::BevelLight);
    win.draw(line);
    // Right inner
    line.setSize({1.f, rect.height - 2.f});
    line.setPosition({rect.position.x + rect.width - 2.f, rect.position.y + 1.f});
    line.setFillColor(raised ? C::BevelDark : C::BevelMedium);
    win.draw(line);
}

void drawCaptionButton(sf::RenderWindow& win, sf::FloatRect rect,
                       const sf::Font& font, const std::string& glyph,
                       sf::Color face, bool pressed) {
    sf::RectangleShape bg(rect.size);
    bg.setPosition(rect.position);
    bg.setFillColor(face);
    win.draw(bg);
    drawDoubleBevel(win, rect, !pressed);

    sf::Text t(font, U8(glyph), 11u);
    t.setFillColor(C::TextBlack);
    t.setStyle(sf::Text::Bold);
    sf::FloatRect tr = t.getLocalBounds();
    t.setOrigin({tr.position.x + tr.size.x / 2.f, tr.position.y + tr.size.y / 2.f});
    t.setPosition({rect.position.x + rect.width / 2.f + (pressed ? 1.f : 0.f),
                   rect.position.y + rect.height / 2.f + (pressed ? 1.f : 0.f)});
    win.draw(t);
}

// ═══════════════════════════════════════════════════════════════
// 1. StoryManager - 剧情管理
// ═══════════════════════════════════════════════════════════════
bool StoryManager::tryUnlockArchive(const std::string& input) {
    if (input == keyPart1) {
        archiveOpened = true;
        if (g_Desktop && g_Desktop->secretIcon) g_Desktop->secretIcon->highlight = true;
        return true;
    }
    return false;
}

bool StoryManager::tryPerfectKey(const std::string& input) {
    if (input == fullKey) {
        hasPerfectClear = true;
        triggerPerfectEnding();
        return true;
    }
    return false;
}

void StoryManager::markDocClueRead() { hasDocClue = true; advanceStage(2); }
void StoryManager::markBrowserClueSeen() { hasBrowserClue = true; advanceStage(3); }
void StoryManager::unlockRecycleBin() { recycleBinUnlocked = true; advanceStage(4); }
void StoryManager::onKeyPart2Restored() {
    keyPart2Restored = true;
    unlockRewardExe();
}
void StoryManager::unlockRewardExe() {
    isRewardUnlocked = true;
    if (g_Desktop) g_Desktop->syncStoryVisuals();
}

void StoryManager::triggerNormalEnding() {
    if (g_Desktop && !hasPerfectClear) {
        g_Desktop->addWindow(new RewardAnimWindow({WIN_W/2 - 250.f, WIN_H/2 - 200.f}, 500.f, 400.f, g_Desktop->systemFont));
    }
}
void StoryManager::triggerPerfectEnding() {
    if (g_Desktop) {
        g_Desktop->addWindow(new RewardAnimWindow({WIN_W/2 - 250.f, WIN_H/2 - 200.f}, 500.f, 400.f, g_Desktop->systemFont));
    }
}
void StoryManager::advanceStage(int stage) {
    if (storyStage < stage) storyStage = stage;
}

// ═══════════════════════════════════════════════════════════════
// 2. 基础 UI 组件
// ═══════════════════════════════════════════════════════════════
ContextMenu::ContextMenu() : UIComponent({0, 0}, 150.f, 0.f) {
    background.setFillColor(C::MenuBg);
    shadow.setFillColor(sf::Color(0, 0, 0, 80));
    visible = false;
}

void ContextMenu::setFont(const sf::Font& font) { menuFont = &font; }

void ContextMenu::show(sf::Vector2f p, const std::vector<MenuItem>& newItems) {
    items = newItems;
    itemTexts.clear();
    position = p;
    height = BORDER * 2 + items.size() * itemHeight;

    if (position.x + width > WIN_W) position.x = WIN_W - width;
    if (position.y + height > WIN_H - TASKBAR_H) position.y = WIN_H - TASKBAR_H - height;
    if (position.y < 0) position.y = 0;

    background.setSize({width, height});
    background.setPosition(position);
    shadow.setSize({width, height});
    shadow.setPosition({position.x + 4.f, position.y + 4.f});

    for (const auto& it : items) {
        sf::Text t(*menuFont, U8(it.label), 12u);
        if (it.isDefault) t.setStyle(sf::Text::Bold);
        t.setFillColor(it.enabled ? C::TextBlack : sf::Color(160, 160, 160));
        itemTexts.push_back(t);
    }
    open = true;
    hoveredIdx = -1;
}

void ContextMenu::hide() { open = false; items.clear(); }

int ContextMenu::itemAt(sf::Vector2f mp) const {
    if (!open || !background.getGlobalBounds().contains(mp)) return -1;
    float relY = mp.y - position.y - BORDER;
    if (relY < 0 || relY >= items.size() * itemHeight) return -1;
    return static_cast<int>(relY / itemHeight);
}

void ContextMenu::updateHover(sf::Vector2f mp) {
    int idx = itemAt(mp);
    if (idx >= 0 && idx < (int)items.size() && items[idx].enabled && !items[idx].separator)
        hoveredIdx = idx;
    else
        hoveredIdx = -1;
}

bool ContextMenu::tryClick(sf::Vector2f mp) {
    if (!open) return false;
    int idx = itemAt(mp);
    if (idx >= 0 && idx < (int)items.size()) {
        if (items[idx].enabled && items[idx].action) {
            items[idx].action();
        }
        hide();
        return true;
    }
    return false;
}

void ContextMenu::render(sf::RenderWindow& win) {
    if (!open) return;
    win.draw(shadow);
    win.draw(background);
    drawDoubleBevel(win, bounds(), true);

    for (std::size_t i = 0; i < items.size(); ++i) {
        float y = position.y + BORDER + i * itemHeight;
        if (items[i].separator) {
            sf::RectangleShape line({width - 4.f, 1.f});
            line.setPosition({position.x + 2.f, y + itemHeight / 2.f});
            line.setFillColor(C::BevelDark);
            win.draw(line);
            line.setPosition({position.x + 2.f, y + itemHeight / 2.f + 1.f});
            line.setFillColor(C::BevelLight);
            win.draw(line);
        } else {
            if ((int)i == hoveredIdx) {
                sf::RectangleShape hl({width - 4.f, itemHeight});
                hl.setPosition({position.x + 2.f, y});
                hl.setFillColor(C::MenuHover);
                win.draw(hl);
                itemTexts[i].setFillColor(C::TextWhite);
            } else {
                itemTexts[i].setFillColor(items[i].enabled ? C::TextBlack : sf::Color(160, 160, 160));
            }
            itemTexts[i].setPosition({position.x + 24.f, y + 2.f});
            win.draw(itemTexts[i]);
        }
    }
}

bool ContextMenu::isMouseHit(sf::Vector2f mp) { return open && bounds().contains(mp); }

// ═══════════════════════════════════════════════════════════════
// 3. FileObject 体系实现
// ═══════════════════════════════════════════════════════════════
void TextFile::open() {
    if (g_Desktop)
        g_Desktop->addWindow(new NotepadWindow(name, content, {100, 100}, 500, 400, g_Desktop->systemFont));
}

void LockedFile::open() {
    if (g_Desktop)
        g_Desktop->addWindow(new PasswordDialog({200, 150}, 320, 180, g_Desktop->systemFont, 0));
}

void DeletedFile::open() {
    if (g_Desktop) {
        if (!StoryManager::getInstance().keyPart2Restored) {
            g_Desktop->addWindow(new MsgBoxWin("回收站提示", "该文件已被删除，请先将其还原。\\n提示：选中文件后点击上方“还原”按钮。", {300, 200}, 300, 150, g_Desktop->systemFont));
        } else {
            g_Desktop->addWindow(new NotepadWindow(name, content, {100, 100}, 500, 400, g_Desktop->systemFont));
        }
    }
}

void ExeProgramFile::open() {
    if (g_Desktop)
        g_Desktop->addWindow(new RewardAnimWindow({WIN_W/2 - 250, WIN_H/2 - 200}, 500, 400, g_Desktop->systemFont));
}

"""
with open(r"d:\Xgame\Xgame\write_cpp_part1.py", "w", encoding="utf-8") as f:
    f.write('cpp_content = ' + repr(cpp_content) + '\n')
    f.write('with open(r"d:\\Xgame\\Xgame\\src\\DesktopGame.cpp", "w", encoding="utf-8") as f:\n')
    f.write('    f.write(cpp_content)\n')
    f.write('print("Part 1 written")\n')
