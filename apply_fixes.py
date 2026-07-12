import re

with open(r'd:\Xgame\Xgame\src\DesktopGame.cpp', 'r', encoding='utf-8') as f:
    code = f.read()

# 1. sf::FloatRect size properties
code = code.replace('rect.width', 'rect.size.x')
code = code.replace('rect.height', 'rect.size.y')
code = code.replace('tr.width', 'tr.size.x')
code = code.replace('tr.height', 'tr.size.y')

# 2. Fix sf::Vector2f passing in { } by explicit sf::Vector2f(x, y) 
# wait, actually, we can just replace `{x, y}` to `sf::Vector2f({x, y})` where needed.
# Since `{x, y}` might be used for other things, let's just do targeted replaces.
code = code.replace('line.setSize({rect.size.x - 2.f, 1.f})', 'line.setSize(sf::Vector2f(rect.size.x - 2.f, 1.f))')
code = code.replace('line.setSize({1.f, rect.size.y})', 'line.setSize(sf::Vector2f(1.f, rect.size.y))')
code = code.replace('line.setSize({1.f, rect.size.y - 2.f})', 'line.setSize(sf::Vector2f(1.f, rect.size.y - 2.f))')
code = code.replace('line.setSize({rect.size.x, 1.f})', 'line.setSize(sf::Vector2f(rect.size.x, 1.f))')
code = code.replace('line.setSize({rect.size.x - 2.f, 1.f})', 'line.setSize(sf::Vector2f(rect.size.x - 2.f, 1.f))')

# setPosition with rect properties
code = code.replace('line.setPosition({rect.position.x, rect.position.y})', 'line.setPosition(rect.position)')
code = code.replace('line.setPosition({rect.position.x + 1.f, rect.position.y + 1.f})', 'line.setPosition(sf::Vector2f(rect.position.x + 1.f, rect.position.y + 1.f))')
code = code.replace('line.setPosition({rect.position.x, rect.position.y + rect.size.y - 1.f})', 'line.setPosition(sf::Vector2f(rect.position.x, rect.position.y + rect.size.y - 1.f))')
code = code.replace('line.setPosition({rect.position.x + 1.f, rect.position.y + rect.size.y - 2.f})', 'line.setPosition(sf::Vector2f(rect.position.x + 1.f, rect.position.y + rect.size.y - 2.f))')
code = code.replace('line.setPosition({rect.position.x + rect.size.x - 1.f, rect.position.y})', 'line.setPosition(sf::Vector2f(rect.position.x + rect.size.x - 1.f, rect.position.y))')
code = code.replace('line.setPosition({rect.position.x + rect.size.x - 2.f, rect.position.y + 1.f})', 'line.setPosition(sf::Vector2f(rect.position.x + rect.size.x - 2.f, rect.position.y + 1.f))')

# t.setPosition in drawCaptionButton
code = code.replace('t.setPosition({rect.position.x + rect.size.x / 2.f + (pressed ? 1.f : 0.f),\n                   rect.position.y + rect.size.y / 2.f + (pressed ? 1.f : 0.f)})', 't.setPosition(sf::Vector2f(rect.position.x + rect.size.x / 2.f + (pressed ? 1.f : 0.f), rect.position.y + rect.size.y / 2.f + (pressed ? 1.f : 0.f)))')
# and the t.setOrigin
code = code.replace('t.setOrigin({tr.position.x + tr.size.x / 2.f, tr.position.y + tr.size.y / 2.f})', 't.setOrigin(sf::Vector2f(tr.position.x + tr.size.x / 2.f, tr.position.y + tr.size.y / 2.f))')


# 3. SFML 3 Text and Vector constructions

# ContextMenu::show
code = code.replace('background.setSize({width, height});', 'background.setSize(sf::Vector2f(width, height));')
code = code.replace('shadow.setSize({width, height}); shadow.setPosition({position.x + 4.f, position.y + 4.f});', 'shadow.setSize(sf::Vector2f(width, height)); shadow.setPosition(sf::Vector2f(position.x + 4.f, position.y + 4.f));')
# ContextMenu::render
code = code.replace('line.setPosition({position.x + 2.f, y + itemHeight / 2.f});', 'line.setPosition(sf::Vector2f(position.x + 2.f, y + itemHeight / 2.f));')
code = code.replace('line.setPosition({position.x + 2.f, y + itemHeight / 2.f + 1.f});', 'line.setPosition(sf::Vector2f(position.x + 2.f, y + itemHeight / 2.f + 1.f));')
code = code.replace('hl.setPosition({position.x + 2.f, y});', 'hl.setPosition(sf::Vector2f(position.x + 2.f, y));')
code = code.replace('itemTexts[i].setPosition({position.x + 24.f, y + 2.f});', 'itemTexts[i].setPosition(sf::Vector2f(position.x + 24.f, y + 2.f));')
code = code.replace('sf::RectangleShape line({width - 4.f, 1.f});', 'sf::RectangleShape line(sf::Vector2f(width - 4.f, 1.f));')
code = code.replace('sf::RectangleShape hl({width - 4.f, itemHeight});', 'sf::RectangleShape hl(sf::Vector2f(width - 4.f, itemHeight));')

# PuzzleWindowBase constructors
code = code.replace('title(t), uiFont(&font) {', 'title(t), uiFont(&font), titleText(font) {')
code = code.replace('textContent(content) {', 'textContent(content), docText(font) {')
code = code.replace('winFont(&font) {\n    addressInput', 'winFont(&font), addrText(font), contentText(font), statusText(font) {\n    addressInput')
code = code.replace('mode(pwdMode) {\n    isModal', 'mode(pwdMode), promptText(font), inputText(font), okText(font), cancelText(font) {\n    isModal')
code = code.replace('winFont(&font) {\n    canMaximize', 'winFont(&font), secretBtnText(font), okText(font), cancelText(font) {\n    canMaximize')
code = code.replace('PuzzleWindowBase("系统奖励", p, w, h, font) {\n    congratsText', 'PuzzleWindowBase("系统奖励", p, w, h, font), congratsText(font) {\n    congratsText')
code = code.replace('PuzzleWindowBase(boxTitle, p, w, h, font) {\n    isModal', 'PuzzleWindowBase(boxTitle, p, w, h, font), msgText(font), okBtnText(font) {\n    isModal')
code = code.replace('label(lbl) {\n    sf::Image img;', 'label(lbl), labelText(font) {\n    sf::Image img;')
code = code.replace('isRecycleBinMode(recycleBin), winFont(&font) {\n    folderPath', 'isRecycleBinMode(recycleBin), winFont(&font), restoreBtnText(font) {\n    folderPath')

# Vector2 constructions inside Desktop and Windows
def fix_vector_args(code):
    return re.sub(r'\.setSize\(\{\s*([^,]+)\s*,\s*([^}]+)\s*\}\)', r'.setSize(sf::Vector2f(\1, \2))', code)

def fix_pos_args(code):
    return re.sub(r'\.setPosition\(\{\s*([^,]+)\s*,\s*([^}]+)\s*\}\)', r'.setPosition(sf::Vector2f(\1, \2))', code)

def fix_rect_args(code):
    return re.sub(r'sf::RectangleShape\s+(\w+)\(\{\s*([^,]+)\s*,\s*([^}]+)\s*\}\)', r'sf::RectangleShape \1(sf::Vector2f(\2, \3))', code)

def fix_pos_2args(code):
    return re.sub(r'\.setPosition\(([^,]+),\s*([^)]+)\)', r'.setPosition(sf::Vector2f(\1, \2))', code)

code = fix_vector_args(code)
code = fix_pos_args(code)
code = fix_rect_args(code)
code = fix_pos_2args(code)

# FloatRect creations
code = code.replace('return {position.x + width - BORDER - CAPTION_BTN - 2.f, position.y + BORDER + 2.f, CAPTION_BTN, CAPTION_BTN};', 'return sf::FloatRect(sf::Vector2f(position.x + width - BORDER - CAPTION_BTN - 2.f, position.y + BORDER + 2.f), sf::Vector2f(CAPTION_BTN, CAPTION_BTN));')
code = code.replace('return {position.x + width - BORDER - CAPTION_BTN*2 - 4.f, position.y + BORDER + 2.f, CAPTION_BTN, CAPTION_BTN};', 'return sf::FloatRect(sf::Vector2f(position.x + width - BORDER - CAPTION_BTN*2 - 4.f, position.y + BORDER + 2.f), sf::Vector2f(CAPTION_BTN, CAPTION_BTN));')
code = code.replace('return {position.x + width - BORDER - CAPTION_BTN*3 - 4.f, position.y + BORDER + 2.f, CAPTION_BTN, CAPTION_BTN};', 'return sf::FloatRect(sf::Vector2f(position.x + width - BORDER - CAPTION_BTN*3 - 4.f, position.y + BORDER + 2.f), sf::Vector2f(CAPTION_BTN, CAPTION_BTN));')
code = code.replace('return {position.x + BORDER, position.y + TITLEBAR_H + 1.f, width - 2*BORDER, height - TITLEBAR_H - BORDER - 1.f};', 'return sf::FloatRect(sf::Vector2f(position.x + BORDER, position.y + TITLEBAR_H + 1.f), sf::Vector2f(width - 2*BORDER, height - TITLEBAR_H - BORDER - 1.f));')
code = code.replace('return {position, {width, TITLEBAR_H}};', 'return sf::FloatRect(position, sf::Vector2f(width, TITLEBAR_H));')

# 4. sf::Event properties
# event.type == sf::Event::MouseButtonPressed => event.is<sf::Event::MouseButtonPressed>()
# Wait, SFML 3 sf::Event is a std::variant!
code = code.replace('event.type == sf::Event::MouseButtonPressed', 'event.is<sf::Event::MouseButtonPressed>()')
code = code.replace('event.type == sf::Event::MouseButtonReleased', 'event.is<sf::Event::MouseButtonReleased>()')
code = code.replace('event.type == sf::Event::MouseMoved', 'event.is<sf::Event::MouseMoved>()')
code = code.replace('event.type == sf::Event::TextEntered', 'event.is<sf::Event::TextEntered>()')
code = code.replace('event.type == sf::Event::KeyPressed', 'event.is<sf::Event::KeyPressed>()')
code = code.replace('event.type == sf::Event::Closed', 'event.is<sf::Event::Closed>()')

# Button accesses:
# event.mouseButton.button == sf::Mouse::Left
# In SFML 3, if event is MouseButtonPressed, you access it via event.getIf<sf::Event::MouseButtonPressed>()->button
# But for simplicity, we can do:
def replace_mouse_pressed(m):
    return """if (const auto* mb = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mb->button == sf::Mouse::Button::Left) {"""

code = code.replace('if (event.is<sf::Event::MouseButtonPressed>() && event.mouseButton.button == sf::Mouse::Left) {',
                    'if (const auto* mb = event.getIf<sf::Event::MouseButtonPressed>()) {\n        if (mb->button == sf::Mouse::Button::Left) {')
# also replace right click
code = code.replace('if (event.is<sf::Event::MouseButtonPressed>() && event.mouseButton.button == sf::Mouse::Right) {',
                    'if (const auto* mb = event.getIf<sf::Event::MouseButtonPressed>()) {\n        if (mb->button == sf::Mouse::Button::Right) {')

# Need to fix x,y for mouse events
code = code.replace('event.mouseButton.x', 'mb->position.x')
code = code.replace('event.mouseButton.y', 'mb->position.y')

code = code.replace('if (event.is<sf::Event::MouseButtonReleased>() && event.mouseButton.button == sf::Mouse::Left) {',
                    'if (const auto* mb = event.getIf<sf::Event::MouseButtonReleased>()) {\n        if (mb->button == sf::Mouse::Button::Left) {')
code = code.replace('if (event.is<sf::Event::MouseButtonReleased>() && event.mouseButton.button == sf::Mouse::Right) {',
                    'if (const auto* mb = event.getIf<sf::Event::MouseButtonReleased>()) {\n        if (mb->button == sf::Mouse::Button::Right) {')

# event.text.unicode
code = code.replace('event.text.unicode', 'event.getIf<sf::Event::TextEntered>()->unicode')

# event.key.code
code = code.replace('event.key.code', 'event.getIf<sf::Event::KeyPressed>()->code')
code = code.replace('sf::Keyboard::Enter', 'sf::Keyboard::Key::Enter')


# 5. Fix desktop clockText and startLabel
code = code.replace('clockText.setFont(systemFont);', 'clockText.emplace(systemFont);')
code = code.replace('clockText.setCharacterSize(14);', 'clockText->setCharacterSize(14);')
code = code.replace('clockText.setFillColor(C::TextWhite);', 'clockText->setFillColor(C::TextWhite);')
code = code.replace('clockText.setPosition(sf::Vector2f(WIN_W - 60.f, WIN_H - TASKBAR_H + 12.f));', 'clockText->setPosition(sf::Vector2f(WIN_W - 60.f, WIN_H - TASKBAR_H + 12.f));')
code = code.replace('clockText.setString(buf);', 'clockText->setString(buf);')
code = code.replace('renderWin->draw(clockText);', 'if (clockText) renderWin->draw(*clockText);')

code = code.replace('startLabel.setFont(systemFont);', 'startLabel.emplace(systemFont);')
code = code.replace('startLabel.setCharacterSize(16);', 'startLabel->setCharacterSize(16);')
code = code.replace('startLabel.setStyle(sf::Text::Bold | sf::Text::Italic);', 'startLabel->setStyle(sf::Text::Bold | sf::Text::Italic);')
code = code.replace('startLabel.setFillColor(C::TextWhite);', 'startLabel->setFillColor(C::TextWhite);')
code = code.replace('startLabel.setString(U8("开始"));', 'startLabel->setString(U8("开始"));')
code = code.replace('startLabel.setPosition(sf::Vector2f(35.f, WIN_H - TASKBAR_H + 10.f));', 'startLabel->setPosition(sf::Vector2f(35.f, WIN_H - TASKBAR_H + 10.f));')
code = code.replace('renderWin->draw(startLabel);', 'if (startLabel) renderWin->draw(*startLabel);')


with open(r'd:\Xgame\Xgame\src\DesktopGame.cpp', 'w', encoding='utf-8') as f:
    f.write(code)

print("Applied quick fixes to DesktopGame.cpp")
