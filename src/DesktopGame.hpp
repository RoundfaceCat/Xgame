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
    // Windows XP / Classic 3D color palette
    inline const sf::Color DesktopBg      { 61,  130, 226 }; // Classic XP Blue
    inline const sf::Color TaskbarBg      { 16,  61,  181 }; // XP Blue Dark
    inline const sf::Color WindowTitle    { 25,  82,  171 }; // XP Titlebar Blue
    inline const sf::Color WindowBody     { 236, 233, 216 }; // XP Window Body Cream-gray
    inline const sf::Color WinTitleText   { 255, 255, 255 }; // White title text
    inline const sf::Color TextWhite      { 0,   0,   0   }; // Black window body text
    inline const sf::Color CloseBtn       { 224, 76,  44  }; // XP Orange-red close button
    inline const sf::Color CloseBtnHov    { 240, 100, 70  }; // Hover close button
    inline const sf::Color MenuBg         { 236, 233, 216 }; // XP Menu Bg
    inline const sf::Color MenuHover      { 49,  106, 197 }; // XP Menu Hover Blue
    inline const sf::Color SelectBorder   { 49,  106, 197 }; // XP Selection blue
    inline const sf::Color IconLabel      { 255, 255, 255 }; // White label text

    // Windows XP Luna Theme Colors
    inline const sf::Color XpBlueLight    { 36,  94,  219 }; // Top of taskbar gradient
    inline const sf::Color XpBlueDark     { 16,  61,  181 }; // Bottom of taskbar gradient
    inline const sf::Color XpBlueTray     { 9,   46,  140 }; // System tray / clock background
    inline const sf::Color XpMenuBg       { 236, 233, 216 }; // Cream-gray context menu background
    inline const sf::Color XpMenuHover    { 49,  106, 197 }; // Windows XP classic menu selection blue
    inline const sf::Color XpMenuBorder   { 172, 168, 153 }; // Classic menu border gray
    inline const sf::Color XpMenuText     { 0,   0,   0   }; // Black text for XP menu

    // Bevel highlights & shadows for classic Windows 3D style
    inline const sf::Color BevelLight     { 255, 255, 255 }; // White highlight
    inline const sf::Color BevelMedium    { 212, 208, 200 }; // Light gray midtone
    inline const sf::Color BevelDark      { 113, 111, 100 }; // Dark gray shadow

    // Windows XP sizing cursor patterns
    inline const char* const CursorResizeH[16] = {
        "................",
        "................",
        "................",
        "....K.......K...",
        "...KK.......KK..",
        "..KWW.......WWK.",
        ".KWWWWKKKKKWWWWK",
        "KWWWWWWWWWWWWWWK",
        ".KWWWWKKKKKWWWWK",
        "..KWW.......WWK.",
        "...KK.......KK..",
        "....K.......K...",
        "................",
        "................",
        "................",
        "................"
    };

    inline const char* const CursorResizeV[16] = {
        "......K.........",
        ".....KKK........",
        "....KWWWK.......",
        "....KWWWK.......",
        ".....KKK........",
        ".....KWK........",
        ".....KWK........",
        ".....KWK........",
        ".....KWK........",
        ".....KWK........",
        ".....KKK........",
        "....KWWWK.......",
        "....KWWWK.......",
        ".....KKK........",
        "......K.........",
        "................"
    };

    inline const char* const CursorResizeDiag[16] = {
        "KKKKK...........",
        "KWWWWK..........",
        "KWWKK...........",
        "KWK.KK..........",
        "KWK..KK.........",
        ".KK...KK........",
        ".......KK.......",
        "........KK......",
        ".........KK...KK",
        "..........KK..KW",
        "...........KK.KW",
        "..........KKKWWK",
        ".........KWWWWK.",
        "..........KKKKK.",
        "................",
        "................"
    };

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
            case 'Y': return sf::Color(245, 200, 110);  // XP Folder Yellow
            case 'C': return sf::Color(120, 160, 215);  // Notepad Line Blue
            case 'B': return sf::Color(190, 200, 215);  // Calculator Silver-Blue
            case 'g': return sf::Color(50, 150, 70);    // Soft LCD Green
            case 'R': return sf::Color(200, 50, 50);    // Calculator C Button Red
            case 'O': return sf::Color(210, 130, 40);   // Folder Shadow Orange
            case 'G': return sf::Color(160, 160, 160);  // Recycle Bin Gray
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
    const sf::Texture* texturePtr = nullptr;

    DesktopIcon(sf::Vector2f p, sf::Vector2f sz, const std::string& lbl,
                const std::string& app, const sf::Font& font, const sf::Texture* tex = nullptr);

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
    const sf::Texture* startTexPtr = nullptr;

    Taskbar(const sf::Font& font, const sf::Texture* startTex = nullptr);

    void update(std::vector<GameWindow>& windows, const sf::Font& font);
    int  windowBtnAt(sf::Vector2i mousePos) const;
    void draw(sf::RenderWindow& win) const;
};

// ── Free functions ─────────────────────────────────────────────
std::string timeString();
sf::Vector2f clampWindowPos(const GameWindow& w, sf::Vector2f desired);
