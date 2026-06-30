#pragma once
#include <SFML/Graphics.hpp>
#include <functional>
#include <string>
#include <vector>

// ── Constants ──────────────────────────────────────────────────
constexpr float WIN_W          = 1600.f;
constexpr float WIN_H          = 1200.f;
constexpr float TASKBAR_H      = 64.f;
constexpr float DESKTOP_H      = WIN_H - TASKBAR_H;
constexpr float ICON_W         = 64.f;
constexpr float ICON_H         = 64.f;
constexpr float ICON_TEXT_H    = 40.f;
constexpr int   DOUBLE_CLICK_MS = 600;

// ── Color palette & Pixel Art ──────────────────────────────────
namespace C {
    // Cyberpunk pixel art color palette
    inline const sf::Color DesktopBg      { 18,  12,  33 }; // Deep space/violet dark
    inline const sf::Color TaskbarBg      { 30,  18,  54 }; // Dark purple
    inline const sf::Color WindowTitle    { 85,  26, 139 }; // Deep violet titlebar
    inline const sf::Color WindowBody     { 24,  16,  42 }; // Sunken dark violet body
    inline const sf::Color WinTitleText   { 255,  85, 219 }; // Hot neon pink
    inline const sf::Color TextWhite      { 235, 235, 255 }; // Light neon blue-white
    inline const sf::Color CloseBtn       { 180,  20,  70 }; // Neon crimson close button
    inline const sf::Color CloseBtnHov    { 240,  40, 110 }; // Active close button
    inline const sf::Color MenuBg         { 20,  12,  38, 250 }; // Context menu background
    inline const sf::Color MenuHover      { 255,   0, 127 }; // Neon pink selection
    inline const sf::Color SelectBorder   { 0,  255, 240 }; // Neon cyan selection border
    inline const sf::Color IconLabel      { 0,  255, 200 }; // Neon mint label

    // Bevel highlights & shadows for pixel 3D style
    inline const sf::Color BevelLight     { 255, 110, 220 }; // Cyberpunk neon pink highlight
    inline const sf::Color BevelMedium    { 90,  30, 120 }; // Midtone border
    inline const sf::Color BevelDark      { 10,   5,  20 }; // Dark shadows

    // Custom 16x16 pixel-art patterns
    inline const char* const NotepadIconPattern[16] = {
        "................",
        "..KKKKKKKKKKKK..",
        ".KWWWWWWWWWWWWK.",
        ".KWYYYYYYYYYYWK.",
        ".KWKKKKKKKKKKWK.",
        ".KWKWWWWWWWWKWK.",
        ".KWKWWWWWWWWKWK.",
        ".KWKCCCCCCCCCWK.",
        ".KWKWWWWWWWWKWK.",
        ".KWKCCCCCCCCCWK.",
        ".KWKWWWWWWWWKWK.",
        ".KWKCCCCCCCCCWK.",
        ".KWKWWWWWWWWKWK.",
        ".KWKKKKKKKKKKWK.",
        ".KWWWWWWWWWWWWK.",
        "..KKKKKKKKKKKK.."
    };

    inline const char* const CalculatorIconPattern[16] = {
        "................",
        "..KKKKKKKKKKKK..",
        ".KBBBBBBBBBBBBK.",
        ".KBKKKKKKKKKKBK.",
        ".KBKggggggggKBK.",
        ".KBKggggggggKBK.",
        ".KBKKKKKKKKKKBK.",
        ".KBKWWKWWKWWKBK.",
        ".KBKWWKWWKWWKBK.",
        ".KBKWWKWWKWWKBK.",
        ".KBKWWKWWKWWKBK.",
        ".KBKWWKWWKRRKBK.",
        ".KBKKKKKKKKKKBK.",
        ".KBBBBBBBBBBBBK.",
        "..KKKKKKKKKKKK..",
        "................"
    };

    inline const char* const FolderIconPattern[16] = {
        "................",
        "..KKKKKKK.......",
        ".KOOOOOOOK......",
        ".KOYYYYYYOKKKKK.",
        ".KOYYYYYYYYYYYOK",
        ".KOYYYYYYYYYYYOK",
        ".KOOOOOOOOOOOOOK",
        ".KOOOOOOOOOOOOOK",
        ".KOOOOOOOOOOOOOK",
        ".KOOOOOOOOOOOOOK",
        ".KOOOOOOOOOOOOOK",
        ".KOOOOOOOOOOOOOK",
        ".KOOOOOOOOOOOOOK",
        ".KOOOOOOOOOOOOOK",
        "..KKKKKKKKKKKKK.",
        "................"
    };

    inline const char* const RecycleIconPattern[16] = {
        "................",
        "...KKKKKKKKKK...",
        "..KWWWWWWWWWWK..",
        "..KKKKKKKKKKKK..",
        "...KGGGGGGGGK...",
        "...KGWWWWWWGK...",
        "...KGGGGGGGGK...",
        "...KGGggGGGGK...",
        "...KGggggGGGK...",
        "...KGGggGGGGK...",
        "...KGGGGGGGGK...",
        "...KGWWWWWWGK...",
        "...KGGGGGGGGK...",
        "....KKKKKKKK....",
        "................",
        "................"
    };

    inline const char* const CursorPattern[16] = {
        "K...............",
        "KK..............",
        "KWK.............",
        "KWWK............",
        "KWWWK...........",
        "KWWWWK..........",
        "KWWWWWK.........",
        "KWWWWWWK........",
        "KWWWWWWWK.......",
        "KWWWWWWWWK......",
        "KWWWWWKKKKK.....",
        "KWWKWK..........",
        "KWK.KWK.........",
        "KK...KWK........",
        "......KWK.......",
        ".......KK......."
    };

    inline sf::Color charToColor(char c) {
        switch (c) {
            case 'W': return sf::Color::White;
            case 'K': return sf::Color::Black;
            case 'Y': return sf::Color(255, 230, 100);  // Neon Yellow
            case 'C': return sf::Color(0, 255, 240);    // Neon Cyan
            case 'B': return sf::Color(0, 100, 220);    // Cyberpunk Blue
            case 'g': return sf::Color(50, 220, 100);   // Neon Green
            case 'R': return sf::Color(255, 0, 100);    // Neon Magenta/Red
            case 'O': return sf::Color(240, 120, 30);   // Electric Orange
            case 'G': return sf::Color(120, 120, 140);  // Retro Gray
            default: return sf::Color::Transparent;
        }
    }
}

// ── Rendering Helper Functions ─────────────────────────────────
void drawPixelPattern(sf::RenderWindow& win, sf::Vector2f pos, const char* const pattern[16], float pixelSize);
void drawDoubleBevel(sf::RenderWindow& win, sf::FloatRect rect, bool raised);

// ── DesktopIcon ────────────────────────────────────────────────
class DesktopIcon {
public:
    sf::Vector2f pos;
    sf::Vector2f size;
    std::string  label;
    sf::RectangleShape shape;
    sf::Text     labelText;
    std::string  appType;
    bool         selected = false;

    DesktopIcon(sf::Vector2f p, sf::Vector2f sz, const std::string& lbl,
                const std::string& app, const sf::Font& font);

    bool contains(sf::Vector2i mousePos) const;
    void draw(sf::RenderWindow& win, sf::Vector2f offset = {0.f, 0.f}) const;
};

// ── MenuItem + ContextMenu ─────────────────────────────────────
struct MenuItem {
    std::string              label;
    std::function<void()>    action;
};

class ContextMenu {
public:
    bool                  visible = false;
    sf::Vector2f          position;
    std::vector<MenuItem> items;
    sf::RectangleShape    background;
    std::vector<sf::Text> itemTexts;
    float                 itemHeight = 26.f;
    float                 width      = 170.f;
    int                   hoveredIdx = -1;

    ContextMenu(const sf::Font& font);

    void show(sf::Vector2f pos, const std::vector<MenuItem>& newItems, const sf::Font& font);
    void hide();
    bool contains(sf::Vector2i mousePos) const;
    int  itemAt(sf::Vector2i mousePos) const;
    void updateHover(sf::Vector2i mousePos);
    void draw(sf::RenderWindow& win) const;
};

enum class ResizeDir { None, Right, Bottom, BottomRight };

// ── GameWindow (an open application window) ────────────────────
class GameWindow {
public:
    sf::RectangleShape titleBar;
    sf::RectangleShape body;
    sf::RectangleShape closeBtn;
    sf::RectangleShape maxBtn;
    sf::RectangleShape minBtn;
    sf::Text           titleText;
    sf::Text           bodyText;
    sf::Text           bodyText2;        // second line of content
    std::string        appType;
    bool               isDragging  = false;
    sf::Vector2f       dragOffset;
    bool               isOpen      = true;
    bool               isMinimized = false;
    int                zOrder      = 0;

    // Resizing state
    bool               isResizing  = false;
    ResizeDir          resizeDir   = ResizeDir::None;
    sf::Vector2f       resizeStartSize;
    sf::Vector2i       resizeStartMouse;

    // Maximized state
    bool               isMaximized = false;
    sf::Vector2f       preMaxPos;
    sf::Vector2f       preMaxSize;

    // Puzzle state
    int         puzzleStep  = 0;         // 0 = unsolved, 1 = intermediate, 2 = solved
    std::string userInput;              // what the player has typed
    bool        puzzleSolved = false;

    GameWindow(const sf::Font& font, const std::string& type);

    void setPos(sf::Vector2f p);
    void setSize(sf::Vector2f sz);
    sf::FloatRect bounds() const;
    sf::FloatRect titleBarBounds() const;
    sf::FloatRect closeBtnBounds() const;
    sf::FloatRect maxBtnBounds() const;
    sf::FloatRect minBtnBounds() const;
    
    ResizeDir hitResizeBorder(sf::Vector2i mousePos) const;
    bool containsTitleBar(sf::Vector2i mousePos) const;
    bool containsCloseBtn(sf::Vector2i mousePos) const;
    bool containsMaxBtn(sf::Vector2i mousePos) const;
    bool containsMinBtn(sf::Vector2i mousePos) const;
    
    void handleTextInput(uint32_t unicode);
    void updateContent();   // refresh body text based on puzzleStep / userInput
    void draw(sf::RenderWindow& win) const;
};

// ── Taskbar ────────────────────────────────────────────────────
class Taskbar {
public:
    float              h = TASKBAR_H;
    sf::RectangleShape bg;
    sf::Text           clockText;
    sf::Clock          clock;
    // window buttons drawn as small rectangles + labels
    std::vector<sf::RectangleShape> winBtns;
    std::vector<sf::Text>           winBtnLabels;

    Taskbar(const sf::Font& font);

    void update(std::vector<GameWindow>& windows, const sf::Font& font);
    int  windowBtnAt(sf::Vector2i mousePos) const;
    void draw(sf::RenderWindow& win) const;
};

// ── Free functions ─────────────────────────────────────────────
std::string timeString();
sf::Vector2f clampWindowPos(const GameWindow& w, sf::Vector2f desired);
