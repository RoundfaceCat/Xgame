#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <string>
#include <vector>
#include <functional>
#include <algorithm>
#include <optional>
#include <set>
#include <fstream>
#include <array>
#include <memory>
#include <cmath>

// ── Shell metrics (simulating Win32 / XP metrics) ───────────────
constexpr float WIN_W            = 1280.f;
constexpr float WIN_H            = 800.f;
constexpr float TASKBAR_H        = 40.f;
constexpr float DESKTOP_H        = WIN_H - TASKBAR_H;
constexpr float ICON_W           = 84.f;
constexpr float ICON_H           = 84.f;
constexpr float ICON_CELL_H      = 70.f;
constexpr float GRID_COL_W       = 100.f;   // grid cell width
constexpr float GRID_ROW_H       = 100.f;   // grid cell height
constexpr float GRID_MARGIN_X    = 10.f;    // left margin
constexpr float GRID_MARGIN_Y    = 10.f;    // top margin
constexpr int   GRID_COLS        = 5;       // default icon columns (left side)
constexpr std::string_view ICON_POS_FILE = "icon_positions.ini";
constexpr float TITLEBAR_H       = 22.f;
constexpr float BORDER           = 3.f;
constexpr float CAPTION_BTN      = 16.f;
constexpr int   DOUBLE_CLICK_MS  = 500;
constexpr float TASK_BTN_W       = 160.f;

// Windows XP Luna / Classic palette
namespace C {
    inline const sf::Color DesktopBg      { 58,  110, 165 };
    inline const sf::Color TaskbarBg      { 13,  50,  135 };
    inline const sf::Color TaskbarBtn     { 30,  70,  160 };
    inline const sf::Color TaskbarBtnAct  { 50,  95,  190 };
    inline const sf::Color StartGreen     { 60,  140, 60  };
    inline const sf::Color TitleActive    { 10,  36,  106 };
    inline const sf::Color TitleInactive  { 128, 128, 128 };
    inline const sf::Color WindowBody     { 236, 233, 216 };
    inline const sf::Color ClientWhite    { 255, 255, 255 };
    inline const sf::Color TextBlack      { 0,   0,   0   };
    inline const sf::Color TextWhite      { 255, 255, 255 };
    inline const sf::Color CloseBtn       { 200, 60,  40  };
    inline const sf::Color MenuBg         { 255, 255, 255 };
    inline const sf::Color MenuHover      { 49,  106, 197 };
    inline const sf::Color SelectBlue     { 10,  36,  106 };
    inline const sf::Color SelectIcon     { 10,  36,  106, 90 };
    inline const sf::Color BevelLight     { 255, 255, 255 };
    inline const sf::Color BevelMedium    { 212, 208, 200 };
    inline const sf::Color BevelDark      { 113, 111, 100 };
    inline const sf::Color StatusBar      { 236, 233, 216 };
    inline const sf::Color HighlightGuide { 255, 220, 80  };
}

void drawDoubleBevel(sf::RenderWindow& win, sf::FloatRect rect, bool raised);
void drawCaptionButton(sf::RenderWindow& win, sf::FloatRect rect,
                       const sf::Font& font, const std::string& glyph,
                       sf::Color face, bool pressed = false);

// ── Procedural XP-style UI sound effects ───────────────────────
class Sfx {
public:
    enum class Id {
        Click = 0,   // soft UI click / select
        Open,        // open window / double-click
        Menu,        // context / start menu
        Error,       // wrong password / access denied
        Ok,          // password accepted
        Unlock,      // archive / recycle unlocked
        Restore,     // recycle restore
        Notify,      // clue / stage advance
        Ending,      // normal clear
        Perfect,     // perfect clear
        Count
    };

    static Sfx& get();
    void init();
    void play(Id id);
    void setEnabled(bool on) { enabled = on; }
    bool isEnabled() const { return enabled; }

private:
    Sfx() = default;
    bool enabled = true;
    bool ready   = false;
    std::array<sf::SoundBuffer, static_cast<std::size_t>(Id::Count)> buffers{};
    // Pool so short sounds can overlap
    static constexpr std::size_t kVoices = 8;
    std::array<std::unique_ptr<sf::Sound>, kVoices> voices{};
    std::size_t nextVoice = 0;

    static sf::SoundBuffer makeBeep(float freqHz, float durationSec,
                                    float volume = 0.45f, bool decay = true);
    static sf::SoundBuffer makeChord(std::initializer_list<float> freqs,
                                     float durationSec, float volume = 0.4f);
};

class Desktop;

// ═══════════════════════════════════════════════════════════════
// 1. StoryManager - 剧情管理器（单例，业务数据解耦）
// ═══════════════════════════════════════════════════════════════
class StoryManager {
private:
    StoryManager() = default;
public:
    static StoryManager& getInstance() {
        static StoryManager instance;
        return instance;
    }
    StoryManager(const StoryManager&) = delete;
    StoryManager& operator=(const StoryManager&) = delete;

    int         storyStage         = 1;
    std::string keyPart1           = "20230420";
    std::string keyPart2           = "2012";
    std::string fullKey            = "202304202012";
    bool        isRewardUnlocked   = false;
    bool        hasPerfectClear    = false;
    bool        hasNormalClear     = false;
    bool        recycleBinUnlocked = false; // 加密档案室解锁标志
    bool        hasDocClue         = false;
    bool        hasBrowserClue     = false;
    bool        archiveOpened      = false;
    bool        keyPart2Restored   = false;

    bool tryUnlockArchive(const std::string& input);
    bool tryPerfectKey(const std::string& input);
    void markDocClueRead();
    void markBrowserClueSeen();
    void unlockRecycleBin();
    void onKeyPart2Restored();
    void unlockRewardExe();
    void triggerNormalEnding();
    void triggerPerfectEnding();
    void advanceStage(int stage);
};

// ═══════════════════════════════════════════════════════════════
// 2. 基础 UI 组件
// ═══════════════════════════════════════════════════════════════
class UIComponent {
public:
    sf::Vector2f position;
    float width   = 0.f;
    float height  = 0.f;
    bool  visible = true;

    UIComponent(sf::Vector2f p, float w, float h) : position(p), width(w), height(h) {}
    virtual ~UIComponent() = default;

    virtual void render(sf::RenderWindow& win) = 0;
    virtual bool isMouseHit(sf::Vector2f mousePos) = 0;
    sf::FloatRect bounds() const { return {position, {width, height}}; }
};

struct MenuItem {
    std::string           label;
    std::function<void()> action;
    bool                  enabled   = true;
    bool                  separator = false;
    bool                  isDefault = false;
};

class ContextMenu : public UIComponent {
public:
    std::vector<MenuItem> items;
    sf::RectangleShape    background;
    sf::RectangleShape    shadow;
    std::vector<sf::Text> itemTexts;
    float                 itemHeight = 22.f;
    int                   hoveredIdx = -1;
    const sf::Font*       menuFont   = nullptr;
    bool                  open       = false;

    ContextMenu();
    void setFont(const sf::Font& font);
    void show(sf::Vector2f p, const std::vector<MenuItem>& newItems);
    void hide();
    int  itemAt(sf::Vector2f mp) const;
    void updateHover(sf::Vector2f mp);
    bool tryClick(sf::Vector2f mp);

    void render(sf::RenderWindow& win) override;
    bool isMouseHit(sf::Vector2f mousePos) override;
};

// ═══════════════════════════════════════════════════════════════
// 3. FileObject 文件抽象链 (统一文件打开逻辑)
// ═══════════════════════════════════════════════════════════════
class FileObject {
public:
    std::string name;
    std::string iconType; // 用于绘制文件类型图标 "doc" / "txt" / "locked"
    
    FileObject(const std::string& n, const std::string& type) : name(n), iconType(type) {}
    virtual ~FileObject() = default;
    
    virtual void open() = 0;
};

class TextFile : public FileObject {
public:
    std::string content;
    TextFile(const std::string& n, const std::string& c, const std::string& type = "txt") 
        : FileObject(n, type), content(c) {}
    void open() override;
};

class LockedFile : public FileObject {
public:
    std::string content;
    LockedFile(const std::string& n, const std::string& c = "")
        : FileObject(n, "locked"), content(c) {}
    void open() override;
};

class DeletedFile : public FileObject {
public:
    std::string content;
    DeletedFile(const std::string& n, const std::string& c) : FileObject(n, "txt"), content(c) {}
    void open() override;
};

class ExeProgramFile : public FileObject {
public:
    ExeProgramFile(const std::string& n) : FileObject(n, "exe") {}
    void open() override;
};

// ═══════════════════════════════════════════════════════════════
// 4. PuzzleWindowBase 窗口抽象链 (通用外壳复用)
// ═══════════════════════════════════════════════════════════════
class PuzzleWindowBase : public UIComponent {
public:
    std::string  title;
    bool         isDragging   = false;
    bool         isResizing   = false;
    int          resizeDir    = 0; // 1:R, 2:B, 3:BR
    sf::Vector2f dragOffset;
    bool         isOpen       = true;
    bool         isMinimized  = false;
    bool         isMaximized  = false;
    bool         isActive     = false;  
    bool         canMinimize  = true;
    bool         canMaximize  = true;
    bool         isModal      = false;  

    sf::Vector2f restorePos;
    float        restoreW = 0.f;
    float        restoreH = 0.f;

    sf::RectangleShape frame;
    sf::RectangleShape titleBar;
    sf::Text           titleText;
    const sf::Font*    uiFont = nullptr;

    PuzzleWindowBase(const std::string& t, sf::Vector2f p, float w, float h, const sf::Font& font);

    sf::FloatRect titleBarRect() const;
    sf::FloatRect closeBtnRect() const;
    sf::FloatRect maxBtnRect() const;
    sf::FloatRect minBtnRect() const;
    sf::FloatRect clientRect() const;

    void dragUpdate(sf::Vector2f mousePos);
    void closeWindow();
    void minimizeWindow();
    void toggleMaximize();
    void restoreFromTaskbar();

    int hitCaptionButton(sf::Vector2f mp) const;
    bool containsTitleBarDrag(sf::Vector2f mp) const;

    void renderFrame(sf::RenderWindow& win);
    void render(sf::RenderWindow& win) override;
    bool isMouseHit(sf::Vector2f mousePos) override;

    virtual void renderContent(sf::RenderWindow& win) = 0;
    virtual void handleInput(const sf::Event& event) = 0;
    virtual void buildClientContextMenu(std::vector<MenuItem>& /*out*/, sf::Vector2f /*mp*/) {}
};

class NotepadWindow : public PuzzleWindowBase {
public:
    sf::Text    docText;
    std::string textContent;

    NotepadWindow(const std::string& fileTitle, const std::string& content,
                  sf::Vector2f p, float w, float h, const sf::Font& font);
    void renderContent(sf::RenderWindow& win) override;
    void handleInput(const sf::Event& event) override;
};

class BrowserWindow : public PuzzleWindowBase {
public:
    sf::Text           addrText;
    sf::Text           contentText;
    sf::Text           statusText;
    std::string        addressInput;
    sf::RectangleShape addrBar;
    sf::RectangleShape goBtn;
    bool               showSecretPage = false;
    bool               editingAddr    = false;
    const sf::Font*    winFont = nullptr;
    sf::Clock          blinkClock;

    BrowserWindow(sf::Vector2f p, float w, float h, const sf::Font& font);
    void renderContent(sf::RenderWindow& win) override;
    void handleInput(const sf::Event& event) override;
    void navigateToAddress();
};

class FileListWin : public PuzzleWindowBase {
public:
    std::vector<FileObject*> files;
    bool                  isRecycleBinMode = false;
    const sf::Font*       winFont = nullptr;
    int                   selectedIdx = -1;
    sf::Clock             listClickClock;
    std::string           folderPath;

    sf::RectangleShape restoreBtn;
    sf::Text           restoreBtnText;
    
    sf::Texture           texFolder;
    sf::Texture           texFile;
    sf::Texture           texExe;
    sf::Texture           texComputer;

    FileListWin(const std::string& winTitle, bool recycleBin,
                sf::Vector2f p, float w, float h, const sf::Font& font);
    ~FileListWin();
    void addFile(FileObject* file);
    void renderContent(sf::RenderWindow& win) override;
    void handleInput(const sf::Event& event) override;
    void buildClientContextMenu(std::vector<MenuItem>& out, sf::Vector2f mp) override;
    void refreshFiles();
    void openSelectedFile();
    void restoreSelectedFile();
    float listTopY() const;
};

class SystemPropWin : public PuzzleWindowBase {
public:
    sf::RectangleShape secretBtn;
    sf::RectangleShape okBtn;
    sf::RectangleShape cancelBtn;
    sf::Text           secretBtnText;
    sf::Text           okText;
    sf::Text           cancelText;
    const sf::Font*    winFont = nullptr;

    SystemPropWin(sf::Vector2f p, float w, float h, const sf::Font& font);
    void renderContent(sf::RenderWindow& win) override;
    void handleInput(const sf::Event& event) override;
};

class PasswordDialog : public PuzzleWindowBase {
public:
    sf::Text           promptText;
    sf::Text           inputText;
    std::string        typedPassword;
    sf::RectangleShape inputField;
    sf::RectangleShape okBtn;
    sf::RectangleShape cancelBtn;
    sf::Text           okText;
    sf::Text           cancelText;
    const sf::Font*    winFont = nullptr;
    bool               hasError = false;
    int                mode     = 0; // 0 档案室 / 1 完美结局
    sf::Clock          blinkClock;

    PasswordDialog(sf::Vector2f p, float w, float h,
                   const sf::Font& font, int pwdMode);
    void renderContent(sf::RenderWindow& win) override;
    void handleInput(const sf::Event& event) override;
    void submitPassword();
};

class RewardAnimWindow : public PuzzleWindowBase {
public:
    sf::Text           congratsText;
    sf::RectangleShape teslaFrame;
    sf::Clock          animationClock;

    RewardAnimWindow(sf::Vector2f p, float w, float h, const sf::Font& font);
    void renderContent(sf::RenderWindow& win) override;
    void handleInput(const sf::Event& event) override;
};

class MsgBoxWin : public PuzzleWindowBase {
public:
    sf::Text           msgText;
    sf::RectangleShape okBtn;
    sf::Text           okBtnText;

    MsgBoxWin(const std::string& boxTitle, const std::string& message,
              sf::Vector2f p, float w, float h, const sf::Font& font);
    void renderContent(sf::RenderWindow& win) override;
    void handleInput(const sf::Event& event) override;
};

// ═══════════════════════════════════════════════════════════════
// 5. AppIconBase 图标抽象链 (一个图标一个类)
// ═══════════════════════════════════════════════════════════════
class AppIconBase : public UIComponent {
public:
    sf::Texture iconTex;
    sf::Sprite  iconSpr;
    std::string label;
    sf::Text    labelText;
    bool        selected  = false;
    bool        highlight = false;
    int         gridSlot  = -1;   // which grid slot this icon occupies (-1 = unassigned)
    std::string iconId;           // stable identifier for position saving

    AppIconBase(sf::Vector2f p, const std::string& lbl,
                const std::string& texPath, const sf::Font& font);

    virtual void onDoubleClick() = 0;
    virtual void buildContextMenu(std::vector<MenuItem>& out);

    void render(sf::RenderWindow& win) override;
    bool isMouseHit(sf::Vector2f mousePos) override;
};

class MyDocumentsIcon : public AppIconBase {
public:
    MyDocumentsIcon(sf::Vector2f p, const std::string& texPath, const sf::Font& font);
    void onDoubleClick() override;
    void buildContextMenu(std::vector<MenuItem>& out) override;
};

class IEBrowserIcon : public AppIconBase {
public:
    IEBrowserIcon(sf::Vector2f p, const std::string& texPath, const sf::Font& font);
    void onDoubleClick() override;
};

class RecycleBinIcon : public AppIconBase {
public:
    RecycleBinIcon(sf::Vector2f p, const std::string& texPath, const sf::Font& font);
    void onDoubleClick() override;
};

class SecretFolderIcon : public AppIconBase {
public:
    SecretFolderIcon(sf::Vector2f p, const std::string& texPath, const sf::Font& font);
    void onDoubleClick() override;
};

class TeslaExeIcon : public AppIconBase {
public:
    TeslaExeIcon(sf::Vector2f p, const std::string& texPath, const sf::Font& font);
    void onDoubleClick() override;
};

class MyComputerIcon : public AppIconBase {
public:
    MyComputerIcon(sf::Vector2f p, const std::string& texPath, const sf::Font& font);
    void onDoubleClick() override;
    void buildContextMenu(std::vector<MenuItem>& out) override;
};

// ═══════════════════════════════════════════════════════════════
// 6. Desktop - 桌面总控
// ═══════════════════════════════════════════════════════════════
class Desktop {
public:
    std::vector<AppIconBase*> icons;
    std::vector<PuzzleWindowBase*>  openWindows;

    sf::RenderWindow* renderWin = nullptr;
    sf::Font          systemFont;
    sf::Clock         clickClock;           // time since last completed single-click on an icon
    AppIconBase*      lastClickedIcon = nullptr;

    // Icon drag state
    AppIconBase*      draggingIcon    = nullptr;
    sf::Vector2f      iconDragOffset;
    bool              iconDragMoved   = false;

    ContextMenu shellMenu;

    sf::RectangleShape taskbarBg;
    sf::Texture        startTex;
    sf::Sprite         startSpr;
    sf::Texture        wallTex;
    sf::Sprite         wallSpr;
    bool               hasWallpaper = false;
    std::optional<sf::Text> clockText;
    std::optional<sf::Text> startLabel;
    sf::Clock          animClock;

    TeslaExeIcon*     rewardIcon  = nullptr;
    RecycleBinIcon*   recycleIcon = nullptr;
    SecretFolderIcon* secretIcon  = nullptr;

    // Grid slot management
    std::set<int>  usedSlots;
    int  allocateSlot();
    sf::Vector2f slotPosition(int slot) const;
    int  nearestFreeSlot(sf::Vector2f pos) const;
    void saveIconPositions() const;
    void loadIconPositions();

    Desktop();
    ~Desktop();
    void init(sf::RenderWindow& win);
    void run();

    void addWindow(PuzzleWindowBase* win);
    void bringToFront(PuzzleWindowBase* win);
    void setActiveWindow(PuzzleWindowBase* win);
    PuzzleWindowBase* topWindow() const;
    PuzzleWindowBase* findWindowByTitle(const std::string& title) const;
    bool activateExisting(const std::string& title);

    void syncStoryVisuals();
    void hideShellMenu();

private:
    void processEvents();
    void renderAll();
    void updateTaskbar();
    void renderTaskbar();
    void handleTaskbarClick(sf::Vector2f mp);
    sf::FloatRect taskButtonRect(std::size_t index) const;
    std::vector<PuzzleWindowBase*> visibleTaskWindows() const;
};

extern Desktop* g_Desktop;
