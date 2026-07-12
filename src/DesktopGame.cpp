#include "DesktopGame.hpp"
#include <iostream>
#include <cstring>
#include <sstream>

// U8 string helper for MSVC
inline sf::String U8(const char* s) { return sf::String::fromUtf8(s, s + std::strlen(s)); }
inline sf::String U8(const std::string& s) { return sf::String::fromUtf8(s.begin(), s.end()); }

Desktop* g_Desktop = nullptr;

// ── Drawing Helpers ─────────────────────────────────────────────
void drawDoubleBevel(sf::RenderWindow& win, sf::FloatRect rect, bool raised) {
    sf::RectangleShape line(sf::Vector2f(rect.size.x, 1.f));
    line.setPosition(rect.position);
    line.setFillColor(raised ? C::BevelLight : C::BevelDark);
    win.draw(line);
    line.setPosition(sf::Vector2f(rect.position.x + 1.f, rect.position.y + 1.f));
    line.setSize(sf::Vector2f(rect.size.x - 2.f, 1.f));
    line.setFillColor(raised ? C::BevelMedium : C::TextBlack);
    win.draw(line);
    line.setSize(sf::Vector2f(1.f, rect.size.y));
    line.setPosition(rect.position);
    line.setFillColor(raised ? C::BevelLight : C::BevelDark);
    win.draw(line);
    line.setSize(sf::Vector2f(1.f, rect.size.y - 2.f));
    line.setPosition(sf::Vector2f(rect.position.x + 1.f, rect.position.y + 1.f));
    line.setFillColor(raised ? C::BevelMedium : C::TextBlack);
    win.draw(line);
    line.setSize(sf::Vector2f(rect.size.x, 1.f));
    line.setPosition(sf::Vector2f(rect.position.x, rect.position.y + rect.size.y - 1.f));
    line.setFillColor(raised ? C::TextBlack : C::BevelLight);
    win.draw(line);
    line.setSize(sf::Vector2f(rect.size.x - 2.f, 1.f));
    line.setPosition(sf::Vector2f(rect.position.x + 1.f, rect.position.y + rect.size.y - 2.f));
    line.setFillColor(raised ? C::BevelDark : C::BevelMedium);
    win.draw(line);
    line.setSize(sf::Vector2f(1.f, rect.size.y));
    line.setPosition(sf::Vector2f(rect.position.x + rect.size.x - 1.f, rect.position.y));
    line.setFillColor(raised ? C::TextBlack : C::BevelLight);
    win.draw(line);
    line.setSize(sf::Vector2f(1.f, rect.size.y - 2.f));
    line.setPosition(sf::Vector2f(rect.position.x + rect.size.x - 2.f, rect.position.y + 1.f));
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
    t.setOrigin(sf::Vector2f(tr.position.x + tr.size.x / 2.f, tr.position.y + tr.size.y / 2.f));
    t.setPosition(sf::Vector2f(rect.position.x + rect.size.x / 2.f + (pressed ? 1.f : 0.f), rect.position.y + rect.size.y / 2.f + (pressed ? 1.f : 0.f)));
    win.draw(t);
}

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
void StoryManager::advanceStage(int stage) { if (storyStage < stage) storyStage = stage; }

ContextMenu::ContextMenu() : UIComponent({0, 0}, 150.f, 0.f) {
    background.setFillColor(C::MenuBg);
    shadow.setFillColor(sf::Color(0, 0, 0, 80));
    visible = false;
}
void ContextMenu::setFont(const sf::Font& font) { menuFont = &font; }
void ContextMenu::show(sf::Vector2f p, const std::vector<MenuItem>& newItems) {
    items = newItems; itemTexts.clear(); position = p;
    height = BORDER * 2 + items.size() * itemHeight;
    if (position.x + width > WIN_W) position.x = WIN_W - width;
    if (position.y + height > WIN_H - TASKBAR_H) position.y = WIN_H - TASKBAR_H - height;
    if (position.y < 0) position.y = 0;
    background.setSize(sf::Vector2f(width, height)); background.setPosition(position);
    shadow.setSize(sf::Vector2f(width, height)); shadow.setPosition(sf::Vector2f(position.x + 4.f, position.y + 4.f));
    for (const auto& it : items) {
        sf::Text t(*menuFont, U8(it.label), 12u);
        if (it.isDefault) t.setStyle(sf::Text::Bold);
        t.setFillColor(it.enabled ? C::TextBlack : sf::Color(160, 160, 160));
        itemTexts.push_back(t);
    }
    open = true; hoveredIdx = -1;
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
    if (idx >= 0 && idx < (int)items.size() && items[idx].enabled && !items[idx].separator) hoveredIdx = idx;
    else hoveredIdx = -1;
}
bool ContextMenu::tryClick(sf::Vector2f mp) {
    if (!open) return false;
    int idx = itemAt(mp);
    if (idx >= 0 && idx < (int)items.size()) {
        if (items[idx].enabled && items[idx].action) items[idx].action();
        hide(); return true;
    }
    return false;
}
void ContextMenu::render(sf::RenderWindow& win) {
    if (!open) return;
    win.draw(shadow); win.draw(background); drawDoubleBevel(win, bounds(), true);
    for (std::size_t i = 0; i < items.size(); ++i) {
        float y = position.y + BORDER + i * itemHeight;
        if (items[i].separator) {
            sf::RectangleShape line(sf::Vector2f(width - 4.f, 1.f));
            line.setPosition(sf::Vector2f(position.x + 2.f, y + itemHeight / 2.f)); line.setFillColor(C::BevelDark); win.draw(line);
            line.setPosition(sf::Vector2f(position.x + 2.f, y + itemHeight / 2.f + 1.f)); line.setFillColor(C::BevelLight); win.draw(line);
        } else {
            if ((int)i == hoveredIdx) {
                sf::RectangleShape hl(sf::Vector2f(width - 4.f, itemHeight));
                hl.setPosition(sf::Vector2f(position.x + 2.f, y)); hl.setFillColor(C::MenuHover); win.draw(hl);
                itemTexts[i].setFillColor(C::TextWhite);
            } else {
                itemTexts[i].setFillColor(items[i].enabled ? C::TextBlack : sf::Color(160, 160, 160));
            }
            itemTexts[i].setPosition(sf::Vector2f(position.x + 24.f, y + 2.f)); win.draw(itemTexts[i]);
        }
    }
}
bool ContextMenu::isMouseHit(sf::Vector2f mp) { return open && bounds().contains(mp); }

void TextFile::open() {
    if (g_Desktop) g_Desktop->addWindow(new NotepadWindow(name, content, {100, 100}, 500, 400, g_Desktop->systemFont));
}
void LockedFile::open() {
    if (g_Desktop) g_Desktop->addWindow(new PasswordDialog({200, 150}, 320, 180, g_Desktop->systemFont, 0));
}
void DeletedFile::open() {
    if (g_Desktop) {
        if (!StoryManager::getInstance().keyPart2Restored) {
            g_Desktop->addWindow(new MsgBoxWin("回收站提示", "该文件已被删除，请先将其还原。\n提示：选中文件后点击上方“还原”按钮。", {300, 200}, 300, 150, g_Desktop->systemFont));
        } else {
            g_Desktop->addWindow(new NotepadWindow(name, content, {100, 100}, 500, 400, g_Desktop->systemFont));
        }
    }
}
void ExeProgramFile::open() {
    if (g_Desktop) g_Desktop->addWindow(new RewardAnimWindow({WIN_W/2 - 250, WIN_H/2 - 200}, 500, 400, g_Desktop->systemFont));
}

PuzzleWindowBase::PuzzleWindowBase(const std::string& t, sf::Vector2f p, float w, float h, const sf::Font& font)
    : UIComponent(p, w, h), title(t), uiFont(&font), titleText(font) {
    frame.setFillColor(C::WindowBody); titleBar.setFillColor(C::TitleActive);
    titleText.setFont(font); titleText.setCharacterSize(12); titleText.setString(U8(title));
    titleText.setFillColor(C::TextWhite); titleText.setStyle(sf::Text::Bold);
}
sf::FloatRect PuzzleWindowBase::titleBarRect() const { return sf::FloatRect(position, sf::Vector2f(width, TITLEBAR_H)); }
sf::FloatRect PuzzleWindowBase::closeBtnRect() const { return sf::FloatRect(sf::Vector2f(position.x + width - BORDER - CAPTION_BTN - 2.f, position.y + BORDER + 2.f), sf::Vector2f(CAPTION_BTN, CAPTION_BTN)); }
sf::FloatRect PuzzleWindowBase::maxBtnRect() const { return sf::FloatRect(sf::Vector2f(position.x + width - BORDER - CAPTION_BTN*2 - 4.f, position.y + BORDER + 2.f), sf::Vector2f(CAPTION_BTN, CAPTION_BTN)); }
sf::FloatRect PuzzleWindowBase::minBtnRect() const { return sf::FloatRect(sf::Vector2f(position.x + width - BORDER - CAPTION_BTN*3 - 4.f, position.y + BORDER + 2.f), sf::Vector2f(CAPTION_BTN, CAPTION_BTN)); }
sf::FloatRect PuzzleWindowBase::clientRect() const { return sf::FloatRect(sf::Vector2f(position.x + BORDER, position.y + TITLEBAR_H + 1.f), sf::Vector2f(width - 2*BORDER, height - TITLEBAR_H - BORDER - 1.f)); }
void PuzzleWindowBase::dragUpdate(sf::Vector2f mousePos) {
    position = mousePos - dragOffset;
    if (position.y < 0) position.y = 0;
    if (position.y > DESKTOP_H - TITLEBAR_H) position.y = DESKTOP_H - TITLEBAR_H;
}
void PuzzleWindowBase::closeWindow() { isOpen = false; }
void PuzzleWindowBase::minimizeWindow() { isMinimized = true; }
void PuzzleWindowBase::restoreFromTaskbar() { isMinimized = false; if (g_Desktop) g_Desktop->setActiveWindow(this); }
void PuzzleWindowBase::toggleMaximize() {
    if (!canMaximize) return;
    if (isMaximized) { position = restorePos; width = restoreW; height = restoreH; isMaximized = false; }
    else { restorePos = position; restoreW = width; restoreH = height; position = {0, 0}; width = WIN_W; height = DESKTOP_H; isMaximized = true; }
}
int PuzzleWindowBase::hitCaptionButton(sf::Vector2f mp) const {
    if (closeBtnRect().contains(mp)) return 0;
    if (canMaximize && maxBtnRect().contains(mp)) return 1;
    if (canMinimize && minBtnRect().contains(mp)) return 2;
    return -1;
}
bool PuzzleWindowBase::containsTitleBarDrag(sf::Vector2f mp) const { return titleBarRect().contains(mp) && hitCaptionButton(mp) == -1; }
void PuzzleWindowBase::renderFrame(sf::RenderWindow& win) {
    frame.setPosition(position); frame.setSize(sf::Vector2f(width, height)); win.draw(frame); drawDoubleBevel(win, bounds(), true);
    titleBar.setPosition(sf::Vector2f(position.x + BORDER, position.y + BORDER)); titleBar.setSize(sf::Vector2f(width - 2*BORDER, TITLEBAR_H - BORDER));
    titleBar.setFillColor(isActive ? C::TitleActive : C::TitleInactive); win.draw(titleBar);
    titleText.setPosition(sf::Vector2f(position.x + BORDER + 4.f, position.y + BORDER + 2.f)); win.draw(titleText);
    drawCaptionButton(win, closeBtnRect(), *uiFont, "X", C::CloseBtn);
    if (canMaximize) drawCaptionButton(win, maxBtnRect(), *uiFont, isMaximized ? "❐" : "□", C::BevelMedium);
    if (canMinimize) drawCaptionButton(win, minBtnRect(), *uiFont, "_", C::BevelMedium);
}
void PuzzleWindowBase::render(sf::RenderWindow& win) { if (!isOpen || isMinimized) return; renderFrame(win); renderContent(win); }
bool PuzzleWindowBase::isMouseHit(sf::Vector2f mousePos) { return isOpen && !isMinimized && bounds().contains(mousePos); }

NotepadWindow::NotepadWindow(const std::string& fileTitle, const std::string& content, sf::Vector2f p, float w, float h, const sf::Font& font)
    : PuzzleWindowBase(fileTitle + " - 记事本", p, w, h, font), textContent(content), docText(font) {
    docText.setFont(font); docText.setCharacterSize(14); docText.setFillColor(C::TextBlack);
    std::string wrapped; int lineLen = 0;
    for (char c : content) {
        if (c == '\n') { lineLen = 0; wrapped += c; }
        else { wrapped += c; lineLen++; if (lineLen > 50) { wrapped += "\n"; lineLen = 0; } }
    }
    docText.setString(U8(wrapped)); canMaximize = true; canMinimize = true;
}
void NotepadWindow::renderContent(sf::RenderWindow& win) {
    sf::FloatRect cr = clientRect();
    sf::RectangleShape bg(cr.size); bg.setPosition(cr.position); bg.setFillColor(C::ClientWhite); win.draw(bg); drawDoubleBevel(win, cr, false);
    docText.setPosition(sf::Vector2f(cr.position.x + 5.f, cr.position.y + 5.f)); win.draw(docText);
}
void NotepadWindow::handleInput(const sf::Event& event) {}

BrowserWindow::BrowserWindow(sf::Vector2f p, float w, float h, const sf::Font& font) : PuzzleWindowBase("Windows Internet Explorer", p, w, h, font), winFont(&font), addrText(font), contentText(font), statusText(font) {
    addressInput = "http://www.spacex.com/history";
    addrText.setFont(font); addrText.setCharacterSize(14); addrText.setFillColor(C::TextBlack); addrText.setString(U8(addressInput));
    contentText.setFont(font); contentText.setCharacterSize(14); contentText.setFillColor(C::TextBlack);
    statusText.setFont(font); statusText.setCharacterSize(12); statusText.setFillColor(C::TextBlack);
    addrBar.setFillColor(C::ClientWhite); goBtn.setFillColor(C::BevelMedium);
}
void BrowserWindow::renderContent(sf::RenderWindow& win) {
    sf::FloatRect cr = clientRect();
    sf::Text lbl(*winFont, U8("地址(D):"), 14u); lbl.setFillColor(C::TextBlack); lbl.setPosition(sf::Vector2f(cr.position.x + 5.f, cr.position.y + 10.f)); win.draw(lbl);
    addrBar.setPosition(sf::Vector2f(cr.position.x + 65.f, cr.position.y + 8.f)); addrBar.setSize(sf::Vector2f(cr.size.x - 130.f, 22.f)); win.draw(addrBar); drawDoubleBevel(win, addrBar.getGlobalBounds(), false);
    addrText.setString(U8(addressInput + (editingAddr && (int)(sf::Clock().getElapsedTime().asSeconds()*2)%2==0 ? "|" : "")));
    addrText.setPosition(sf::Vector2f(addrBar.getPosition().x + 5.f, addrBar.getPosition().y + 2.f)); win.draw(addrText);
    goBtn.setPosition(sf::Vector2f(cr.position.x + cr.size.x - 55.f, cr.position.y + 8.f)); goBtn.setSize(sf::Vector2f(45.f, 22.f)); win.draw(goBtn); drawDoubleBevel(win, goBtn.getGlobalBounds(), true);
    sf::Text goTxt(*winFont, U8("转到"), 12u); goTxt.setFillColor(C::TextBlack); goTxt.setPosition(sf::Vector2f(goBtn.getPosition().x + 10.f, goBtn.getPosition().y + 3.f)); win.draw(goTxt);
    sf::FloatRect pageRect(sf::Vector2f(cr.position.x, cr.position.y + 40.f), sf::Vector2f(cr.size.x, cr.size.y - 60.f));
    sf::RectangleShape page(pageRect.size); page.setPosition(pageRect.position); page.setFillColor(C::ClientWhite); win.draw(page); drawDoubleBevel(win, pageRect, false);
    if (showSecretPage) { contentText.setString(U8("【系统覆盖】\n已接入特斯拉奖励系统。\n请输入完整密钥：202304202012\n(请在浏览器弹窗中校验)")); } 
    else { contentText.setString(U8("SpaceX 历史档案库\n===================\n重大事件回顾：\n星舰首飞（Starship Orbital Test Flight）\n发射日期：20230420\n...\n(页面底部小字：密钥后半段与特斯拉Model S出厂编号相关)")); }
    contentText.setPosition(sf::Vector2f(pageRect.position.x + 10.f, pageRect.position.y + 10.f)); win.draw(contentText);
    sf::RectangleShape sb(sf::Vector2f(cr.size.x, 20.f)); sb.setPosition(sf::Vector2f(cr.position.x, cr.position.y + cr.size.y - 20.f)); sb.setFillColor(C::StatusBar); win.draw(sb); drawDoubleBevel(win, sb.getGlobalBounds(), false);
    statusText.setString(U8("完成")); statusText.setPosition(sf::Vector2f(sb.getPosition().x + 5.f, sb.getPosition().y + 2.f)); win.draw(statusText);
}
void BrowserWindow::handleInput(const sf::Event& event) {
    if (const auto* mb = event.getIf<sf::Event::MouseButtonPressed>(); mb && mb->button == sf::Mouse::Button::Left) {
        sf::Vector2f mp(mb->position.x, mb->position.y);
        if (addrBar.getGlobalBounds().contains(mp)) editingAddr = true; else editingAddr = false;
        if (goBtn.getGlobalBounds().contains(mp)) navigateToAddress();
    }
    if (editingAddr && event.is<sf::Event::TextEntered>()) {
        if (event.getIf<sf::Event::TextEntered>()->unicode == 8 && !addressInput.empty()) addressInput.pop_back();
        else if (event.getIf<sf::Event::TextEntered>()->unicode >= 32 && event.getIf<sf::Event::TextEntered>()->unicode < 128) addressInput += static_cast<char>(event.getIf<sf::Event::TextEntered>()->unicode);
    }
    if (editingAddr && event.is<sf::Event::KeyPressed>() && event.getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::Enter) { navigateToAddress(); }
}
void BrowserWindow::navigateToAddress() {
    if (addressInput == "tesla-prize.local") {
        showSecretPage = true;
        if (g_Desktop) g_Desktop->addWindow(new PasswordDialog({position.x+50, position.y+50}, 320, 180, *winFont, 1));
    } else {
        showSecretPage = false;
        StoryManager::getInstance().markBrowserClueSeen();
    }
}

FileListWin::FileListWin(const std::string& winTitle, bool recycleBin, sf::Vector2f p, float w, float h, const sf::Font& font) : PuzzleWindowBase(winTitle, p, w, h, font), isRecycleBinMode(recycleBin), winFont(&font), restoreBtnText(font) {
    folderPath = recycleBin ? "回收站" : "C:\\Documents and Settings\\Elon\\My Documents";
    if (recycleBin) {
        restoreBtn.setFillColor(C::BevelMedium); restoreBtnText.setFont(font); restoreBtnText.setCharacterSize(12); restoreBtnText.setFillColor(C::TextBlack); restoreBtnText.setString(U8("还原选定项目"));
    }
}
FileListWin::~FileListWin() { for (auto f : files) delete f; files.clear(); }
void FileListWin::addFile(FileObject* file) { files.push_back(file); }
float FileListWin::listTopY() const { return clientRect().position.y + 45.f; }
void FileListWin::renderContent(sf::RenderWindow& win) {
    sf::FloatRect cr = clientRect();
    sf::RectangleShape addrBg(sf::Vector2f(cr.size.x, 35.f)); addrBg.setPosition(cr.position); addrBg.setFillColor(C::StatusBar); win.draw(addrBg);
    sf::Text addrLbl(*winFont, U8("地址(D): " + folderPath), 12u); addrLbl.setFillColor(C::TextBlack); addrLbl.setPosition(sf::Vector2f(cr.position.x + 10.f, cr.position.y + 10.f)); win.draw(addrLbl);
    if (isRecycleBinMode) {
        restoreBtn.setSize(sf::Vector2f(120.f, 22.f)); restoreBtn.setPosition(sf::Vector2f(cr.position.x + cr.size.x - 130.f, cr.position.y + 6.f)); win.draw(restoreBtn); drawDoubleBevel(win, restoreBtn.getGlobalBounds(), true);
        restoreBtnText.setPosition(sf::Vector2f(restoreBtn.getPosition().x + 15.f, restoreBtn.getPosition().y + 3.f)); win.draw(restoreBtnText);
    }
    sf::FloatRect listRect(sf::Vector2f(cr.position.x, cr.position.y + 35.f), sf::Vector2f(cr.size.x, cr.size.y - 35.f));
    sf::RectangleShape listBg(listRect.size); listBg.setPosition(listRect.position); listBg.setFillColor(C::ClientWhite); win.draw(listBg); drawDoubleBevel(win, listRect, false);
    
    float sx = listRect.position.x + 20.f; float sy = listRect.position.y + 20.f;
    for (size_t i = 0; i < files.size(); ++i) {
        if ((int)i == selectedIdx) {
            sf::RectangleShape sel(sf::Vector2f(ICON_W + 20.f, ICON_W + 40.f)); sel.setPosition(sf::Vector2f(sx - 10.f, sy - 5.f)); sel.setFillColor(C::SelectIcon); win.draw(sel);
        }
        sf::RectangleShape iconRect(sf::Vector2f(32.f, 32.f)); iconRect.setPosition(sf::Vector2f(sx + (ICON_W - 32.f)/2.f, sy));
        if (files[i]->iconType == "txt") iconRect.setFillColor(sf::Color::White);
        else if (files[i]->iconType == "locked") iconRect.setFillColor(sf::Color::Yellow);
        else iconRect.setFillColor(sf::Color::Cyan);
        win.draw(iconRect);
        
        sf::Text fn(*winFont, U8(files[i]->name), 12u);
        if ((int)i == selectedIdx) {
            fn.setFillColor(C::TextWhite); sf::FloatRect textBg = fn.getGlobalBounds();
            sf::RectangleShape tbg(sf::Vector2f(textBg.size.x + 4.f, textBg.size.y + 4.f)); tbg.setPosition(sf::Vector2f(textBg.position.x - 2.f, textBg.position.y - 2.f)); tbg.setFillColor(C::SelectBlue); win.draw(tbg);
        } else { fn.setFillColor(C::TextBlack); }
        fn.setPosition(sf::Vector2f(sx + ICON_W/2.f - fn.getLocalBounds().size.x/2.f, sy + 36.f)); win.draw(fn);
        
        sx += 100.f; if (sx + 100.f > cr.position.x + cr.size.x) { sx = listRect.position.x + 20.f; sy += 80.f; }
    }
}
void FileListWin::handleInput(const sf::Event& event) {
    if (const auto* mb = event.getIf<sf::Event::MouseButtonPressed>(); mb && mb->button == sf::Mouse::Button::Left) {
        sf::Vector2f mp(mb->position.x, mb->position.y);
        if (isRecycleBinMode && restoreBtn.getGlobalBounds().contains(mp)) { restoreSelectedFile(); return; }
        
        float sx = clientRect().position.x + 20.f; float sy = listTopY() + 20.f;
        selectedIdx = -1;
        for (size_t i = 0; i < files.size(); ++i) {
            sf::FloatRect hitBox(sf::Vector2f(sx - 10.f, sy - 5.f), sf::Vector2f(ICON_W + 20.f, ICON_W + 40.f));
            if (hitBox.contains(mp)) {
                selectedIdx = (int)i;
                if (listClickClock.getElapsedTime().asMilliseconds() < DOUBLE_CLICK_MS) openSelectedFile();
                listClickClock.restart(); break;
            }
            sx += 100.f; if (sx + 100.f > clientRect().position.x + clientRect().size.x) { sx = clientRect().position.x + 20.f; sy += 80.f; }
        }
    }
}
void FileListWin::openSelectedFile() { if (selectedIdx >= 0 && selectedIdx < (int)files.size()) files[selectedIdx]->open(); }
void FileListWin::restoreSelectedFile() {
    if (selectedIdx >= 0 && selectedIdx < (int)files.size() && files[selectedIdx]->iconType == "txt") {
        StoryManager::getInstance().onKeyPart2Restored();
        if (g_Desktop) g_Desktop->addWindow(new MsgBoxWin("回收站", "文件已还原成功！", {position.x+50, position.y+50}, 300, 150, *winFont));
        refreshFiles();
    }
}
void FileListWin::refreshFiles() {}
void FileListWin::buildClientContextMenu(std::vector<MenuItem>& out, sf::Vector2f mp) {
    if (selectedIdx >= 0 && selectedIdx < (int)files.size()) {
        out.push_back({"打开", [this](){ openSelectedFile(); }, true, false, true});
        if (isRecycleBinMode) { out.push_back({"", nullptr, false, true, false}); out.push_back({"还原", [this](){ restoreSelectedFile(); }}); }
    } else { out.push_back({"查看", nullptr, false}); out.push_back({"刷新", [this](){ refreshFiles(); }}); }
}

PasswordDialog::PasswordDialog(sf::Vector2f p, float w, float h, const sf::Font& font, int pwdMode) : PuzzleWindowBase(pwdMode == 0 ? "系统安全校验" : "Grok 认证", p, w, h, font), winFont(&font), mode(pwdMode), promptText(font), inputText(font), okText(font), cancelText(font) {
    isModal = true; canMinimize = false; canMaximize = false;
    promptText.setFont(font); promptText.setCharacterSize(14); promptText.setFillColor(C::TextBlack);
    if (mode == 0) promptText.setString(U8("请输入加密档案室访问密钥 (提示:与星舰相关)")); else promptText.setString(U8("请输入完整认证密钥 (数字组合)："));
    inputText.setFont(font); inputText.setCharacterSize(20); inputText.setFillColor(C::TextBlack);
    inputField.setFillColor(C::ClientWhite); okBtn.setFillColor(C::BevelMedium); cancelBtn.setFillColor(C::BevelMedium);
    okText.setFont(font); okText.setCharacterSize(12); okText.setFillColor(C::TextBlack); okText.setString(U8("确定"));
    cancelText.setFont(font); cancelText.setCharacterSize(12); cancelText.setFillColor(C::TextBlack); cancelText.setString(U8("取消"));
}
void PasswordDialog::renderContent(sf::RenderWindow& win) {
    sf::FloatRect cr = clientRect();
    promptText.setPosition(sf::Vector2f(cr.position.x + 20.f, cr.position.y + 20.f)); win.draw(promptText);
    inputField.setPosition(sf::Vector2f(cr.position.x + 20.f, cr.position.y + 50.f)); inputField.setSize(sf::Vector2f(cr.size.x - 40.f, 28.f)); win.draw(inputField); drawDoubleBevel(win, inputField.getGlobalBounds(), false);
    inputText.setString(hasError ? U8("密码错误！") : U8(typedPassword + ((int)(sf::Clock().getElapsedTime().asSeconds()*2)%2==0 ? "|" : "")));
    if (hasError) inputText.setFillColor(sf::Color::Red); else inputText.setFillColor(C::TextBlack);
    inputText.setPosition(sf::Vector2f(inputField.getPosition().x + 5.f, inputField.getPosition().y + 2.f)); win.draw(inputText);
    okBtn.setPosition(sf::Vector2f(cr.position.x + cr.size.x - 180.f, cr.position.y + cr.size.y - 40.f)); okBtn.setSize(sf::Vector2f(70.f, 24.f)); win.draw(okBtn); drawDoubleBevel(win, okBtn.getGlobalBounds(), true);
    okText.setPosition(sf::Vector2f(okBtn.getPosition().x + 22.f, okBtn.getPosition().y + 4.f)); win.draw(okText);
    cancelBtn.setPosition(sf::Vector2f(cr.position.x + cr.size.x - 90.f, cr.position.y + cr.size.y - 40.f)); cancelBtn.setSize(sf::Vector2f(70.f, 24.f)); win.draw(cancelBtn); drawDoubleBevel(win, cancelBtn.getGlobalBounds(), true);
    cancelText.setPosition(sf::Vector2f(cancelBtn.getPosition().x + 22.f, cancelBtn.getPosition().y + 4.f)); win.draw(cancelText);
}
void PasswordDialog::handleInput(const sf::Event& event) {
    if (const auto* mb = event.getIf<sf::Event::MouseButtonPressed>(); mb && mb->button == sf::Mouse::Button::Left) {
        sf::Vector2f mp(mb->position.x, mb->position.y);
        if (hasError) { hasError = false; typedPassword = ""; }
        if (okBtn.getGlobalBounds().contains(mp)) submitPassword();
        if (cancelBtn.getGlobalBounds().contains(mp)) closeWindow();
    }
    if (event.is<sf::Event::TextEntered>()) {
        if (hasError) { hasError = false; typedPassword = ""; }
        if (event.getIf<sf::Event::TextEntered>()->unicode == 8 && !typedPassword.empty()) typedPassword.pop_back();
        else if (event.getIf<sf::Event::TextEntered>()->unicode >= 32 && event.getIf<sf::Event::TextEntered>()->unicode < 128 && typedPassword.size() < 20) typedPassword += static_cast<char>(event.getIf<sf::Event::TextEntered>()->unicode);
    }
    if (event.is<sf::Event::KeyPressed>() && event.getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::Enter) submitPassword();
}
void PasswordDialog::submitPassword() {
    if (mode == 0) {
        if (StoryManager::getInstance().tryUnlockArchive(typedPassword)) {
            closeWindow();
            if (g_Desktop) g_Desktop->addWindow(new MsgBoxWin("系统提示", "档案室已解锁。\n请前往桌面查看机密档案。", {position.x, position.y}, 300, 150, *winFont));
        } else { hasError = true; }
    } else {
        if (StoryManager::getInstance().tryPerfectKey(typedPassword)) { closeWindow(); } else { hasError = true; }
    }
}

SystemPropWin::SystemPropWin(sf::Vector2f p, float w, float h, const sf::Font& font) : PuzzleWindowBase("系统属性", p, w, h, font), winFont(&font), secretBtnText(font), okText(font), cancelText(font) {
    canMaximize = false; secretBtn.setFillColor(C::BevelMedium); okBtn.setFillColor(C::BevelMedium); cancelBtn.setFillColor(C::BevelMedium);
    secretBtnText.setFont(font); secretBtnText.setCharacterSize(12); secretBtnText.setFillColor(C::TextBlack); secretBtnText.setString(U8("加密档案室..."));
    okText.setFont(font); okText.setCharacterSize(12); okText.setFillColor(C::TextBlack); okText.setString(U8("确定"));
    cancelText.setFont(font); cancelText.setCharacterSize(12); cancelText.setFillColor(C::TextBlack); cancelText.setString(U8("取消"));
}
void SystemPropWin::renderContent(sf::RenderWindow& win) {
    sf::FloatRect cr = clientRect();
    sf::Text info(*winFont, U8("系统:\nMicrosoft Windows XP\nProfessional\n版本 2008\n\n注册到:\nElon Musk\nSpaceX"), 12u); info.setFillColor(C::TextBlack); info.setPosition(sf::Vector2f(cr.position.x + 20.f, cr.position.y + 20.f)); win.draw(info);
    secretBtn.setPosition(sf::Vector2f(cr.position.x + 20.f, cr.position.y + cr.size.y - 80.f)); secretBtn.setSize(sf::Vector2f(100.f, 24.f)); win.draw(secretBtn); drawDoubleBevel(win, secretBtn.getGlobalBounds(), true); secretBtnText.setPosition(sf::Vector2f(secretBtn.getPosition().x + 10.f, secretBtn.getPosition().y + 4.f)); win.draw(secretBtnText);
    okBtn.setPosition(sf::Vector2f(cr.position.x + cr.size.x - 180.f, cr.position.y + cr.size.y - 40.f)); okBtn.setSize(sf::Vector2f(70.f, 24.f)); win.draw(okBtn); drawDoubleBevel(win, okBtn.getGlobalBounds(), true); okText.setPosition(sf::Vector2f(okBtn.getPosition().x + 22.f, okBtn.getPosition().y + 4.f)); win.draw(okText);
    cancelBtn.setPosition(sf::Vector2f(cr.position.x + cr.size.x - 90.f, cr.position.y + cr.size.y - 40.f)); cancelBtn.setSize(sf::Vector2f(70.f, 24.f)); win.draw(cancelBtn); drawDoubleBevel(win, cancelBtn.getGlobalBounds(), true); cancelText.setPosition(sf::Vector2f(cancelBtn.getPosition().x + 22.f, cancelBtn.getPosition().y + 4.f)); win.draw(cancelText);
}
void SystemPropWin::handleInput(const sf::Event& event) {
    if (const auto* mb = event.getIf<sf::Event::MouseButtonPressed>(); mb && mb->button == sf::Mouse::Button::Left) {
        sf::Vector2f mp(mb->position.x, mb->position.y);
        if (secretBtn.getGlobalBounds().contains(mp)) if (g_Desktop) g_Desktop->addWindow(new PasswordDialog({position.x + 50, position.y + 50}, 320, 180, *winFont, 0));
        if (okBtn.getGlobalBounds().contains(mp) || cancelBtn.getGlobalBounds().contains(mp)) closeWindow();
    }
}

RewardAnimWindow::RewardAnimWindow(sf::Vector2f p, float w, float h, const sf::Font& font) : PuzzleWindowBase("系统奖励", p, w, h, font), congratsText(font) {
    congratsText.setFont(font); congratsText.setCharacterSize(18); congratsText.setFillColor(C::TextBlack); congratsText.setStyle(sf::Text::Bold);
    if (StoryManager::getInstance().hasPerfectClear) congratsText.setString(U8("★ 完美结局 ★\n密码校验成功！\n已解锁：无限制 Grok 永久使用权\n已解锁：特斯拉 Model S 兑换资格！"));
    else congratsText.setString(U8("☆ 普通结局 ☆\n基础剧情通关。\n已解锁：无限制 Grok 永久使用权\n(提示：似乎还有隐藏页面未探索)"));
}
void RewardAnimWindow::renderContent(sf::RenderWindow& win) {
    sf::FloatRect cr = clientRect(); sf::RectangleShape bg(cr.size); bg.setPosition(cr.position); bg.setFillColor(C::ClientWhite); win.draw(bg); drawDoubleBevel(win, cr, false);
    if ((int)(animationClock.getElapsedTime().asSeconds() * 2) % 2 == 0) congratsText.setFillColor(StoryManager::getInstance().hasPerfectClear ? sf::Color(200, 100, 0) : sf::Color::Blue);
    else congratsText.setFillColor(C::TextBlack);
    congratsText.setPosition(sf::Vector2f(cr.position.x + 20.f, cr.position.y + 50.f)); win.draw(congratsText);
}
void RewardAnimWindow::handleInput(const sf::Event& event) {}

MsgBoxWin::MsgBoxWin(const std::string& boxTitle, const std::string& message, sf::Vector2f p, float w, float h, const sf::Font& font) : PuzzleWindowBase(boxTitle, p, w, h, font), msgText(font), okBtnText(font) {
    isModal = true; canMaximize = false; canMinimize = false; msgText.setFont(font); msgText.setCharacterSize(14); msgText.setFillColor(C::TextBlack); msgText.setString(U8(message));
    okBtn.setFillColor(C::BevelMedium); okBtnText.setFont(font); okBtnText.setCharacterSize(12); okBtnText.setFillColor(C::TextBlack); okBtnText.setString(U8("确定"));
}
void MsgBoxWin::renderContent(sf::RenderWindow& win) {
    sf::FloatRect cr = clientRect(); msgText.setPosition(sf::Vector2f(cr.position.x + 20.f, cr.position.y + 20.f)); win.draw(msgText);
    okBtn.setPosition(sf::Vector2f(cr.position.x + cr.size.x/2.f - 35.f, cr.position.y + cr.size.y - 40.f)); okBtn.setSize(sf::Vector2f(70.f, 24.f)); win.draw(okBtn); drawDoubleBevel(win, okBtn.getGlobalBounds(), true);
    okBtnText.setPosition(sf::Vector2f(okBtn.getPosition().x + 22.f, okBtn.getPosition().y + 4.f)); win.draw(okBtnText);
}
void MsgBoxWin::handleInput(const sf::Event& event) {
    if (const auto* mb = event.getIf<sf::Event::MouseButtonPressed>(); mb && mb->button == sf::Mouse::Button::Left) {
        if (okBtn.getGlobalBounds().contains(sf::Vector2f(mb->position.x, mb->position.y))) closeWindow();
    }
}

// ═══════════════════════════════════════════════════════════════
// 5. AppIconBase 体系实现
// ═══════════════════════════════════════════════════════════════
AppIconBase::AppIconBase(sf::Vector2f p, const std::string& lbl, const std::string& texPath, const sf::Font& font) : UIComponent(p, ICON_W, ICON_CELL_H), label(lbl), labelText(font), iconSpr(iconTex) {
    sf::Image img;
    if (img.loadFromFile(texPath)) {
        
        (void)iconTex.loadFromImage(img); iconSpr.setTexture(iconTex, true);
        iconSpr.setScale(sf::Vector2f(ICON_W / (float)iconTex.getSize().x, ICON_H / (float)iconTex.getSize().y));
    }
    labelText.setFont(font); labelText.setCharacterSize(20); labelText.setString(U8(label)); labelText.setFillColor(C::TextWhite);
}
void AppIconBase::render(sf::RenderWindow& win) {
    iconSpr.setPosition(sf::Vector2f(position.x + (width - ICON_W)/2.f, position.y + 4.f));
    if (selected || highlight) {
        sf::RectangleShape filter(sf::Vector2f(ICON_W, ICON_H)); filter.setPosition(iconSpr.getPosition()); filter.setFillColor(highlight ? sf::Color(255, 255, 0, 100) : C::SelectIcon); win.draw(filter);
    }
    win.draw(iconSpr);
    sf::FloatRect tr = labelText.getLocalBounds(); float tx = position.x + width/2 - tr.size.x/2; float ty = position.y + ICON_H + 8.f;
    if (selected) {
        sf::RectangleShape tbg(sf::Vector2f(tr.size.x + 6.f, tr.size.y + 6.f)); tbg.setPosition(sf::Vector2f(tx - 3.f, ty - 3.f)); tbg.setFillColor(C::SelectBlue); win.draw(tbg);
    } else {
        sf::Text ts = labelText; ts.setFillColor(sf::Color(0,0,0,150)); ts.setPosition(sf::Vector2f(tx + 1.f, ty + 1.f)); win.draw(ts);
    }
    labelText.setPosition(sf::Vector2f(tx, ty)); win.draw(labelText);
}
bool AppIconBase::isMouseHit(sf::Vector2f mp) { return bounds().contains(mp); }
void AppIconBase::buildContextMenu(std::vector<MenuItem>& out) {
    out.push_back({"打开(O)", [this](){ onDoubleClick(); }, true, false, true}); out.push_back({"属性(R)", nullptr, false});
}

MyDocumentsIcon::MyDocumentsIcon(sf::Vector2f p, const std::string& texPath, const sf::Font& font) : AppIconBase(p, "我的文档", texPath, font) {}
void MyDocumentsIcon::onDoubleClick() {
    if (g_Desktop) {
        FileListWin* win = new FileListWin("我的文档", false, {100, 100}, 500, 400, g_Desktop->systemFont);
        win->addFile(new TextFile("项目备忘录.txt", "2008年 SpaceX 内部淘汰机密\n\n星舰首飞记录：20230420\n...\n(密钥分为两段，后半段已被删除)"));
        win->addFile(new TextFile("加密提示.doc", "加密档案室的入口... 好像在系统属性里？"));
        g_Desktop->addWindow(win); StoryManager::getInstance().markDocClueRead();
    }
}
void MyDocumentsIcon::buildContextMenu(std::vector<MenuItem>& out) {
    AppIconBase::buildContextMenu(out); out.insert(out.begin() + 1, {"", nullptr, false, true}); out.insert(out.begin() + 2, {"提示:星舰首飞藏在网页，密码不止一组数字", nullptr, false});
}

IEBrowserIcon::IEBrowserIcon(sf::Vector2f p, const std::string& texPath, const sf::Font& font) : AppIconBase(p, "Internet Explorer", texPath, font) {}
void IEBrowserIcon::onDoubleClick() { if (g_Desktop) g_Desktop->addWindow(new BrowserWindow({150, 150}, 600, 450, g_Desktop->systemFont)); }

RecycleBinIcon::RecycleBinIcon(sf::Vector2f p, const std::string& texPath, const sf::Font& font) : AppIconBase(p, "回收站", texPath, font) {}
void RecycleBinIcon::onDoubleClick() {
    if (g_Desktop) {
        if (!StoryManager::getInstance().recycleBinUnlocked) {
            g_Desktop->addWindow(new MsgBoxWin("拒绝访问", "该文件夹已被加密系统锁定，禁止访问。\n请先通过【加密档案室】验证权限。", {200, 200}, 300, 150, g_Desktop->systemFont));
            return;
        }
        FileListWin* win = new FileListWin("回收站", true, {200, 200}, 500, 400, g_Desktop->systemFont);
        win->addFile(new DeletedFile("特斯拉编号记录.txt", "车辆出厂编号：2012"));
        g_Desktop->addWindow(win);
    }
}

SecretFolderIcon::SecretFolderIcon(sf::Vector2f p, const std::string& texPath, const sf::Font& font) : AppIconBase(p, "加密档案室", texPath, font) {}
void SecretFolderIcon::onDoubleClick() {
    if (g_Desktop) {
        if (!StoryManager::getInstance().archiveOpened) return;
        FileListWin* win = new FileListWin("加密档案室", false, {250, 250}, 400, 300, g_Desktop->systemFont);
        win->addFile(new TextFile("密钥上半段.txt", "完整密钥 = 试飞日期 + 特斯拉 Model S 出厂编号\n后半段密钥已被移入回收站永久删除。"));
        g_Desktop->addWindow(win); StoryManager::getInstance().unlockRecycleBin(); highlight = false;
    }
}

TeslaExeIcon::TeslaExeIcon(sf::Vector2f p, const std::string& texPath, const sf::Font& font) : AppIconBase(p, "Starship.exe", texPath, font) {}
void TeslaExeIcon::onDoubleClick() { if (g_Desktop) StoryManager::getInstance().triggerNormalEnding(); }

MyComputerIcon::MyComputerIcon(sf::Vector2f p, const std::string& texPath, const sf::Font& font) : AppIconBase(p, "我的电脑", texPath, font) {}
void MyComputerIcon::onDoubleClick() {
    if (g_Desktop) {
        FileListWin* win = new FileListWin("我的电脑", false, {50, 50}, 500, 400, g_Desktop->systemFont); win->folderPath = "我的电脑";
        win->addFile(new TextFile("C盘", "本地磁盘", "locked")); win->addFile(new TextFile("D盘", "本地磁盘", "locked"));
        g_Desktop->addWindow(win);
    }
}
void MyComputerIcon::buildContextMenu(std::vector<MenuItem>& out) {
    AppIconBase::buildContextMenu(out); out.back() = {"属性(R)", [this](){ if (g_Desktop) g_Desktop->addWindow(new SystemPropWin({100, 100}, 400, 450, g_Desktop->systemFont)); }, true};
}

// ═══════════════════════════════════════════════════════════════
// 6. Desktop 实现
// ═══════════════════════════════════════════════════════════════

// ────────────────────────────────────────────────────────────
// Desktop grid helpers
// ────────────────────────────────────────────────────────────
sf::Vector2f Desktop::slotPosition(int slot) const {
    int row = slot / GRID_COLS;
    int col = slot % GRID_COLS;
    return sf::Vector2f(GRID_MARGIN_X + col * GRID_COL_W,
                        GRID_MARGIN_Y + row * GRID_ROW_H);
}

int Desktop::allocateSlot() {
    for (int s = 0; ; ++s) {
        if (usedSlots.find(s) == usedSlots.end()) {
            usedSlots.insert(s);
            return s;
        }
    }
}

int Desktop::nearestFreeSlot(sf::Vector2f pos) const {
    // How many slots fit vertically
    int maxRows = (int)((WIN_H - TASKBAR_H - GRID_MARGIN_Y) / GRID_ROW_H);
    // Find nearest slot by distance
    int best = -1;
    float bestDist = 1e9f;
    for (int s = 0; s < GRID_COLS * maxRows; ++s) {
        if (usedSlots.find(s) != usedSlots.end()) continue;
        int row = s / GRID_COLS;
        int col = s % GRID_COLS;
        sf::Vector2f sp(GRID_MARGIN_X + col * GRID_COL_W,
                        GRID_MARGIN_Y + row * GRID_ROW_H);
        float d = std::hypot(sp.x - pos.x, sp.y - pos.y);
        if (d < bestDist) { bestDist = d; best = s; }
    }
    return best;
}

void Desktop::saveIconPositions() const {
    std::ofstream f("icon_positions.ini");
    if (!f.is_open()) return;
    for (auto* icon : icons) {
        if (!icon->iconId.empty())
            f << icon->iconId << "=" << icon->gridSlot << "\n";
    }
}

void Desktop::loadIconPositions() {
    std::ifstream f("icon_positions.ini");
    if (!f.is_open()) return;
    std::string line;
    while (std::getline(f, line)) {
        auto eq = line.find('=');
        if (eq == std::string::npos) continue;
        std::string id  = line.substr(0, eq);
        int slot = std::stoi(line.substr(eq + 1));
        for (auto* icon : icons) {
            if (icon->iconId == id) {
                // Free old slot, assign new
                if (icon->gridSlot >= 0) usedSlots.erase(icon->gridSlot);
                icon->gridSlot = slot;
                usedSlots.insert(slot);
                icon->position = slotPosition(slot);
                break;
            }
        }
    }
}

Desktop::Desktop() : startSpr(startTex), wallSpr(wallTex) { g_Desktop = this; }
Desktop::~Desktop() { for (auto i : icons) delete i; for (auto w : openWindows) delete w; g_Desktop = nullptr; }
void Desktop::init(sf::RenderWindow& win) {
    renderWin = &win;
    if (!systemFont.openFromFile("C:\\Windows\\Fonts\\simsun.ttc") && !systemFont.openFromFile("C:\\Windows\\Fonts\\msyh.ttc")) {}
    shellMenu.setFont(systemFont);
    
    sf::Image wImg;
    if (wImg.loadFromFile("assets/wallpaper.jpg")) {
        (void)wallTex.loadFromImage(wImg); wallTex.setSmooth(true); wallSpr.setTexture(wallTex, true);
        float sx = WIN_W / wImg.getSize().x; float sy = DESKTOP_H / wImg.getSize().y; wallSpr.setScale(sf::Vector2f(std::max(sx, sy), std::max(sx, sy))); hasWallpaper = true;
    }
    
    sf::Image sImg;
    if (sImg.loadFromFile("assets/start.png")) {
        
        (void)startTex.loadFromImage(sImg); startSpr.setTexture(startTex, true);
        startSpr.setPosition(sf::Vector2f(0.f, WIN_H - TASKBAR_H)); startSpr.setScale(sf::Vector2f(TASKBAR_H / (float)startTex.getSize().y, TASKBAR_H / (float)startTex.getSize().y));
    }
    
    taskbarBg.setSize(sf::Vector2f(WIN_W, TASKBAR_H)); taskbarBg.setPosition(sf::Vector2f(0.f, WIN_H - TASKBAR_H)); taskbarBg.setFillColor(C::TaskbarBg);
    clockText.emplace(systemFont); clockText->setCharacterSize(18); clockText->setFillColor(C::TextWhite); clockText->setPosition(sf::Vector2f(WIN_W - 60.f, WIN_H - TASKBAR_H + 12.f));

    // Helper lambda: assign a grid slot to a newly created icon
    auto assignSlot = [&](AppIconBase* icon, const std::string& id) {
        icon->iconId  = id;
        icon->gridSlot = allocateSlot();
        icon->position = slotPosition(icon->gridSlot);
    };

    auto* myDocs = new MyDocumentsIcon({0,0}, "assets/folder.png", systemFont);
    assignSlot(myDocs, "MyDocuments"); icons.push_back(myDocs);

    auto* myComp = new MyComputerIcon({0,0}, "assets/computer.png", systemFont);
    assignSlot(myComp, "MyComputer"); icons.push_back(myComp);

    auto* ie = new IEBrowserIcon({0,0}, "assets/ie.png", systemFont);
    assignSlot(ie, "IEBrowser"); icons.push_back(ie);

    recycleIcon = new RecycleBinIcon({0,0}, "assets/recycle.png", systemFont);
    assignSlot(recycleIcon, "RecycleBin"); icons.push_back(recycleIcon);

    secretIcon = new SecretFolderIcon({0,0}, "assets/folder.png", systemFont);
    assignSlot(secretIcon, "SecretFolder"); icons.push_back(secretIcon);

    rewardIcon = new TeslaExeIcon({0,0}, "assets/start.png", systemFont);
    assignSlot(rewardIcon, "TeslaExe"); rewardIcon->visible = false; icons.push_back(rewardIcon);

    // Load saved positions (overrides default grid assignments)
    loadIconPositions();
}
void Desktop::addWindow(PuzzleWindowBase* win) { openWindows.push_back(win); setActiveWindow(win); }
void Desktop::bringToFront(PuzzleWindowBase* win) {
    auto it = std::find(openWindows.begin(), openWindows.end(), win);
    if (it != openWindows.end()) { openWindows.erase(it); openWindows.push_back(win); }
}
void Desktop::setActiveWindow(PuzzleWindowBase* win) {
    for (auto w : openWindows) w->isActive = false;
    if (win) { win->isActive = true; bringToFront(win); }
}
PuzzleWindowBase* Desktop::topWindow() const {
    for (auto it = openWindows.rbegin(); it != openWindows.rend(); ++it) if ((*it)->isOpen && !(*it)->isMinimized) return *it;
    return nullptr;
}
PuzzleWindowBase* Desktop::findWindowByTitle(const std::string& title) const {
    for (auto w : openWindows) if (w->title.find(title) != std::string::npos) return w;
    return nullptr;
}
bool Desktop::activateExisting(const std::string& title) {
    PuzzleWindowBase* w = findWindowByTitle(title);
    if (w) { w->restoreFromTaskbar(); return true; }
    return false;
}
void Desktop::syncStoryVisuals() { if (StoryManager::getInstance().isRewardUnlocked && rewardIcon) rewardIcon->visible = true; }
void Desktop::hideShellMenu() { shellMenu.hide(); }
std::vector<PuzzleWindowBase*> Desktop::visibleTaskWindows() const {
    std::vector<PuzzleWindowBase*> res; for (auto w : openWindows) if (w->isOpen) res.push_back(w); return res;
}
sf::FloatRect Desktop::taskButtonRect(std::size_t index) const {
    float startX = 100.f; float w = TASK_BTN_W; return sf::FloatRect(sf::Vector2f(startX + index * (w + 4.f), WIN_H - TASKBAR_H + 4.f), sf::Vector2f(w, TASKBAR_H - 8.f));
}
void Desktop::handleTaskbarClick(sf::Vector2f mp) {
    if (startSpr.getGlobalBounds().contains(mp)) {
        std::vector<MenuItem> items;
        items.push_back({"所有程序(P)", nullptr, true}); items.push_back({"", nullptr, false, true}); items.push_back({"注销(L)", nullptr, true}); items.push_back({"关闭计算机(U)", [this](){ renderWin->close(); }, true});
        shellMenu.show({0, WIN_H - TASKBAR_H - items.size()*22.f - 6.f}, items); return;
    }
    auto vws = visibleTaskWindows();
    for (std::size_t i = 0; i < vws.size(); ++i) {
        if (taskButtonRect(i).contains(mp)) {
            if (vws[i]->isActive && !vws[i]->isMinimized) vws[i]->minimizeWindow(); else vws[i]->restoreFromTaskbar();
            return;
        }
    }
}
void Desktop::processEvents() {
    while (std::optional<sf::Event> eventOpt = renderWin->pollEvent()) {
        const sf::Event& event = *eventOpt;
        if (event.is<sf::Event::Closed>()) renderWin->close();
        sf::Vector2f mp;
        if (event.is<sf::Event::MouseButtonPressed>() || event.is<sf::Event::MouseButtonReleased>() || event.is<sf::Event::MouseMoved>()) mp = sf::Vector2f(sf::Mouse::getPosition(*renderWin));
        if (event.is<sf::Event::MouseMoved>()) {
            shellMenu.updateHover(mp);
            for (auto w : openWindows) {
                if (w->isDragging) w->dragUpdate(mp);
                if (w->isResizing) {
                    if (w->resizeDir == 1 || w->resizeDir == 3) w->width = std::max(200.f, mp.x - w->position.x);
                    if (w->resizeDir == 2 || w->resizeDir == 3) w->height = std::max(150.f, mp.y - w->position.y);
                }
            }
            // Icon drag
            if (draggingIcon) {
                sf::Vector2f newPos = mp - iconDragOffset;
                newPos.x = std::max(0.f, std::min(newPos.x, (float)WIN_W - draggingIcon->width));
                newPos.y = std::max(0.f, std::min(newPos.y, (float)(WIN_H - TASKBAR_H) - draggingIcon->height));
                draggingIcon->position = newPos;
                iconDragMoved = true;
            }
        }
        if (const auto* mbRelease = event.getIf<sf::Event::MouseButtonReleased>()) {
            if (mbRelease->button == sf::Mouse::Button::Left) {
                for (auto w : openWindows) { w->isDragging = false; w->isResizing = false; }
                if (draggingIcon) {
                    if (!iconDragMoved) {
                        // Stationary release: single-click or double-click
                        AppIconBase* released = draggingIcon;
                        draggingIcon->highlight = false;
                        draggingIcon = nullptr;
                        iconDragMoved = false;
                        if (lastClickedIcon == released && clickClock.getElapsedTime().asMilliseconds() < DOUBLE_CLICK_MS) {
                            released->onDoubleClick();
                        }
                    } else {
                        // Snap to nearest free slot
                        AppIconBase* icon = draggingIcon;
                        int oldSlot = icon->gridSlot;
                        // Temporarily free old slot to allow snapping back to it
                        if (oldSlot >= 0) usedSlots.erase(oldSlot);
                        int newSlot = nearestFreeSlot(icon->position);
                        if (newSlot < 0) newSlot = oldSlot; // fallback
                        usedSlots.insert(newSlot);
                        icon->gridSlot = newSlot;
                        icon->position = slotPosition(newSlot);
                        icon->highlight = false;
                        draggingIcon = nullptr;
                        iconDragMoved = false;
                        saveIconPositions();
                    }
                }
            }
        }
        if (const auto* mb = event.getIf<sf::Event::MouseButtonPressed>()) {
            mp = sf::Vector2f(mb->position.x, mb->position.y);
            if (shellMenu.open) { if (shellMenu.tryClick(mp)) continue; hideShellMenu(); }
            if (mp.y >= WIN_H - TASKBAR_H) { if (mb->button == sf::Mouse::Button::Left) handleTaskbarClick(mp); continue; }
            bool hitWindow = false;
            for (auto it = openWindows.rbegin(); it != openWindows.rend(); ++it) {
                PuzzleWindowBase* w = *it; if (!w->isOpen || w->isMinimized) continue;
                PuzzleWindowBase* topModal = nullptr;
                for (auto w2 : openWindows) if (w2->isOpen && !w2->isMinimized && w2->isModal) topModal = w2;
                if (topModal && topModal != w) continue;
                if (w->isMouseHit(mp)) {
                    hitWindow = true; setActiveWindow(w);
                    if (mb->button == sf::Mouse::Button::Left) {
                        int capHit = w->hitCaptionButton(mp);
                        if (capHit == 0) w->closeWindow(); else if (capHit == 1) w->toggleMaximize(); else if (capHit == 2) w->minimizeWindow();
                        else if (w->containsTitleBarDrag(mp)) { w->isDragging = true; w->dragOffset = mp - w->position; }
                        else if (!w->isMaximized && mp.x > w->position.x + w->width - 8.f && mp.y > w->position.y + w->height - 8.f) { w->isResizing = true; w->resizeDir = 3; }
                        else if (!w->isMaximized && mp.x > w->position.x + w->width - 8.f) { w->isResizing = true; w->resizeDir = 1; }
                        else if (!w->isMaximized && mp.y > w->position.y + w->height - 8.f) { w->isResizing = true; w->resizeDir = 2; }
                    } else if (mb->button == sf::Mouse::Button::Right) {
                        if (w->clientRect().contains(mp)) { std::vector<MenuItem> items; w->buildClientContextMenu(items, mp); if (!items.empty()) shellMenu.show(mp, items); }
                    }
                    w->handleInput(event); break;
                }
            }
            if (!hitWindow) {
                PuzzleWindowBase* topModal = nullptr; for (auto w2 : openWindows) if (w2->isOpen && !w2->isMinimized && w2->isModal) topModal = w2;
                if (!topModal) {
                    for (auto w : openWindows) w->isActive = false;
                    bool hitIcon = false;
                    for (auto icon : icons) {
                        if (!icon->visible) continue;
                        if (icon->isMouseHit(mp)) {
                            hitIcon = true;
                            if (mb->button == sf::Mouse::Button::Left) {
                                for (auto i2 : icons) i2->selected = false; icon->selected = true;
                                // Initiate drag; double-click handled on release if no movement
                                draggingIcon   = icon;
                                iconDragOffset = mp - icon->position;
                                iconDragMoved  = false;
                                icon->highlight = true;
                                lastClickedIcon = icon; clickClock.restart();
                            } else if (mb->button == sf::Mouse::Button::Right) {
                                for (auto i2 : icons) i2->selected = false; icon->selected = true;
                                std::vector<MenuItem> items; icon->buildContextMenu(items); if (!items.empty()) shellMenu.show(mp, items);
                            }
                            break;
                        }
                    }
                    if (!hitIcon) {
                        for (auto i2 : icons) i2->selected = false;
                        if (mb->button == sf::Mouse::Button::Right) {
                            std::vector<MenuItem> items;
                            items.push_back({"查看(I)", nullptr, false}); items.push_back({"刷新(E)", [this]() {
                                renderWin->clear(C::DesktopBg);
                                if (hasWallpaper) renderWin->draw(wallSpr);
                                renderTaskbar();
                                renderWin->display();
                                sf::sleep(sf::milliseconds(100));
                            }, true}); items.push_back({"", nullptr, false, true}); items.push_back({"属性(R)", nullptr, true});
                            shellMenu.show(mp, items);
                        }
                    }
                }
            }
        }
        PuzzleWindowBase* active = topWindow();
        if (active && (event.is<sf::Event::TextEntered>() || event.is<sf::Event::KeyPressed>())) active->handleInput(event);
    }
}
void Desktop::updateTaskbar() {
    time_t now = time(nullptr); struct tm* t = localtime(&now); char buf[64]; strftime(buf, sizeof(buf), "%H:%M", t); clockText->setString(buf);
}
void Desktop::renderTaskbar() {
    renderWin->draw(taskbarBg);
    sf::RectangleShape tline(sf::Vector2f(WIN_W, 1.f)); tline.setPosition(sf::Vector2f(0.f, WIN_H - TASKBAR_H)); tline.setFillColor(sf::Color(255, 255, 255, 100)); renderWin->draw(tline);
    renderWin->draw(startSpr); if (startLabel) renderWin->draw(*startLabel);
    auto vws = visibleTaskWindows();
    for (std::size_t i = 0; i < vws.size(); ++i) {
        sf::FloatRect br = taskButtonRect(i); sf::RectangleShape btn(br.size); btn.setPosition(br.position);
        btn.setFillColor(vws[i]->isActive && !vws[i]->isMinimized ? C::TaskbarBtnAct : C::TaskbarBtn); renderWin->draw(btn);
        sf::RectangleShape bline(sf::Vector2f(br.size.x, 1.f)); bline.setPosition(br.position); bline.setFillColor(sf::Color(255,255,255,50)); renderWin->draw(bline);
        bline.setPosition(sf::Vector2f(br.position.x, br.position.y+br.size.y-1.f)); bline.setFillColor(sf::Color(0,0,0,100)); renderWin->draw(bline);
        sf::Text t = vws[i]->titleText; t.setPosition(sf::Vector2f(br.position.x + 10.f, br.position.y + 4.f));
        std::string s = t.getString().toAnsiString(); if (s.length() > 15) { s = s.substr(0, 12) + "..."; t.setString(U8(s)); }
        renderWin->draw(t);
    }
    if (clockText) renderWin->draw(*clockText);
}
void Desktop::renderAll() {
    renderWin->clear(C::DesktopBg); if (hasWallpaper) renderWin->draw(wallSpr);
    for (auto icon : icons) if (icon->visible) icon->render(*renderWin);
    for (auto w : openWindows) w->render(*renderWin);
    renderTaskbar(); shellMenu.render(*renderWin); renderWin->display();
}
void Desktop::run() {
    while (renderWin->isOpen()) {
        processEvents();
        auto it = std::remove_if(openWindows.begin(), openWindows.end(), [](PuzzleWindowBase* w){ return !w->isOpen; });
        for (auto i = it; i != openWindows.end(); ++i) delete *i;
        openWindows.erase(it, openWindows.end());
        updateTaskbar(); renderAll();
    }
}
