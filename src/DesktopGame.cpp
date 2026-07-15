#include "DesktopGame.hpp"
#include <iostream>
#include <cstring>
#include <sstream>
#include <cstdint>

// U8 string helper for MSVC
inline sf::String U8(const char* s) { return sf::String::fromUtf8(s, s + std::strlen(s)); }
inline sf::String U8(const std::string& s) { return sf::String::fromUtf8(s.data(), s.data() + s.size()); }

Desktop* g_Desktop = nullptr;

// ── Sfx (procedural UI sounds) ─────────────────────────────────
Sfx& Sfx::get() {
    static Sfx instance;
    return instance;
}

sf::SoundBuffer Sfx::makeBeep(float freqHz, float durationSec, float volume, bool decay) {
    constexpr unsigned rate = 44100;
    const auto n = static_cast<std::size_t>(rate * durationSec);
    std::vector<std::int16_t> samples(n > 0 ? n : 1);
    constexpr float twoPi = 6.28318530718f;
    for (std::size_t i = 0; i < samples.size(); ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(rate);
        float env = 1.f;
        if (decay) {
            env = 1.f - t / durationSec;
            if (env < 0.f) env = 0.f;
            // short attack to avoid click
            const float attack = 0.008f;
            if (t < attack) env *= t / attack;
        }
        const float s = std::sin(twoPi * freqHz * t) * env * volume;
        samples[i] = static_cast<std::int16_t>(s * 32767.f);
    }
    sf::SoundBuffer buf;
    const std::vector<sf::SoundChannel> mono{sf::SoundChannel::Mono};
    (void)buf.loadFromSamples(samples.data(),
                              static_cast<std::uint64_t>(samples.size()),
                              1u, rate, mono);
    return buf;
}

sf::SoundBuffer Sfx::makeChord(std::initializer_list<float> freqs, float durationSec, float volume) {
    constexpr unsigned rate = 44100;
    const auto n = static_cast<std::size_t>(rate * durationSec);
    std::vector<std::int16_t> samples(n > 0 ? n : 1);
    constexpr float twoPi = 6.28318530718f;
    const float inv = 1.f / static_cast<float>(freqs.size() > 0 ? freqs.size() : 1);
    for (std::size_t i = 0; i < samples.size(); ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(rate);
        float env = 1.f - t / durationSec;
        if (env < 0.f) env = 0.f;
        const float attack = 0.01f;
        if (t < attack) env *= t / attack;
        float mix = 0.f;
        for (float f : freqs) mix += std::sin(twoPi * f * t);
        mix *= inv * env * volume;
        if (mix > 1.f) mix = 1.f;
        if (mix < -1.f) mix = -1.f;
        samples[i] = static_cast<std::int16_t>(mix * 32767.f);
    }
    sf::SoundBuffer buf;
    const std::vector<sf::SoundChannel> mono{sf::SoundChannel::Mono};
    (void)buf.loadFromSamples(samples.data(),
                              static_cast<std::uint64_t>(samples.size()),
                              1u, rate, mono);
    return buf;
}

void Sfx::init() {
    if (ready) return;
    using I = Id;
    buffers[static_cast<std::size_t>(I::Click)]   = makeBeep(900.f,  0.04f, 0.28f);
    buffers[static_cast<std::size_t>(I::Open)]    = makeBeep(620.f,  0.08f, 0.35f);
    buffers[static_cast<std::size_t>(I::Menu)]    = makeBeep(750.f,  0.05f, 0.30f);
    buffers[static_cast<std::size_t>(I::Error)]   = makeChord({220.f, 180.f}, 0.28f, 0.45f);
    buffers[static_cast<std::size_t>(I::Ok)]      = makeChord({523.f, 659.f}, 0.18f, 0.40f);
    buffers[static_cast<std::size_t>(I::Unlock)]  = makeChord({392.f, 523.f, 659.f}, 0.35f, 0.42f);
    buffers[static_cast<std::size_t>(I::Restore)] = makeChord({440.f, 554.f}, 0.22f, 0.40f);
    buffers[static_cast<std::size_t>(I::Notify)]  = makeBeep(1046.f, 0.10f, 0.32f);
    buffers[static_cast<std::size_t>(I::Ending)]  = makeChord({523.f, 659.f, 784.f}, 0.55f, 0.45f);
    buffers[static_cast<std::size_t>(I::Perfect)] = makeChord({523.f, 659.f, 784.f, 1046.f}, 0.85f, 0.48f);
    buffers[static_cast<std::size_t>(I::Boot)].loadFromFile("assets/boot.wav");
    ready = true;
}

void Sfx::play(Id id) {
    if (!enabled || !ready) return;
    const auto idx = static_cast<std::size_t>(id);
    if (idx >= static_cast<std::size_t>(Id::Count)) return;
    auto& slot = voices[nextVoice];
    nextVoice = (nextVoice + 1) % kVoices;
    slot = std::make_unique<sf::Sound>(buffers[idx]);
    slot->setVolume(75.f);
    slot->play();
}

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

std::string wrapText(const std::string& str, const sf::Font& font, unsigned int charSize, float maxWidth, int maxLines) {
    if (str.empty()) return "";
    sf::String sfStr = sf::String::fromUtf8(str.begin(), str.end());
    sf::String sfResult;
    sf::String sfCurrentLine;
    sf::Text tester(font, "", charSize);
    int lineCount = 1;

    for (std::size_t i = 0; i < sfStr.getSize(); ++i) {
        auto c = sfStr[i];
        sfCurrentLine += c;
        tester.setString(sfCurrentLine);
        if (tester.getLocalBounds().size.x > maxWidth) {
            if (maxLines > 0 && lineCount >= maxLines) {
                sfCurrentLine.erase(sfCurrentLine.getSize() - 1, 1);
                while(sfCurrentLine.getSize() > 0) {
                    tester.setString(sfCurrentLine + "...");
                    if (tester.getLocalBounds().size.x <= maxWidth) break;
                    sfCurrentLine.erase(sfCurrentLine.getSize() - 1, 1);
                }
                sfResult += sfCurrentLine + "...";
                auto u8str = sfResult.toUtf8();
                return std::string(u8str.begin(), u8str.end());
            }
            sfCurrentLine.erase(sfCurrentLine.getSize() - 1, 1);
            sfResult += sfCurrentLine + "\n";
            sfCurrentLine = c;
            lineCount++;
        }
    }
    sfResult += sfCurrentLine;
    auto u8str2 = sfResult.toUtf8();
    return std::string(u8str2.begin(), u8str2.end());
}

bool StoryManager::tryUnlockArchive(const std::string& input) {
    if (input == keyPart1) {
        archiveOpened = true;
        advanceStage(3);
        Sfx::get().play(Sfx::Id::Unlock);
        if (g_Desktop) g_Desktop->syncStoryVisuals();
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
void StoryManager::markDocClueRead() {
    if (!hasDocClue) {
        hasDocClue = true;
        advanceStage(2);
        Sfx::get().play(Sfx::Id::Notify);
    }
}
void StoryManager::markBrowserClueSeen() {
    if (!hasBrowserClue) {
        hasBrowserClue = true;
        advanceStage(3);
        Sfx::get().play(Sfx::Id::Notify);
    }
}
void StoryManager::unlockRecycleBin() {
    if (!recycleBinUnlocked) {
        recycleBinUnlocked = true;
        advanceStage(4);
        Sfx::get().play(Sfx::Id::Unlock);
        if (g_Desktop) g_Desktop->syncStoryVisuals();
    }
}
void StoryManager::onKeyPart2Restored() {
    if (!keyPart2Restored) {
        keyPart2Restored = true;
        Sfx::get().play(Sfx::Id::Restore);
        unlockRewardExe();
    }
}
void StoryManager::unlockRewardExe() {
    isRewardUnlocked = true;
    if (g_Desktop) g_Desktop->syncStoryVisuals();
}
void StoryManager::triggerNormalEnding() {
    // 只设置标志 + 播放音效。窗口创建完全交给 Desktop 主循环安全处理
    if (hasNormalClear && !hasPerfectClear) return; // 已触发过
    hasNormalClear = true;
    Sfx::get().play(Sfx::Id::Ending);
}
void StoryManager::triggerPerfectEnding() {
    // 只设置标志。音效和窗口改造全部放到主循环，彻底安全
    hasPerfectClear = true;
    hasNormalClear = true;
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
    float maxWidth = 150.f;
    for (const auto& it : items) {
        sf::Text t(*menuFont, U8(it.label), 12u);
        if (it.isDefault) t.setStyle(sf::Text::Bold);
        float tw = t.getLocalBounds().size.x + 40.f;
        if (tw > maxWidth) maxWidth = tw;
    }
    width = maxWidth;
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
    if (!g_Desktop) return;
    // 设计文档：阅读项目备忘录推进阶段1→2
    if (name.find("项目备忘录") != std::string::npos)
        StoryManager::getInstance().markDocClueRead();
    // 兼容旧路径：如果有人直接以 TextFile 形式打开密钥文档也解锁
    if (name.find("密钥上半段") != std::string::npos || name.find("密钥") != std::string::npos)
        StoryManager::getInstance().unlockRecycleBin();
    if (g_Desktop->activateExisting(name)) return;
    g_Desktop->addWindow(new NotepadWindow(name, content, {100, 100}, 520, 420, g_Desktop->systemFont));
}
void LockedFile::open() {
    if (!g_Desktop) return;
    auto& story = StoryManager::getInstance();
    if (story.archiveOpened) {
        // 阅读密钥文档后正式解锁回收站（与 SecretFolder 打开双重保险，防卡关）
        if (name.find("密钥") != std::string::npos)
            story.unlockRecycleBin();
        if (g_Desktop->activateExisting(name)) return;
        const std::string body = content.empty()
            ? "【文件已解密】\n完整密钥 = 试飞日期 + 特斯拉 Model S 出厂编号\n后半段密钥已被移入回收站永久删除。\n请前往回收站还原文件。"
            : content;
        g_Desktop->addWindow(new NotepadWindow(name, body, {100, 100}, 520, 420, g_Desktop->systemFont));
    } else {
        if (g_Desktop->activateExisting("系统安全校验")) return;
        g_Desktop->queueWindow(new PasswordDialog({200, 150}, 340, 200, g_Desktop->systemFont, 0));
    }
}
void DeletedFile::open() {
    if (!g_Desktop) return;
    if (!StoryManager::getInstance().keyPart2Restored) {
        g_Desktop->addWindow(new MsgBoxWin(
            "回收站提示",
            "该文件已被永久删除。\n\n请先选中文件，然后点击上方【还原选定项目】按钮\n（或右键 → 还原）。\n还原后即可双击查看密钥后半段。",
            {280, 180}, 360, 180, g_Desktop->systemFont));
    } else {
        if (g_Desktop->activateExisting(name)) return;
        g_Desktop->addWindow(new NotepadWindow(name, content, {100, 100}, 520, 400, g_Desktop->systemFont));
    }
}
void ExeProgramFile::open() {
    StoryManager::getInstance().triggerNormalEnding();
}
void RestrictedDriveFile::open() {
    if (!g_Desktop) return;
    Sfx::get().play(Sfx::Id::Error);
    std::string msg = "无法访问 本地磁盘 (" + driveLetter + ":)。\n\n拒绝访问。";
    g_Desktop->addWindow(new MsgBoxWin(
        "拒绝访问", msg,
        {WIN_W/2.f - 160.f, WIN_H/2.f - 90.f}, 320, 180, g_Desktop->systemFont));
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
    if (g_Desktop && g_Desktop->appIconTex.getSize().x > 0) {
        sf::Sprite iconSpr(g_Desktop->appIconTex);
        iconSpr.setPosition(sf::Vector2f(position.x + BORDER + 4.f, position.y + BORDER + 2.f));
        float scale = (TITLEBAR_H - BORDER * 2.f - 4.f) / (float)iconSpr.getTexture().getSize().y;
        iconSpr.setScale(sf::Vector2f(scale, scale));
        win.draw(iconSpr);
        titleText.setPosition(sf::Vector2f(position.x + BORDER + 4.f + iconSpr.getGlobalBounds().size.x + 4.f, position.y + BORDER + 2.f));
    } else {
        titleText.setPosition(sf::Vector2f(position.x + BORDER + 4.f, position.y + BORDER + 2.f));
    }
    win.draw(titleText);
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
    cursorPos = addressInput.length();
    addrText.setFont(font); addrText.setCharacterSize(14); addrText.setFillColor(C::TextBlack); addrText.setString(U8(addressInput));
    contentText.setFont(font); contentText.setCharacterSize(14); contentText.setFillColor(C::TextBlack);
    statusText.setFont(font); statusText.setCharacterSize(12); statusText.setFillColor(C::TextBlack);
    addrBar.setFillColor(C::ClientWhite); goBtn.setFillColor(C::BevelMedium);
}
void BrowserWindow::renderContent(sf::RenderWindow& win) {
    sf::FloatRect cr = clientRect();
    sf::Text lbl(*winFont, U8("地址(D):"), 14u); lbl.setFillColor(C::TextBlack); lbl.setPosition(sf::Vector2f(cr.position.x + 5.f, cr.position.y + 10.f)); win.draw(lbl);
    addrBar.setPosition(sf::Vector2f(cr.position.x + 65.f, cr.position.y + 8.f)); addrBar.setSize(sf::Vector2f(cr.size.x - 130.f, 22.f)); win.draw(addrBar); drawDoubleBevel(win, addrBar.getGlobalBounds(), false);
    const bool cursorOn = editingAddr && ((int)(blinkClock.getElapsedTime().asSeconds() * 2) % 2 == 0);
    std::string disp = addressInput;
    if (cursorOn) { disp.insert(cursorPos, "|"); }
    addrText.setString(U8(disp));
    addrText.setPosition(sf::Vector2f(addrBar.getPosition().x + 5.f, addrBar.getPosition().y + 2.f)); win.draw(addrText);
    goBtn.setPosition(sf::Vector2f(cr.position.x + cr.size.x - 55.f, cr.position.y + 8.f)); goBtn.setSize(sf::Vector2f(45.f, 22.f)); win.draw(goBtn); drawDoubleBevel(win, goBtn.getGlobalBounds(), true);
    sf::Text goTxt(*winFont, U8("转到"), 12u); goTxt.setFillColor(C::TextBlack); goTxt.setPosition(sf::Vector2f(goBtn.getPosition().x + 10.f, goBtn.getPosition().y + 3.f)); win.draw(goTxt);
    sf::FloatRect pageRect(sf::Vector2f(cr.position.x, cr.position.y + 40.f), sf::Vector2f(cr.size.x, cr.size.y - 60.f));
    sf::RectangleShape page(pageRect.size); page.setPosition(pageRect.position); page.setFillColor(C::ClientWhite); win.draw(page); drawDoubleBevel(win, pageRect, false);
    if (perfectVictory) {
        contentText.setString(U8(
            "★★★★★★★★★★★★★★★★★★★★★★★★\n"
            "        ★ 完美隐藏结局 ★\n"
            "★★★★★★★★★★★★★★★★★★★★★★★★\n\n"
            "密码校验成功！\n\n"
            "已解锁：无限制 Grok 永久使用权\n"
            "已解锁：特斯拉 Model S 免费兑换资格\n\n"
            "恭喜你完整解开了埃隆·马斯克的\n"
            "尘封星舰工作站全部谜题！\n\n"
            "感谢游玩《尘封的星舰工作站》\n"
            "================================\n"
            "（可关闭此窗口，桌面已永久标记完美通关）"));
    } else if (showSecretPage) {
        contentText.setString(U8(
            "【系统覆盖 · 特斯拉奖励系统】\n"
            "================================\n"
            "已成功接入隐藏奖励通道。\n\n"
            "请输入完整认证密钥（12 位数字）以解锁终极奖励。\n\n"
            "提示：完整密钥 = 星舰首飞日期 + 特斯拉 Model S 出厂编号\n\n"
            "(系统已自动弹出密码校验窗口，请在窗口中输入)"));
    } else {
        contentText.setString(U8(
            "SpaceX 历史档案库\n"
            "===============================\n"
            "重大事件回顾（机密级）：\n\n"
            "【头条】星舰首飞（Starship Orbital Test Flight）\n"
            "发射日期：20230420\n"
            "地点：Starbase / Boca Chica\n"
            "状态：成功进入轨道测试\n\n"
            "备注：本页面由 2008 年内部系统归档生成。\n"
            "Grok 早期训练日志中亦有相关记录。\n\n"
            "--------------------------------"));
    }
    contentText.setPosition(sf::Vector2f(pageRect.position.x + 10.f, pageRect.position.y + 10.f)); win.draw(contentText);
    
    if (!perfectVictory && !showSecretPage) {
        sf::Text hintText(*winFont, U8("(提示：密钥后半段与特斯拉 Model S 出厂编号相关。完整解谜可获神秘大奖)"), 12u);
        hintText.setFillColor(sf::Color(120, 120, 120));
        hintText.setPosition(sf::Vector2f(pageRect.position.x + 10.f, pageRect.position.y + pageRect.size.y - 30.f));
        win.draw(hintText);
    }
    
    sf::RectangleShape sb(sf::Vector2f(cr.size.x, 20.f)); sb.setPosition(sf::Vector2f(cr.position.x, cr.position.y + cr.size.y - 20.f)); sb.setFillColor(C::StatusBar); win.draw(sb); drawDoubleBevel(win, sb.getGlobalBounds(), false);
}
void BrowserWindow::handleInput(const sf::Event& event) {
    if (const auto* mb = event.getIf<sf::Event::MouseButtonPressed>(); mb && mb->button == sf::Mouse::Button::Left) {
        sf::Vector2f mp(mb->position.x, mb->position.y);
        if (addrBar.getGlobalBounds().contains(mp)) editingAddr = true; else editingAddr = false;
        if (goBtn.getGlobalBounds().contains(mp)) navigateToAddress();
    }
    if (editingAddr && event.is<sf::Event::TextEntered>()) {
        if (event.getIf<sf::Event::TextEntered>()->unicode == 8 && cursorPos > 0) {
            addressInput.erase(cursorPos - 1, 1);
            cursorPos--;
        }
        else if (event.getIf<sf::Event::TextEntered>()->unicode >= 32 && event.getIf<sf::Event::TextEntered>()->unicode < 128) {
            addressInput.insert(cursorPos, 1, static_cast<char>(event.getIf<sf::Event::TextEntered>()->unicode));
            cursorPos++;
        }
    }
    if (editingAddr && event.is<sf::Event::KeyPressed>()) {
        auto code = event.getIf<sf::Event::KeyPressed>()->code;
        if (code == sf::Keyboard::Key::Enter) { navigateToAddress(); }
        else if (code == sf::Keyboard::Key::Left && cursorPos > 0) { cursorPos--; }
        else if (code == sf::Keyboard::Key::Right && cursorPos < (int)addressInput.length()) { cursorPos++; }
    }
}
void BrowserWindow::navigateToAddress() {
    // Normalize common prefixes for secret page
    std::string addr = addressInput;
    if (addr.rfind("http://", 0) == 0) addr = addr.substr(7);
    if (addr.rfind("https://", 0) == 0) addr = addr.substr(8);

    if (addr == "tesla-prize.local" || addressInput == "tesla-prize.local") {
        showSecretPage = true;
        perfectVictory = false;
        if (g_Desktop) {
            if (!g_Desktop->activateExisting("Grok 认证"))
                // 使用 queue 防止闪退
                g_Desktop->queueWindow(new PasswordDialog(
                    {position.x + 50, position.y + 50}, 340, 200, *winFont, 1));
        }
    } else {
        showSecretPage = false;
        perfectVictory = false;
        StoryManager::getInstance().markBrowserClueSeen();
    }
}
void BrowserWindow::showPerfectVictory() {
    perfectVictory = true;
    showSecretPage = true;
    addressInput = "tesla-prize.local";
    title = "Windows Internet Explorer - ★ 完美结局 ★";
    titleText.setString(U8(title));
}

FileListWin::FileListWin(const std::string& winTitle, bool recycleBin, sf::Vector2f p, float w, float h, const sf::Font& font) : PuzzleWindowBase(winTitle, p, w, h, font), isRecycleBinMode(recycleBin), winFont(&font), restoreBtnText(font) {
    folderPath = recycleBin ? "回收站" : "C:\\Documents and Settings\\Elon\\My Documents";
    texFolder.loadFromFile("assets/folder.png"); texFolder.setSmooth(true);
    texFile.loadFromFile("assets/notepad.png"); texFile.setSmooth(true);
    texExe.loadFromFile("assets/start_exe.png"); texExe.setSmooth(true);
    texComputer.loadFromFile("assets/computer.png"); texComputer.setSmooth(true);
}
FileListWin::~FileListWin() { for (auto f : files) delete f; files.clear(); }
void FileListWin::addFile(FileObject* file) { files.push_back(file); }
float FileListWin::listTopY() const { return clientRect().position.y + 45.f; }
void FileListWin::renderContent(sf::RenderWindow& win) {
    sf::FloatRect cr = clientRect();
    sf::RectangleShape addrBg(sf::Vector2f(cr.size.x, 35.f)); addrBg.setPosition(cr.position); addrBg.setFillColor(C::StatusBar); win.draw(addrBg);
    sf::Text addrLbl(*winFont, U8("地址(D): " + folderPath), 13u); addrLbl.setFillColor(C::TextBlack); addrLbl.setPosition(sf::Vector2f(cr.position.x + 10.f, cr.position.y + 9.f)); win.draw(addrLbl);
    sf::FloatRect listRect(sf::Vector2f(cr.position.x, cr.position.y + 35.f), sf::Vector2f(cr.size.x, cr.size.y - 35.f));
    sf::RectangleShape listBg(listRect.size); listBg.setPosition(listRect.position); listBg.setFillColor(C::ClientWhite); win.draw(listBg); drawDoubleBevel(win, listRect, false);

    // ── Special render for My Computer: show drives with storage bars ──
    if (folderPath == "我的电脑") {
        // Drive data: {name, used_gb, total_gb, label}
        struct DriveInfo { std::string name; float used; float total; std::string label; };
        DriveInfo drives[] = {
            {"本地磁盘 (C:)", 18.6f, 40.0f, "系统盘"},
            {"本地磁盘 (D:)", 42.3f, 120.0f, "数据盘"}
        };
        float dy = listRect.position.y + 20.f;
        for (size_t di = 0; di < 2; ++di) {
            float dx = listRect.position.x + 20.f;
            // Selection highlight
            if ((int)di == selectedIdx) {
                sf::RectangleShape sel(sf::Vector2f(listRect.size.x - 30.f, 72.f));
                sel.setPosition(sf::Vector2f(dx - 5.f, dy - 5.f));
                sel.setFillColor(C::SelectIcon); win.draw(sel);
            }
            // Drive icon
            sf::Sprite iconSpr(texComputer);
            float sc = 48.f / std::max(texComputer.getSize().x, texComputer.getSize().y);
            iconSpr.setScale(sf::Vector2f(sc, sc));
            iconSpr.setPosition(sf::Vector2f(dx, dy + 8.f));
            win.draw(iconSpr);
            // Drive name
            sf::Text nameTxt(*winFont, U8(drives[di].name), 14u);
            nameTxt.setFillColor(C::TextBlack);
            nameTxt.setPosition(sf::Vector2f(dx + 60.f, dy + 4.f));
            win.draw(nameTxt);
            // Storage bar background
            float barW = std::min(280.f, listRect.size.x - 120.f);
            sf::RectangleShape barBg(sf::Vector2f(barW, 16.f));
            barBg.setPosition(sf::Vector2f(dx + 60.f, dy + 26.f));
            barBg.setFillColor(sf::Color(200, 200, 200));
            barBg.setOutlineColor(sf::Color(128,128,128)); barBg.setOutlineThickness(1.f);
            win.draw(barBg);
            // Storage bar fill (XP blue, turns red if > 90%)
            float ratio = drives[di].used / drives[di].total;
            sf::Color barColor = ratio > 0.9f ? sf::Color(200, 50, 50) : sf::Color(58, 110, 200);
            sf::RectangleShape barFill(sf::Vector2f(barW * ratio, 16.f));
            barFill.setPosition(barBg.getPosition());
            barFill.setFillColor(barColor);
            win.draw(barFill);
            // Storage text: "xx.x GB 可用，共 xx.x GB"
            float free_gb = drives[di].total - drives[di].used;
            std::ostringstream ss;
            ss.precision(1); ss << std::fixed;
            ss << free_gb << " GB 可用，共 " << drives[di].total << " GB";
            sf::Text storageTxt(*winFont, U8(ss.str()), 12u);
            storageTxt.setFillColor(C::TextBlack);
            storageTxt.setPosition(sf::Vector2f(dx + 60.f, dy + 46.f));
            win.draw(storageTxt);
            dy += 82.f;
        }
        return;
    }

    // ── Normal file list render ──
    constexpr float ICON_DRAW_SIZE = 48.f;
    constexpr float CELL_W = 120.f;
    constexpr float CELL_H = 96.f;
    float sx = listRect.position.x + 20.f; float sy = listRect.position.y + 16.f;
    for (size_t i = 0; i < files.size(); ++i) {
        // Selection box drawn LATER to accommodate dynamic height
        const sf::Texture* currentTex = &texFile;
        if (files[i]->iconType == "locked") {
            currentTex = &texFolder;
        } else if (files[i]->iconType == "exe") {
            currentTex = &texExe;
        }
        sf::Sprite iconSpr(*currentTex);
        float scale = ICON_DRAW_SIZE / std::max(currentTex->getSize().x, currentTex->getSize().y);
        iconSpr.setScale(sf::Vector2f(scale, scale));
        iconSpr.setPosition(sf::Vector2f(sx + (CELL_W - 16.f - iconSpr.getGlobalBounds().size.x) / 2.f, sy));
        if (files[i]->iconType == "restored") iconSpr.setColor(sf::Color(255, 255, 255, 200));
        win.draw(iconSpr);

        std::string displayName = files[i]->name;
        if (isRecycleBinMode && files[i]->iconType != "restored" &&
            !StoryManager::getInstance().keyPart2Restored)
            displayName = displayName + " (已删除)";
            
        float maxTextW = CELL_W - 16.f;
        std::string wrapped = wrapText(displayName, *winFont, 14, maxTextW, ((int)i == selectedIdx) ? 0 : 2);
        
        sf::Text fn(*winFont, U8(wrapped), 14u);
        fn.setFillColor(C::TextBlack);
        
        // Centered horizontally, but could be multi-line
        sf::FloatRect bounds = fn.getLocalBounds();
        fn.setPosition(sf::Vector2f(sx + (CELL_W - 16.f) / 2.f - bounds.size.x / 2.f, sy + ICON_DRAW_SIZE + 4.f));
        
        // If selected, expand the selection box dynamically
        if ((int)i == selectedIdx) {
            float boxHeight = ICON_DRAW_SIZE + 4.f + bounds.size.y + 6.f;
            if (boxHeight < CELL_H - 8.f) boxHeight = CELL_H - 8.f;
            sf::RectangleShape sel(sf::Vector2f(CELL_W, boxHeight)); 
            sel.setPosition(sf::Vector2f(sx - 8.f, sy - 4.f)); 
            sel.setFillColor(C::SelectIcon); 
            win.draw(sel);
            
            // Re-draw icon over the selection box since we draw selection late
            iconSpr.setColor(files[i]->iconType == "restored" ? sf::Color(255, 255, 255, 200) : sf::Color::White);
            win.draw(iconSpr);
        }
        
        win.draw(fn);

        sx += CELL_W;
        if (sx + CELL_W > listRect.position.x + listRect.size.x) {
            sx = listRect.position.x + 20.f;
            sy += CELL_H;
        }
    }
}
void FileListWin::handleInput(const sf::Event& event) {
    if (const auto* mb = event.getIf<sf::Event::MouseButtonPressed>(); mb && mb->button == sf::Mouse::Button::Left) {
        sf::Vector2f mp(mb->position.x, mb->position.y);
        if (isRecycleBinMode && restoreBtn.getGlobalBounds().contains(mp)) { restoreSelectedFile(); return; }

        sf::FloatRect listRect(sf::Vector2f(clientRect().position.x, clientRect().position.y + 35.f),
                               sf::Vector2f(clientRect().size.x, clientRect().size.y - 35.f));

        // My Computer: hit test drives by row
        if (folderPath == "我的电脑") {
            float dy = listRect.position.y + 20.f;
            for (size_t i = 0; i < files.size(); ++i) {
                sf::FloatRect hitBox(sf::Vector2f(listRect.position.x + 15.f, dy - 5.f),
                                     sf::Vector2f(listRect.size.x - 25.f, 72.f));
                if (hitBox.contains(mp)) {
                    const bool isDouble = selectedIdx == static_cast<int>(i) &&
                        listClickClock.getElapsedTime().asMilliseconds() < DOUBLE_CLICK_MS;
                    selectedIdx = static_cast<int>(i);
                    if (isDouble) openSelectedFile();
                    else Sfx::get().play(Sfx::Id::Click);
                    listClickClock.restart();
                    return;
                }
                dy += 82.f;
            }
            selectedIdx = -1;
            return;
        }

        // Normal file list
        constexpr float CELL_W = 120.f;
        constexpr float CELL_H = 96.f;
        float sx = listRect.position.x + 20.f;
        float sy = listRect.position.y + 16.f;
        for (size_t i = 0; i < files.size(); ++i) {
            sf::FloatRect hitBox(sf::Vector2f(sx - 8.f, sy - 4.f), sf::Vector2f(CELL_W, CELL_H));
            if (hitBox.contains(mp)) {
                const bool isDouble =
                    selectedIdx == static_cast<int>(i) &&
                    listClickClock.getElapsedTime().asMilliseconds() < DOUBLE_CLICK_MS;
                selectedIdx = static_cast<int>(i);
                if (isDouble) openSelectedFile();
                else Sfx::get().play(Sfx::Id::Click);
                listClickClock.restart();
                return;
            }
            sx += CELL_W;
            if (sx + CELL_W > listRect.position.x + listRect.size.x) {
                sx = listRect.position.x + 20.f;
                sy += CELL_H;
            }
        }
        selectedIdx = -1;
    }
}
void FileListWin::openSelectedFile() {
    if (selectedIdx >= 0 && selectedIdx < (int)files.size()) {
        Sfx::get().play(Sfx::Id::Open);
        files[selectedIdx]->open();
    }
}
void FileListWin::restoreSelectedFile() {
    if (selectedIdx < 0 || selectedIdx >= (int)files.size()) return;
    if (!isRecycleBinMode) return;
    
    const std::string fname = files[selectedIdx]->name;
    // 仅对目标文件触发剧情推进
    if (fname.find("特斯拉") != std::string::npos || fname.find("编号") != std::string::npos) {
        StoryManager::getInstance().onKeyPart2Restored();
    }
    
    std::string content = "";
    if (auto* delFile = dynamic_cast<DeletedFile*>(files[selectedIdx])) {
        content = delFile->content;
    }
    
    // Remove from recycle bin
    delete files[selectedIdx];
    files.erase(files.begin() + selectedIdx);
    selectedIdx = -1;
    
    // Add to desktop
    if (g_Desktop) {
        TextFileIcon* restoredIcon = new TextFileIcon({0,0}, "assets/notepad.png", *winFont, fname, content);
        g_Desktop->icons.push_back(restoredIcon);
        // allocate grid slot
        restoredIcon->gridSlot = g_Desktop->allocateSlot();
        restoredIcon->position = g_Desktop->slotPosition(restoredIcon->gridSlot);
        g_Desktop->saveIconPositions();
        
        g_Desktop->addWindow(new MsgBoxWin(
            "回收站",
            "文件已还原到桌面！\n\n现在可以双击打开查看内容，\n获取密钥后半段（2012），\n然后拼接完整密钥 202304202012。\n\n桌面已解锁 Starship.exe 彩蛋程序。",
            {position.x + 50, position.y + 50}, 380, 200, *winFont));
    }
    refreshFiles();
}
void FileListWin::refreshFiles() {
    // Visual state is driven by StoryManager + iconType (e.g. "restored")
}
void FileListWin::buildClientContextMenu(std::vector<MenuItem>& out, sf::Vector2f mp) {
    if (selectedIdx >= 0 && selectedIdx < (int)files.size()) {
        out.push_back({"打开", [this](){ openSelectedFile(); }, true, false, true});
        if (isRecycleBinMode) { out.push_back({"", nullptr, false, true, false}); out.push_back({"还原", [this](){ restoreSelectedFile(); }}); }
    } else { out.push_back({"查看", nullptr, false}); out.push_back({"刷新", [this](){ refreshFiles(); }}); }
}

PasswordDialog::PasswordDialog(sf::Vector2f p, float w, float h, const sf::Font& font, int pwdMode) : PuzzleWindowBase(pwdMode == 0 ? "系统安全校验" : "Grok 认证", p, w, h, font), winFont(&font), mode(pwdMode), promptText(font), inputText(font), okText(font), cancelText(font) {
    isModal = true; canMinimize = false; canMaximize = false;
    promptText.setFont(font); promptText.setCharacterSize(14); promptText.setFillColor(C::TextBlack);
    if (mode == 0) promptText.setString(U8("请输入加密档案室访问密钥\n(提示：星舰首飞日期，8位数字)")); 
    else promptText.setString(U8("请输入完整认证密钥（12位数字）：\n试飞日期 + Model S 出厂编号"));
    inputText.setFont(font); inputText.setCharacterSize(20); inputText.setFillColor(C::TextBlack);
    inputField.setFillColor(C::ClientWhite); okBtn.setFillColor(C::BevelMedium); cancelBtn.setFillColor(C::BevelMedium);
    okText.setFont(font); okText.setCharacterSize(12); okText.setFillColor(C::TextBlack); okText.setString(U8("确定"));
    cancelText.setFont(font); cancelText.setCharacterSize(12); cancelText.setFillColor(C::TextBlack); cancelText.setString(U8("取消"));
}
void PasswordDialog::renderContent(sf::RenderWindow& win) {
    sf::FloatRect cr = clientRect();
    promptText.setPosition(sf::Vector2f(cr.position.x + 20.f, cr.position.y + 12.f)); 
    promptText.setCharacterSize(13);
    win.draw(promptText);
    // 多行提示后下移输入框，避免重叠
    float inputY = cr.position.y + 55.f;
    inputField.setPosition(sf::Vector2f(cr.position.x + 20.f, inputY)); 
    inputField.setSize(sf::Vector2f(cr.size.x - 40.f, 28.f)); 
    win.draw(inputField); drawDoubleBevel(win, inputField.getGlobalBounds(), false);
    const bool cursorOn = !hasError && ((int)(blinkClock.getElapsedTime().asSeconds() * 2) % 2 == 0);
    inputText.setString(hasError ? U8("密码错误！") : U8(typedPassword + (cursorOn ? "|" : "")));
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
            Sfx::get().play(Sfx::Id::Ok);
            closeWindow();  // 只关闭，桌面图标变化作为反馈
        } else {
            hasError = true;
            Sfx::get().play(Sfx::Id::Error);
        }
    } else {
        // 隐藏结局：成功后只关闭密码框 + 触发浏览器变通关页（零新窗口，最稳）
        if (StoryManager::getInstance().tryPerfectKey(typedPassword)) {
            Sfx::get().play(Sfx::Id::Ok);
            closeWindow();
            // triggerPerfectEnding 已在 tryPerfectKey 里调用，会改造浏览器
        } else {
            hasError = true;
            Sfx::get().play(Sfx::Id::Error);
        }
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
    sf::Text info(*winFont, U8(
        "系统:\nMicrosoft Windows XP\nProfessional\n版本 2008 (内部定制)\n\n"
        "注册到:\nElon Musk\nSpaceX\n\n"
        "计算机名: STARSHIP-WS-01\n"
        "工作组: SPACEX-INTERNAL\n\n"
        "【注意】本机存储星舰机密与 Grok 早期日志\n"
        "部分功能已被加密保护"), 12u);
    info.setFillColor(C::TextBlack); info.setPosition(sf::Vector2f(cr.position.x + 20.f, cr.position.y + 15.f)); win.draw(info);
    // 隐藏按钮：加密档案室（设计文档核心入口）
    secretBtn.setPosition(sf::Vector2f(cr.position.x + 20.f, cr.position.y + cr.size.y - 90.f)); 
    secretBtn.setSize(sf::Vector2f(130.f, 26.f)); 
    win.draw(secretBtn); drawDoubleBevel(win, secretBtn.getGlobalBounds(), true); 
    secretBtnText.setPosition(sf::Vector2f(secretBtn.getPosition().x + 12.f, secretBtn.getPosition().y + 5.f)); win.draw(secretBtnText);
    okBtn.setPosition(sf::Vector2f(cr.position.x + cr.size.x - 180.f, cr.position.y + cr.size.y - 40.f)); okBtn.setSize(sf::Vector2f(70.f, 24.f)); win.draw(okBtn); drawDoubleBevel(win, okBtn.getGlobalBounds(), true); okText.setPosition(sf::Vector2f(okBtn.getPosition().x + 22.f, okBtn.getPosition().y + 4.f)); win.draw(okText);
    cancelBtn.setPosition(sf::Vector2f(cr.position.x + cr.size.x - 90.f, cr.position.y + cr.size.y - 40.f)); cancelBtn.setSize(sf::Vector2f(70.f, 24.f)); win.draw(cancelBtn); drawDoubleBevel(win, cancelBtn.getGlobalBounds(), true); cancelText.setPosition(sf::Vector2f(cancelBtn.getPosition().x + 22.f, cancelBtn.getPosition().y + 4.f)); win.draw(cancelText);
}
void SystemPropWin::handleInput(const sf::Event& event) {
    if (const auto* mb = event.getIf<sf::Event::MouseButtonPressed>(); mb && mb->button == sf::Mouse::Button::Left) {
        sf::Vector2f mp(mb->position.x, mb->position.y);
        if (secretBtn.getGlobalBounds().contains(mp) && g_Desktop) {
            if (!g_Desktop->activateExisting("系统安全校验"))
                g_Desktop->queueWindow(new PasswordDialog(
                    {position.x + 50, position.y + 50}, 340, 200, *winFont, 0));
        }
        if (okBtn.getGlobalBounds().contains(mp) || cancelBtn.getGlobalBounds().contains(mp))
            closeWindow();
    }
}

RewardAnimWindow::RewardAnimWindow(sf::Vector2f p, float w, float h, const sf::Font& font) : PuzzleWindowBase("系统奖励", p, w, h, font), congratsText(font) {
    congratsText.setFont(font); congratsText.setCharacterSize(16); congratsText.setFillColor(C::TextBlack); congratsText.setStyle(sf::Text::Bold);
    if (StoryManager::getInstance().hasPerfectClear) {
        congratsText.setString(U8(
            "★ 完美隐藏结局 ★\n"
            "================================\n"
            "密码校验成功！\n\n"
            "已解锁：无限制 Grok 永久使用权\n"
            "已解锁：特斯拉 Model S 免费兑换资格\n\n"
            "恭喜你完整解开了埃隆·马斯克的\n"
            "尘封星舰工作站全部谜题！\n"
            "感谢游玩《尘封的星舰工作站》"));
    } else {
        congratsText.setString(U8(
            "☆ 普通结局 ☆\n"
            "================================\n"
            "基础剧情通关成功！\n\n"
            "已解锁：无限制 Grok 永久使用权\n\n"
            "（提示：似乎还有隐藏页面未探索……\n"
            "  试试在 IE 地址栏输入\n"
            "  tesla-prize.local 并输入完整密钥？）"));
    }
}
void RewardAnimWindow::renderContent(sf::RenderWindow& win) {
    sf::FloatRect cr = clientRect(); sf::RectangleShape bg(cr.size); bg.setPosition(cr.position); bg.setFillColor(C::ClientWhite); win.draw(bg); drawDoubleBevel(win, cr, false);
    if ((int)(animationClock.getElapsedTime().asSeconds() * 2) % 2 == 0) congratsText.setFillColor(StoryManager::getInstance().hasPerfectClear ? sf::Color(200, 100, 0) : sf::Color::Blue);
    else congratsText.setFillColor(C::TextBlack);
    congratsText.setPosition(sf::Vector2f(cr.position.x + 20.f, cr.position.y + 50.f)); win.draw(congratsText);
}
void RewardAnimWindow::handleInput(const sf::Event& event) {}

MsgBoxWin::MsgBoxWin(const std::string& boxTitle, const std::string& message, sf::Vector2f p, float w, float h, const sf::Font& font) : PuzzleWindowBase(boxTitle, p, w, h, font), msgText(font), okBtnText(font) {
    isModal = false; canMaximize = false; canMinimize = false; msgText.setFont(font); msgText.setCharacterSize(14); msgText.setFillColor(C::TextBlack); msgText.setString(U8(message));
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
AppIconBase::AppIconBase(sf::Vector2f p, const std::string& lbl, const std::string& texPath, const sf::Font& font) : UIComponent(p, ICON_W, ICON_CELL_H), label(lbl), labelText(font), iconSpr(iconTex), iconFont(&font) {
    sf::Image img;
    if (img.loadFromFile(texPath)) {
        (void)iconTex.loadFromImage(img); iconSpr.setTexture(iconTex, true);
        float scale = std::min(ICON_W / (float)iconTex.getSize().x, ICON_H / (float)iconTex.getSize().y);
        iconSpr.setScale(sf::Vector2f(scale, scale));
    }
    labelText.setFont(font); labelText.setCharacterSize(16); labelText.setString(U8(label)); labelText.setFillColor(C::TextWhite);
}
void AppIconBase::render(sf::RenderWindow& win) {
    std::string wrapped = wrapText(label, *iconFont, labelText.getCharacterSize(), width + 10.f, selected ? 0 : 2);
    labelText.setString(U8(wrapped));

    sf::FloatRect tr = labelText.getLocalBounds();
    float tx = position.x + width/2.f - tr.size.x/2.f;
    float ty = position.y + ICON_H + 4.f;
    labelText.setPosition(sf::Vector2f(tx, ty));

    if (selected || highlight) {
        const float padX = 5.f;
        const float boxTop    = position.y + 2.f;
        const float boxBottom = ty + tr.size.y + 4.f;
        const float boxCX     = position.x + width / 2.f;
        const float boxHalfW  = std::max(ICON_W / 2.f + padX, tr.size.x / 2.f + padX);

        sf::RectangleShape selBox(sf::Vector2f(boxHalfW * 2.f, boxBottom - boxTop));
        selBox.setPosition(sf::Vector2f(boxCX - boxHalfW, boxTop));
        selBox.setFillColor(highlight ? sf::Color(255, 255, 0, 100) : C::SelectIcon);
        win.draw(selBox);
    }

    float scaledW = iconTex.getSize().x * iconSpr.getScale().x;
    float scaledH = iconTex.getSize().y * iconSpr.getScale().y;
    iconSpr.setPosition(sf::Vector2f(position.x + (width - scaledW)/2.f, position.y + (ICON_H - scaledH)/2.f + 4.f));
    win.draw(iconSpr);

    if (!selected) {
        // Drop shadow when not selected
        sf::Text ts = labelText;
        ts.setFillColor(sf::Color(0, 0, 0, 150));
        ts.setPosition(sf::Vector2f(tx + 1.f, ty + 1.f));
        win.draw(ts);
    }
    win.draw(labelText);
}
bool AppIconBase::isMouseHit(sf::Vector2f mp) { return bounds().contains(mp); }
void AppIconBase::buildContextMenu(std::vector<MenuItem>& out) {
    out.push_back({"打开(O)", [this](){ onDoubleClick(); }, true, false, true}); out.push_back({"属性(R)", nullptr, false});
}

MyDocumentsIcon::MyDocumentsIcon(sf::Vector2f p, const std::string& texPath, const sf::Font& font) : AppIconBase(p, "我的文档", texPath, font) {}
void MyDocumentsIcon::onDoubleClick() {
    if (!g_Desktop) return;
    Sfx::get().play(Sfx::Id::Open);
    if (g_Desktop->activateExisting("我的文档")) return;
    FileListWin* win = new FileListWin("我的文档", false, {100, 100}, 500, 400, g_Desktop->systemFont);
    win->folderPath = "我的文档";
    // 完整剧情线索：对应设计文档阶段1 —— 项目备忘录（主线密钥分段机制）
    win->addFile(new TextFile(
        "项目备忘录.txt",
        "【SpaceX 机密 · 项目备忘录】\n"
        "================================\n"
        "日期：2008 年内部归档\n"
        "主题：星舰初代试飞相关核心资料加密保护\n\n"
        "埃隆特别指示：为防止核心资料外泄，解谜密钥已分段拆分。\n"
        "第一段密钥 = 星舰首飞（Starship Orbital Test Flight）日期\n"
        "完整密钥分为两段，后半段已被移至回收站永久删除。\n\n"
        "提示：请先通过 IE 浏览器查阅历史档案，确认首飞日期。\n"
        "密钥后半段与特斯拉车型出厂编号相关。\n\n"
        "—— 本备忘录由 Grok 早期训练日志系统自动生成"));
    // 加密提示.doc —— 引导系统属性隐藏入口
    win->addFile(new TextFile(
        "加密提示.doc",
        "【隐藏入口提示】\n"
        "================================\n"
        "加密档案室入口并不在桌面普通图标上。\n"
        "请右键单击【我的电脑】→ 选择【属性(R)】\n"
        "在系统属性面板中寻找【加密档案室...】隐藏按钮。\n\n"
        "输入第一段密钥后即可解锁机密文件夹。\n"
        "解锁后桌面会出现高亮的加密档案室图标。"));
    g_Desktop->addWindow(win);
}
void MyDocumentsIcon::buildContextMenu(std::vector<MenuItem>& out) {
    AppIconBase::buildContextMenu(out);
    out.insert(out.begin() + 1, {"", nullptr, false, true});
    // 设计文档伏笔1：右键小字提示
    out.insert(out.begin() + 2, {"提示:星舰首飞藏在网页，密码不止一组数字", nullptr, false});
}

IEBrowserIcon::IEBrowserIcon(sf::Vector2f p, const std::string& texPath, const sf::Font& font) : AppIconBase(p, "IE浏览器", texPath, font) {}
void IEBrowserIcon::onDoubleClick() {
    if (!g_Desktop) return;
    Sfx::get().play(Sfx::Id::Open);
    StoryManager::getInstance().markBrowserClueSeen();
    if (g_Desktop->activateExisting("Windows Internet Explorer")) return;
    g_Desktop->addWindow(new BrowserWindow({150, 150}, 600, 450, g_Desktop->systemFont));
}

TextFileIcon::TextFileIcon(sf::Vector2f p, const std::string& texPath, const sf::Font& font, const std::string& n, const std::string& c) : AppIconBase(p, n, texPath, font), content(c) {}
void TextFileIcon::onDoubleClick() {
    if (!g_Desktop) return;
    Sfx::get().play(Sfx::Id::Open);
    if (g_Desktop->activateExisting(label)) return;
    g_Desktop->addWindow(new NotepadWindow(label, content, {100, 100}, 500, 400, g_Desktop->systemFont));
}

RecycleBinIcon::RecycleBinIcon(sf::Vector2f p, const std::string& texPath, const sf::Font& font) : AppIconBase(p, "回收站", texPath, font) {}
void RecycleBinIcon::onDoubleClick() {
    if (!g_Desktop) return;
    if (!StoryManager::getInstance().recycleBinUnlocked) {
        Sfx::get().play(Sfx::Id::Error);
        g_Desktop->addWindow(new MsgBoxWin(
            "拒绝访问",
            "该文件夹已被加密系统锁定，禁止访问。\n请先通过【加密档案室】验证权限并阅读密钥文档。",
            {200, 200}, 340, 160, g_Desktop->systemFont));
        return;
    }
    Sfx::get().play(Sfx::Id::Open);
    if (g_Desktop->activateExisting("回收站")) return;
    FileListWin* win = new FileListWin("回收站", true, {200, 200}, 500, 400, g_Desktop->systemFont);
    // 设计文档阶段4：DeletedFile《特斯拉编号记录.txt》
    auto* deleted = new DeletedFile(
        "特斯拉编号记录.txt",
        "【特斯拉车辆出厂记录 · 已删除】\n"
        "================================\n"
        "车型：Tesla Model S\n"
        "出厂编号：2012\n"
        "备注：此编号为密钥后半段。\n\n"
        "完整密钥 = 20230420 + 2012 = 202304202012\n\n"
        "（请使用此完整密钥在浏览器隐藏页面触发完美结局）");
    if (StoryManager::getInstance().keyPart2Restored)
        deleted->iconType = "restored";
    win->addFile(deleted);
    g_Desktop->addWindow(win);
}

SecretFolderIcon::SecretFolderIcon(sf::Vector2f p, const std::string& texPath, const sf::Font& font) : AppIconBase(p, "加密档案室", texPath, font) {}
void SecretFolderIcon::onDoubleClick() {
    if (!g_Desktop) return;
    if (!StoryManager::getInstance().archiveOpened) return;
    Sfx::get().play(Sfx::Id::Open);
    // 访问加密档案室即视为获取反转线索，解锁回收站（防止卡关，同时保持可阅读文档）
    StoryManager::getInstance().unlockRecycleBin();
    if (g_Desktop->activateExisting("加密档案室")) return;
    FileListWin* win = new FileListWin("加密档案室", false, {250, 250}, 420, 320, g_Desktop->systemFont);
    win->folderPath = "加密档案室";
    // 设计文档阶段3：LockedFile《密钥上半段》—— 核心反转设定
    win->addFile(new LockedFile(
        "密钥上半段.txt",
        "【文件已解密 · 密钥上半段说明】\n"
        "================================\n"
        "完整密钥 = 试飞日期 + 特斯拉 Model S 出厂编号\n\n"
        "第一段密钥（试飞日期）：20230420\n"
        "后半段密钥已被移入回收站永久删除。\n\n"
        "请立即前往【回收站】，还原《特斯拉编号记录.txt》，\n"
        "读取出厂编号后拼接完整密钥。\n\n"
        "拼接完成后可双击桌面 Starship.exe 获得普通结局，\n"
        "或在浏览器地址栏输入 tesla-prize.local 输入完整密钥\n"
        "触发完美隐藏结局（特斯拉 + 无限制 Grok）。"));
    g_Desktop->addWindow(win);
    highlight = false;
}

TeslaExeIcon::TeslaExeIcon(sf::Vector2f p, const std::string& texPath, const sf::Font& font) : AppIconBase(p, "Starship.exe", texPath, font) {}
void TeslaExeIcon::onDoubleClick() { StoryManager::getInstance().triggerNormalEnding(); }

MyComputerIcon::MyComputerIcon(sf::Vector2f p, const std::string& texPath, const sf::Font& font) : AppIconBase(p, "我的电脑", texPath, font) {}
void MyComputerIcon::onDoubleClick() {
    if (!g_Desktop) return;
    Sfx::get().play(Sfx::Id::Open);
    if (g_Desktop->activateExisting("我的电脑")) return;
    FileListWin* win = new FileListWin("我的电脑", false, {50, 50}, 500, 400, g_Desktop->systemFont);
    win->folderPath = "我的电脑";
    win->addFile(new RestrictedDriveFile("本地磁盘 (C:)", "C"));
    win->addFile(new RestrictedDriveFile("本地磁盘 (D:)", "D"));
    g_Desktop->addWindow(win);
}
void MyComputerIcon::buildContextMenu(std::vector<MenuItem>& out) {
    AppIconBase::buildContextMenu(out);
    out.back() = {"属性(R)", []() {
        if (!g_Desktop) return;
        if (g_Desktop->activateExisting("系统属性")) return;
        g_Desktop->addWindow(new SystemPropWin({100, 100}, 400, 450, g_Desktop->systemFont));
    }, true};
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
    int maxRows = (int)((WIN_H - TASKBAR_H - GRID_MARGIN_Y) / GRID_ROW_H);
    int best = -1;
    float bestDist = 1e9f;
    for (int s = 0; s < GRID_COLS * maxRows; ++s) {
        bool occupied = false;
        for (auto icon : icons) {
            if (icon->visible && icon->gridSlot == s && icon != draggingIcon) {
                occupied = true; break;
            }
        }
        if (occupied) continue;
        
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
Desktop::~Desktop() { 
    for (auto i : icons) delete i; 
    for (auto w : openWindows) delete w; 
    for (auto w : pendingWindows) delete w;
    g_Desktop = nullptr; 
}
void Desktop::init(sf::RenderWindow& win) {
    renderWin = &win;
    Sfx::get().init();
    if (!appIconTex.loadFromFile("assets/app_icon.jpg")) {
        appIconTex.loadFromFile("D:/Xgame/Xgame/assets/app_icon.jpg");
    }
    appIconTex.setSmooth(true);
    // Prefer bundled font, then common Chinese system fonts, then Latin fallback
    const char* fontCandidates[] = {
        "assets/fonts/NotoSansSC-Regular.otf",
        "assets/fonts/SourceHanSansSC-Regular.otf",
        "C:\\Windows\\Fonts\\msyh.ttc",
        "C:\\Windows\\Fonts\\msyhbd.ttc",
        "C:\\Windows\\Fonts\\simsun.ttc",
        "C:\\Windows\\Fonts\\simhei.ttf",
        "C:\\Windows\\Fonts\\arial.ttf",
    };
    bool fontOk = false;
    for (const char* path : fontCandidates) {
        if (systemFont.openFromFile(path)) { fontOk = true; break; }
    }
    if (!fontOk) {
        std::cerr << "[warn] No UI font loaded; Chinese text may not render.\n";
    }
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
    assignSlot(secretIcon, "SecretFolder");
    secretIcon->visible = false; // unlocked via archive password
    icons.push_back(secretIcon);

    rewardIcon = new TeslaExeIcon({0,0}, "assets/start_exe.png", systemFont);
    assignSlot(rewardIcon, "TeslaExe"); rewardIcon->visible = false; icons.push_back(rewardIcon);

    // Load saved positions (overrides default grid assignments)
    loadIconPositions();
    syncStoryVisuals();
}
void Desktop::addWindow(PuzzleWindowBase* win) { 
    openWindows.push_back(win); 
    setActiveWindow(win); 
}
// 安全延迟添加：任何从事件回调 / 密码成功 / 双击 / StoryManager 触发的窗口创建都应该用这个
void Desktop::queueWindow(PuzzleWindowBase* win) {
    if (win) pendingWindows.push_back(win);
}
void Desktop::flushPendingWindows() {
    for (auto* w : pendingWindows) {
        if (w) {
            openWindows.push_back(w);
            setActiveWindow(w);
        }
    }
    pendingWindows.clear();
}
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
    for (auto w : openWindows) {
        if (w->isOpen && w->title.find(title) != std::string::npos) return w;
    }
    return nullptr;
}
bool Desktop::activateExisting(const std::string& title) {
    PuzzleWindowBase* w = findWindowByTitle(title);
    if (!w) return false;
    w->restoreFromTaskbar();
    setActiveWindow(w);
    return true;
}
void Desktop::syncStoryVisuals() {
    auto& story = StoryManager::getInstance();
    
    auto checkOverlap = [&](AppIconBase* ic) {
        if (!ic->visible) return;
        for (auto other : icons) {
            if (other != ic && other->visible && other->gridSlot == ic->gridSlot) {
                ic->gridSlot = nearestFreeSlot(ic->position);
                if (ic->gridSlot >= 0) ic->position = slotPosition(ic->gridSlot);
                saveIconPositions();
                break;
            }
        }
    };

    if (secretIcon) {
        bool wasVisible = secretIcon->visible;
        secretIcon->visible = story.archiveOpened;
        secretIcon->highlight = story.archiveOpened && !story.recycleBinUnlocked;
        if (!wasVisible && secretIcon->visible) checkOverlap(secretIcon);
    }
    if (rewardIcon) {
        bool wasVisible = rewardIcon->visible;
        rewardIcon->visible = story.isRewardUnlocked;
        if (!wasVisible && rewardIcon->visible) checkOverlap(rewardIcon);
    }
}

void Desktop::hideShellMenu() { shellMenu.hide(); }
std::vector<PuzzleWindowBase*> Desktop::visibleTaskWindows() const {
    std::vector<PuzzleWindowBase*> res; for (auto w : openWindows) if (w->isOpen) res.push_back(w); return res;
}
sf::FloatRect Desktop::taskButtonRect(std::size_t index) const {
    float startX = 100.f; float w = TASK_BTN_W; return sf::FloatRect(sf::Vector2f(startX + index * (w + 4.f), WIN_H - TASKBAR_H + 4.f), sf::Vector2f(w, TASKBAR_H - 8.f));
}
void Desktop::handleTaskbarClick(sf::Vector2f mp) {
    // Start button
    if (startSpr.getGlobalBounds().contains(mp)) {
        Sfx::get().play(Sfx::Id::Menu);
        std::vector<MenuItem> items;
        items.push_back({"注销(L)", [this](){
            Sfx::get().play(Sfx::Id::Click);
            // Reset all story progress (re-login to a fresh session)
            auto& story = StoryManager::getInstance();
            story.storyStage          = 1;
            story.isRewardUnlocked   = false;
            story.hasPerfectClear    = false;
            story.hasNormalClear     = false;
            story.rewardWindowCreated = false;
            story.recycleBinUnlocked = false;
            story.hasDocClue         = false;
            story.hasBrowserClue     = false;
            story.archiveOpened      = false;
            story.keyPart2Restored   = false;
            // Close all open windows
            for (auto w : openWindows) w->closeWindow();
            // Reset icon grid positions
            usedSlots.clear();
            for (auto icon : icons) {
                icon->gridSlot = allocateSlot();
                icon->position = slotPosition(icon->gridSlot);
            }
            saveIconPositions();
            // Refresh desktop icon visibility
            syncStoryVisuals();
        }, true});
        items.push_back({"", nullptr, false, true}); // separator
        items.push_back({"关闭计算机(U)", [this](){ renderWin->close(); }, true});
        // Fixed position: always appear above the Start button at left edge
        sf::FloatRect startBounds = startSpr.getGlobalBounds();
        sf::Vector2f menuPos(startBounds.position.x, startBounds.position.y);
        shellMenu.show(menuPos, items);
        return;
    }
    auto vws = visibleTaskWindows();
    for (std::size_t i = 0; i < vws.size(); ++i) {
        if (taskButtonRect(i).contains(mp)) {
            if (vws[i]->isActive && !vws[i]->isMinimized) vws[i]->minimizeWindow();
            else { vws[i]->isMinimized = false; setActiveWindow(vws[i]); }
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
                        // Stationary release: single-click selects; second click within
                        // DOUBLE_CLICK_MS on the same icon counts as double-click.
                        AppIconBase* released = draggingIcon;
                        draggingIcon->highlight = false;
                        draggingIcon = nullptr;
                        iconDragMoved = false;
                        const bool isDouble =
                            lastClickedIcon == released &&
                            clickClock.getElapsedTime().asMilliseconds() < DOUBLE_CLICK_MS;
                        if (isDouble) {
                            released->onDoubleClick();
                            lastClickedIcon = nullptr; // prevent triple-click re-fire
                        } else {
                            Sfx::get().play(Sfx::Id::Click);
                            lastClickedIcon = released;
                            clickClock.restart();
                        }
                    } else {
                        // Snap to nearest free slot
                        AppIconBase* icon = draggingIcon;
                        int oldSlot = icon->gridSlot;
                        int newSlot = nearestFreeSlot(icon->position);
                        if (newSlot < 0) newSlot = oldSlot; // fallback
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
            // 先找到目标窗口（不修改容器），再处理，避免 reverse_iterator 失效导致闪退
            PuzzleWindowBase* targetWin = nullptr;
            PuzzleWindowBase* topModal = nullptr;
            for (auto w2 : openWindows) if (w2->isOpen && !w2->isMinimized && w2->isModal) topModal = w2;
            for (auto it = openWindows.rbegin(); it != openWindows.rend(); ++it) {
                PuzzleWindowBase* w = *it;
                if (!w->isOpen || w->isMinimized) continue;
                if (topModal && topModal != w) continue;
                if (w->isMouseHit(mp)) { targetWin = w; break; }
            }
            if (targetWin) {
                hitWindow = true;
                setActiveWindow(targetWin);  // 现在安全，因为已退出迭代
                if (mb->button == sf::Mouse::Button::Left) {
                    int capHit = targetWin->hitCaptionButton(mp);
                    if (capHit == 0) targetWin->closeWindow();
                    else if (capHit == 1) targetWin->toggleMaximize();
                    else if (capHit == 2) targetWin->minimizeWindow();
                    else if (targetWin->containsTitleBarDrag(mp)) {
                        targetWin->isDragging = true;
                        targetWin->dragOffset = mp - targetWin->position;
                    }
                    else if (!targetWin->isMaximized && mp.x > targetWin->position.x + targetWin->width - 8.f && mp.y > targetWin->position.y + targetWin->height - 8.f) {
                        targetWin->isResizing = true; targetWin->resizeDir = 3;
                    }
                    else if (!targetWin->isMaximized && mp.x > targetWin->position.x + targetWin->width - 8.f) {
                        targetWin->isResizing = true; targetWin->resizeDir = 1;
                    }
                    else if (!targetWin->isMaximized && mp.y > targetWin->position.y + targetWin->height - 8.f) {
                        targetWin->isResizing = true; targetWin->resizeDir = 2;
                    }
                } else if (mb->button == sf::Mouse::Button::Right) {
                    if (targetWin->clientRect().contains(mp)) {
                        std::vector<MenuItem> items;
                        targetWin->buildClientContextMenu(items, mp);
                        if (!items.empty()) shellMenu.show(mp, items);
                    }
                }
                targetWin->handleInput(event);
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
                                for (auto i2 : icons) i2->selected = false;
                                icon->selected = true;
                                // Initiate drag; double-click is decided on release (no clock reset here)
                                draggingIcon   = icon;
                                iconDragOffset = mp - icon->position;
                                iconDragMoved  = false;
                                icon->highlight = true;
                            } else if (mb->button == sf::Mouse::Button::Right) {
                                for (auto i2 : icons) i2->selected = false; icon->selected = true;
                                std::vector<MenuItem> items; icon->buildContextMenu(items);
                                if (!items.empty()) {
                                    Sfx::get().play(Sfx::Id::Menu);
                                    shellMenu.show(mp, items);
                                }
                            }
                            break;
                        }
                    }
                    if (!hitIcon) {
                        for (auto i2 : icons) i2->selected = false;
                        if (mb->button == sf::Mouse::Button::Right) {
                            std::vector<MenuItem> items;
                            // 排列图标（禁用，仅展示）
                            items.push_back({"排列图标(I)", nullptr, false});
                            // 刷新
                            items.push_back({"刷新(E)", [this]() {
                                syncStoryVisuals();
                                for (auto i2 : icons) i2->selected = false;
                                renderWin->clear(C::DesktopBg);
                                if (hasWallpaper) renderWin->draw(wallSpr);
                                renderTaskbar();
                                renderWin->display();
                                sf::sleep(sf::milliseconds(120));
                            }, true});
                            items.push_back({"", nullptr, false, true}); // separator
                            // 新建文件夹（弹提示）
                            items.push_back({"新建文件夹(F)", [this]() {
                                Sfx::get().play(Sfx::Id::Error);
                                addWindow(new MsgBoxWin(
                                    "系统提示",
                                    "操作失败：\n系统权限不足，无法在此位置创建新文件夹。",
                                    {300, 250}, 320, 150, systemFont));
                            }, true});
                            items.push_back({"", nullptr, false, true}); // separator
                            // 桌面右键「属性」打开显示属性（区分于我的电脑的系统属性）
                            items.push_back({"属性(R)", [this]() {
                                if (activateExisting("显示 属性")) return;
                                addWindow(new MsgBoxWin(
                                    "显示 属性",
                                    "主题：Windows XP（经典）\n分辨率：1280 x 800\n色彩：32 位\n\n此工作站的显示设置已被锁定。",
                                    {280, 200}, 320, 200, systemFont));
                            }, true});
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
        sf::Text t = vws[i]->titleText;
        if (appIconTex.getSize().x > 0) {
            sf::Sprite iconSpr(appIconTex);
            iconSpr.setPosition(sf::Vector2f(br.position.x + 4.f, br.position.y + 4.f));
            float scale = 16.f / iconSpr.getTexture().getSize().y;
            iconSpr.setScale(sf::Vector2f(scale, scale));
            renderWin->draw(iconSpr);
            t.setPosition(sf::Vector2f(br.position.x + 4.f + iconSpr.getGlobalBounds().size.x + 4.f, br.position.y + 4.f));
        } else {
            t.setPosition(sf::Vector2f(br.position.x + 10.f, br.position.y + 4.f));
        }
        // Truncate by Unicode code points (not raw UTF-8 bytes)
        sf::String title = t.getString();
        if (title.getSize() > 8) {
            title = title.substring(0, 7) + sf::String("...");
            t.setString(title);
        }
        renderWin->draw(t);
    }
    if (clockText) renderWin->draw(*clockText);
}
void Desktop::renderAll() {
    renderWin->clear(C::DesktopBg); if (hasWallpaper) renderWin->draw(wallSpr);
    for (auto icon : icons) if (icon->visible) icon->render(*renderWin);
    for (auto w : openWindows) w->render(*renderWin);
    renderTaskbar(); shellMenu.render(*renderWin);

    // 完美结局桌面横幅（零窗口、永不闪退的呈现方式）
    auto& story = StoryManager::getInstance();
    if (story.hasPerfectClear) {
        sf::RectangleShape banner(sf::Vector2f(WIN_W, 48.f));
        banner.setPosition(sf::Vector2f(0.f, DESKTOP_H - 48.f - 4.f));
        banner.setFillColor(sf::Color(180, 80, 0, 230));
        renderWin->draw(banner);
        sf::Text bannerTxt(systemFont, U8("★ 完美隐藏结局已达成 ★  无限制 Grok 永久使用权 + 特斯拉 Model S 兑换资格已解锁"), 18u);
        bannerTxt.setFillColor(sf::Color::Yellow);
        bannerTxt.setStyle(sf::Text::Bold);
        sf::FloatRect tb = bannerTxt.getLocalBounds();
        bannerTxt.setPosition(sf::Vector2f((WIN_W - tb.size.x) / 2.f, DESKTOP_H - 48.f + 10.f));
        renderWin->draw(bannerTxt);
    } else if (story.hasNormalClear) {
        sf::RectangleShape banner(sf::Vector2f(WIN_W, 36.f));
        banner.setPosition(sf::Vector2f(0.f, DESKTOP_H - 36.f - 4.f));
        banner.setFillColor(sf::Color(20, 60, 140, 220));
        renderWin->draw(banner);
        sf::Text bannerTxt(systemFont, U8("☆ 普通结局已达成 ☆  无限制 Grok 永久使用权已解锁  （提示：试试隐藏网址 tesla-prize.local）"), 16u);
        bannerTxt.setFillColor(sf::Color::White);
        sf::FloatRect tb = bannerTxt.getLocalBounds();
        bannerTxt.setPosition(sf::Vector2f((WIN_W - tb.size.x) / 2.f, DESKTOP_H - 36.f + 8.f));
        renderWin->draw(bannerTxt);
    }

    renderWin->display();
}
void Desktop::run() {
    // ═══════════════════════════════════════════════════════════════
    // WinXP-style boot animation
    // Phase 0: black  (0.4s)
    // Phase 1: logo   (1.6s) – "Microsoft Windows XP" + "Professional"
    // Phase 2: bar    (2.5s) – smooth buffering progress bar
    // Phase 3: fade   (0.6s) – fade to desktop colour
    // ═══════════════════════════════════════════════════════════════

    sf::Texture bootBgTex;
    sf::Sprite bootBgSpr(bootBgTex);
    bool hasBootBg = false;
    if (bootBgTex.loadFromFile("assets/boot_bg.jpg")) {
        bootBgTex.setSmooth(true);
        bootBgSpr.setTexture(bootBgTex, true);
        float scaleX = WIN_W / bootBgTex.getSize().x;
        float scaleY = WIN_H / bootBgTex.getSize().y;
        float maxScale = std::max(scaleX, scaleY);
        bootBgSpr.setScale(sf::Vector2f(maxScale, maxScale));
        hasBootBg = true;
    }

    sf::Clock bootClock;
    constexpr float phase1Start = 0.4f;
    constexpr float phase2Start = 2.0f;
    constexpr float phase3Start = 4.5f;
    constexpr float phase3End   = 5.1f;

    bool soundStarted = false;

    while (renderWin->isOpen()) {
        while (std::optional<sf::Event> ev = renderWin->pollEvent())
            if (ev->is<sf::Event::Closed>()) { renderWin->close(); return; }

        float t = bootClock.getElapsedTime().asSeconds();
        if (t >= phase3End) break;

        renderWin->clear(sf::Color::Black);
        
        // Only draw the background starting from phase 1, so phase 0 remains a true black screen
        if (t >= phase1Start && hasBootBg) {
            renderWin->draw(bootBgSpr);
        }

        if (t >= phase1Start) {
            if (!soundStarted) {
                Sfx::get().play(Sfx::Id::Boot);
                soundStarted = true;
            }

            sf::Text titleTxt(systemFont, U8("Microsoft Windows XP"), 36u);
            titleTxt.setFillColor(sf::Color::White);
            titleTxt.setStyle(sf::Text::Bold);
            float tw = titleTxt.getLocalBounds().size.x;
            titleTxt.setPosition(sf::Vector2f((WIN_W - tw) / 2.f, WIN_H / 2.f - 70.f));
            renderWin->draw(titleTxt);

            sf::Text sub(systemFont, U8("Professional"), 22u);
            sub.setFillColor(sf::Color(170, 200, 255));
            float sw = sub.getLocalBounds().size.x;
            sub.setPosition(sf::Vector2f((WIN_W - sw) / 2.f, WIN_H / 2.f - 24.f));
            renderWin->draw(sub);

            sf::RectangleShape sep(sf::Vector2f(320.f, 1.f));
            sep.setFillColor(sf::Color(100, 140, 220, 180));
            sep.setPosition(sf::Vector2f((WIN_W - 320.f) / 2.f, WIN_H / 2.f + 10.f));
            renderWin->draw(sep);
        }

        if (t >= phase2Start && t < phase3Start) {
            constexpr float barW = 220.f, barH = 14.f;
            float barX = (WIN_W - barW) / 2.f;
            float barY = WIN_H * 0.72f;

            sf::RectangleShape track(sf::Vector2f(barW, barH));
            track.setPosition(sf::Vector2f(barX, barY));
            track.setFillColor(sf::Color(10, 10, 40, 180));
            track.setOutlineColor(sf::Color(60, 90, 160));
            track.setOutlineThickness(1.f);
            renderWin->draw(track);

            // Smooth buffering style progress bar
            float ratio = (t - phase2Start) / (phase3Start - phase2Start);
            if (ratio < 0.f) ratio = 0.f;
            if (ratio > 1.f) ratio = 1.f;
            
            // Apply a slight easing to make it feel more "real"
            float easeRatio = 1.f - std::pow(1.f - ratio, 3.f);

            sf::RectangleShape fill(sf::Vector2f(barW * easeRatio, barH));
            fill.setPosition(sf::Vector2f(barX, barY));
            fill.setFillColor(sf::Color(58, 110, 220));
            renderWin->draw(fill);

            sf::Text copy(systemFont, U8("Copyright \u00a9 2001-2008 Microsoft Corporation."), 11u);
            copy.setFillColor(sf::Color(160, 160, 160));
            copy.setPosition(sf::Vector2f((WIN_W - copy.getLocalBounds().size.x) / 2.f, barY + barH + 14.f));
            renderWin->draw(copy);
        }

        if (t >= phase3Start) {
            float alpha = (t - phase3Start) / (phase3End - phase3Start);
            if (alpha > 1.f) alpha = 1.f;
            uint8_t a = static_cast<uint8_t>(alpha * 255.f);
            sf::RectangleShape overlay(sf::Vector2f(WIN_W, WIN_H));
            overlay.setFillColor(sf::Color(C::DesktopBg.r, C::DesktopBg.g, C::DesktopBg.b, a));
            renderWin->draw(overlay);
        }

        renderWin->display();
    }

    // ── Main game loop ──
    while (renderWin->isOpen()) {
        processEvents();
        // 安全地把事件回调中 queue 的窗口真正加入 openWindows
        flushPendingWindows();

        // 安全创建结局窗口（完全脱离事件回调，杜绝闪退）
        {
            auto& story = StoryManager::getInstance();

            // 完美结局：安全地改造已打开的浏览器 + 播放音效（主循环中，零风险）
            if (story.hasPerfectClear) {
                static bool perfectSfxPlayed = false;
                if (!perfectSfxPlayed) {
                    Sfx::get().play(Sfx::Id::Perfect);
                    perfectSfxPlayed = true;
                }
                for (auto* w : openWindows) {
                    if (w && w->isOpen) {
                        if (auto* bw = dynamic_cast<BrowserWindow*>(w)) {
                            if (!bw->perfectVictory) {
                                bw->showPerfectVictory();
                                setActiveWindow(bw);
                            }
                            break;
                        }
                    }
                }
            }

            // 普通结局：创建奖励窗口（只创建一次）
            if (story.hasNormalClear && !story.hasPerfectClear && !story.rewardWindowCreated) {
                for (auto* w : openWindows) {
                    if (w->isOpen && w->title == "系统奖励") w->closeWindow();
                }
                addWindow(new RewardAnimWindow(
                    {WIN_W / 2 - 250.f, WIN_H / 2 - 200.f}, 500.f, 400.f, systemFont));
                story.rewardWindowCreated = true;
            }
        }

        auto it = std::remove_if(openWindows.begin(), openWindows.end(), [](PuzzleWindowBase* w){ return !w->isOpen; });
        for (auto i = it; i != openWindows.end(); ++i) delete *i;
        openWindows.erase(it, openWindows.end());
        updateTaskbar(); renderAll();
    }
}
