# Theme System for ChatRoom

## TL;DR
> Add a theming engine with 5 color schemes and 3 font styles, plus Settings page UI to switch between them.
>
> **Deliverables**:
> - ThemeManager C++ class (5 color palettes + 3 font configs)
> - Settings page with theme/font picker
> - All QML files updated to use theme properties
>
> **Estimated Effort**: Large
> **Parallel Execution**: YES - 4 waves
> **Critical Path**: ThemeManager C++ → Settings UI → QML integration

---

## Context

### Original Request
Add theme functionality to the ChatRoom project: 5 color schemes, 3 font styles, Settings page controls.

### Current State
- Project uses hardcoded hex colors throughout all QML files
- Key colors: `#1890ff` (primary), `#95ec69` (bubble mine), `#f0f2f5` (background), `#ff4d4f` (danger)
- SettingsPage.qml exists but has no theme/font controls
- AppCore creates all C++ controllers and exposes them to QML

### Research Findings
- QML files with hardcoded colors: main.qml, LoginPage.qml, RegisterPage.qml, ChatPage.qml, SettingsPage.qml, MessageBubble.qml, GroupMessageBubble.qml, ContactItem.qml, ConversationItem.qml, GroupConversationItem.qml, InputBar.qml, Toast.qml, all dialogs, SearchBar.qml, Avatar.qml
- Most frequent colors: `#1890ff` (~40 uses), `#333` (text primary, ~25 uses), `#999` (text tertiary, ~15 uses), `#f0f2f5` (background, ~5 uses), `#ff4d4f` (danger, ~12 uses)
- C++ pattern exists: all controllers in `src/qmlbridge/` created by AppCore, exposed via Q_PROPERTY

---

## Work Objectives

### Core Objective
Implement a theme engine that allows users to switch between 5 color schemes and 3 font styles via the Settings page, with all UI files reacting to theme changes in real-time.

### Concrete Deliverables
- `client/src/qmlbridge/ThemeManager.h` - C++ class declaration
- `client/src/qmlbridge/ThemeManager.cpp` - C++ class implementation with all 5 themes + 3 fonts
- `client/resources/fonts/` - Downloaded 3 font families (each with regular + bold .ttf/.otf)
- Modified `client/src/qmlbridge/AppCore.h` - add ThemeManager property
- Modified `client/src/qmlbridge/AppCore.cpp` - create ThemeManager, expose to QML
- Modified `client/main.cpp` - expose themeManager context property
- Modified `client/qml/pages/SettingsPage.qml` - add theme/font picker UI
- FontLoader in main.qml to load selected font globally
- Modified all QML files to use `appCore.themeManager.*` color/font properties

### Definition of Done
- [ ] 5 color themes switchable via Settings
- [ ] 3 font styles switchable via Settings
- [ ] All pages/controls update colors instantly when theme changes
- [ ] Theme/font preference saved to QSettings (persists across restarts)
- [ ] No hardcoded colors remain in visible UI elements

### Must Have
- ThemeManager C++ class with all 5 theme palettes and 3 font configs
- Settings page UI with radio-button or grid selection for theme/font
- Primary colors applied to headers, buttons, highlights
- Bubble colors applied to message bubbles
- Background/surface colors applied to pages and cards
- Font sizes applied throughout UI
- QSettings persistence

### Must NOT Have (Guardrails)
- Do NOT refactor every single non-visible color (e.g., Avatar hash-based colors, status indicators) on first pass - focus on visible theming
- Do NOT change the layout structure of any QML file
- Do NOT break existing functionality - theme should be purely additive

---

## Verification Strategy

> **ZERO HUMAN INTERVENTION** - ALL verification is agent-executed.

### Test Decision
- **Infrastructure exists**: NO (no test infrastructure found)
- **Automated tests**: NO
- **Agent-Executed QA**: ALWAYS - each task includes manual verification steps

### QA Policy
Every task MUST include agent-executed QA scenarios. Evidence saved to `.sisyphus/evidence/task-{N}-{scenario-slug}.{ext}`.

- **C++ compilation**: Verify with `cmake --build` that ThemeManager compiles
- **QML rendering**: Run the app and visually verify theme changes via screenshots
- **Settings interaction**: Verify clicking theme/font options in Settings changes the UI
- **Persistence**: Restart app and verify last selected theme/font is still active

---

## Execution Strategy

### Parallel Execution Waves

```
Wave 1 (Foundation - C++ ThemeManager):
├── Task 1: Create ThemeManager.h (class declaration, structs, Q_PROPERTY)
├── Task 2: Create ThemeManager.cpp (5 themes data, 3 fonts, QSettings, getters)
├── Task 3: Update CMakeLists.txt + main.cpp + AppCore (wire ThemeManager)
└── Task 4: Download 3 font families + register in QML

Wave 1b (Font Assets):
├── Task 4a: Download ZCOOL QingKe HuangYou (站酷快乐体) from Google Fonts
├── Task 4b: Download Noto Sans SC (思源黑体) from Google Fonts  
├── Task 4c: Download M PLUS Rounded 1c (圆润体) from Google Fonts
├── Task 4d: Add font files to resources/fonts/ and resources/sounds.qrc (or new fonts.qrc)
├── Task 4e: Add FontLoader in main.qml, load selected font family based on themeManager.fontFamilyName
└── Task 4f: Expose font loading via QML (the font selector switches which FontLoader is active)

Wave 2 (Settings UI):
├── Task 5: Update SettingsPage.qml (theme grid + font picker with previews)

Wave 3 (QML Integration - Color + Font Properties):
├── Task 6: Update main.qml (window bg, reconnect bar, global font)
├── Task 7: Update LoginPage.qml + RegisterPage.qml (background, button, link colors, font)
├── Task 8: Update ChatPage.qml (header, tabs, backgrounds, buttons, font)
├── Task 9: Update MessageBubble.qml + GroupMessageBubble.qml (bubble colors, font, radius)

Wave 4 (QML Integration - Remaining Components):
├── Task 10: Update Toast.qml, InputBar.qml, SearchBar.qml (colors + font)
├── Task 11: Update ContactItem.qml, ConversationItem.qml, GroupConversationItem.qml (colors + font)
├── Task 12: Update all dialogs (ConfirmDialog, AddFriendDialog, CreateGroupDialog, GroupInfoDialog)
└── Task 13: Update FriendRequestPage.qml, SettingsPage existing accent colors

Wave FINAL (Verification):
├── F1: Build + run, verify no compile errors
├── F2: Switch all 5 themes, screenshot each page
├── F3: Switch 3 fonts, verify characters render correctly (Chinese + English)
├── F4: Restart app, verify theme persists
└── F5: Verify no visible hardcoded colors remain

Critical Path: Task 1 → Task 2 → Task 3 → Tasks 4a-4f → Task 5 → Tasks 6-9 → Tasks 10-13 → F1-F5
```

---

## TODOs

- [ ] 1. Create `ThemeManager.h`

  **What to do**:
  Create the header file at `client/src/qmlbridge/ThemeManager.h` with:
  - `ThemePalette` struct with all color fields
  - `FontConfig` struct with font size fields  
  - `ThemeManager` class inheriting QObject
  - Q_PROPERTY for every color and font property (see design below)
  - Q_INVOKABLE `themeNames()` and `fontNames()` for Settings page
  - `themeIndex`/`fontIndex` writable properties with `themeChanged`/`fontChanged` signals
  - QSettings persistence in constructor and saveSettings()

  **ThemePalette fields**:
  ```
  QString name;         // Display name in Chinese
  QColor primary;       // #1890ff - main accent
  QColor primaryLight;  // #40a9ff - hover state
  QColor primaryHover;  // lighter hover
  QColor primaryBg;     // #e6f7ff - tinted background
  QColor background;    // #f0f2f5 - page bg
  QColor surface;       // #ffffff - card bg
  QColor surfaceAlt;    // #fafafa - alternate card bg
  QColor border;        // #e0e0e0 - borders
  QColor divider;       // #f0f0f0 - dividers
  QColor bubbleMine;    // #95ec69 - my bubble
  QColor bubbleOther;   // #ffffff - other's bubble
  QColor bubbleMineText;  // #333
  QColor bubbleOtherText; // #333
  QColor textPrimary;   // #333
  QColor textSecondary; // #666
  QColor textTertiary;  // #999
  QColor textInverse;   // #fff
  QColor success;       // #52c41a
  QColor warning;       // #faad14
  QColor danger;        // #ff4d4f
  QColor headerBg;      // primary - header background
  QColor headerText;    // #fff - header text
  QColor tabActive;     // primary
  QColor tabInactive;   // #666
  QColor tabHoverBg;    // #e6f7ff
  QColor tabActiveBg;   // #ffffff
  ```

  **FontConfig fields**:
  ```
  QString name;         // Display name in Chinese
  QString familyName;   // Font family name (e.g. "ZCOOL QingKe HuangYou", "Noto Sans SC", "Nunito")
  QString fallback;     // English fallback font (e.g. "Quicksand", "Inter", "Nunito")
  QString sourcePath;   // Path to font file in resources (optional, "" if system font)
  int bodySize;         // Base font size (varies per font style, e.g. 14/14/14)
  int smallSize;        // 12
  int bubbleSize;       // 14
  int headerSize;       // 16
  int messageSpacing;   // 8
  int bubbleRadius;     // 8
  ```

  **5 Color Themes**:
  1. 🌸 **樱花** (Sakura Pink): primary=#ff6b81, background=#fff5f7, bubbleMine=#ffdde3
  2. 🌲 **森林** (Forest Green): primary=#2d8a4e, background=#f0f7f0, bubbleMine=#b8e6b8
  3. 🌌 **极光** (Aurora Purple): primary=#6c5ce7, background=#f4f0ff, bubbleMine=#d4c5ff
  4. 🍵 **抹茶** (Matcha): primary=#9acd32, background=#f7fce8, bubbleMine=#d4ed8a
  5. 🌊 **海洋** (Ocean Teal): primary=#00b894, background=#f0faf8, bubbleMine=#b2dfdb

  **3 Font Styles (downloadable font files)**:
  1. 📝 **手写体** (Handwritten): 
     - Chinese: ZCOOL QingKe HuangYou (站酷快乐体)
     - English fallback: Caveat / Quicksand
     - Source: https://fonts.google.com/specimen/ZCOOL+QingKe+HuangYou
     - File: ZCOOLQingKeHuangYou-Regular.ttf
  2. 🎯 **标准体** (Standard Sans):
     - Chinese: Noto Sans SC (思源黑体)
     - English fallback: Inter / Montserrat
     - Source: https://fonts.google.com/specimen/Noto+Sans+SC
     - File: NotoSansSC-Regular.ttf, NotoSansSC-Bold.ttf
  3. 🫧 **圆润体** (Rounded):
     - Chinese: M PLUS Rounded 1c (圆润体)
     - English fallback: Nunito
     - Source: https://fonts.google.com/specimen/M+PLUS+Rounded+1c
     - File: MPLUSRounded1c-Regular.ttf, MPLUSRounded1c-Bold.ttf

  **Must NOT do**:
  - Don't forget Q_PROPERTY NOTIFY signals - required for QML auto-update
  - Don't forget QSettings save on setThemeIndex/setFontIndex
  - Include proper namespace `chatroom::client`

  **Recommended Agent Profile**:
  - Category: `unspecified-high`
  - Skills: [] (no skills needed)

  **References**:
  - `client/src/qmlbridge/AppCore.h` - Follow the existing Q_PROPERTY + QML_ELEMENT pattern
  - `client/src/qmlbridge/UserController.h` - Simple Q_PROPERTY example

  **Acceptance Criteria**:
  - [ ] File exists at `client/src/qmlbridge/ThemeManager.h`
  - [ ] All Q_PROPERTY declarations present (25+ color + 7 font + 2 index)
  - [ ] Q_INVOKABLE `themeNames()` and `fontNames()` declared
  - [ ] Class uses Q_OBJECT, QML_ELEMENT, QML_UNCREATABLE macros

  **QA Scenarios**:
  ```
  Scenario: Header file compiles
    Tool: Bash
    Preconditions: File exists
    Steps:
      1. Verify file exists
      2. Check all structs and class declarations are syntactically valid
    Expected Result: No syntax errors
    Evidence: .sisyphus/evidence/task-1-header-verified.txt

  Scenario: Check all Q_PROPERTY types
    Tool: Bash
    Steps: Grep for Q_PROPERTY declarations and count them
    Expected Result: At least 30 Q_PROPERTY declarations
    Evidence: .sisyphus/evidence/task-1-property-count.txt
  ```

  **Commit**: YES
  - Message: `feat(theme): add ThemeManager header with theme/font data structures`
  - Files: `client/src/qmlbridge/ThemeManager.h`


- [ ] 2. Create `ThemeManager.cpp`

  **What to do**:
  Create the implementation at `client/src/qmlbridge/ThemeManager.cpp` with:
  - Static arrays of 5 ThemePalette structs with all color values
  - Static arrays of 3 FontConfig structs
  - Constructor loading saved settings from QSettings
  - All getter methods returning current theme/font values
  - setThemeIndex/setFontIndex with QSettings save + signal emit
  - themeNames() and fontNames() returning display names

  **Color Theme Data - Complete Values**:

  **Theme 0 - 🌸 樱花 (Sakura Pink)**:
  primary=#ff6b81, primaryLight=#ff8a9e, primaryHover=#fff0f3, primaryBg=#fff0f3
  background=#fff5f7, surface=#ffffff, surfaceAlt=#fafafa
  border=#ffdde3, divider=#ffe8ec
  bubbleMine=#ffdde3, bubbleOther=#ffffff
  bubbleMineText=#333333, bubbleOtherText=#333333
  textPrimary=#333333, textSecondary=#666666, textTertiary=#999999
  textInverse=#ffffff, success=#52c41a, warning=#faad14, danger=#ff4d4f
  headerBg=#ff6b81, headerText=#ffffff
  tabActive=#ff6b81, tabInactive=#666666, tabHoverBg=#fff0f3, tabActiveBg=#ffffff

  **Theme 1 - 🌲 森林 (Forest Green)**:
  primary=#2d8a4e, primaryLight=#4ecb71, primaryHover=#edf7f0, primaryBg=#edf7f0
  background=#f0f7f0, surface=#ffffff, surfaceAlt=#fafafa
  border=#c8e6c9, divider=#dcedc8
  bubbleMine=#b8e6b8, bubbleOther=#ffffff
  bubbleMineText=#333333, bubbleOtherText=#333333
  textPrimary=#333333, textSecondary=#666666, textTertiary=#999999
  textInverse=#ffffff, success=#2d8a4e, warning=#faad14, danger=#ff4d4f
  headerBg=#2d8a4e, headerText=#ffffff
  tabActive=#2d8a4e, tabInactive=#666666, tabHoverBg=#edf7f0, tabActiveBg=#ffffff

  **Theme 2 - 🌌 极光 (Aurora Purple)**:
  primary=#6c5ce7, primaryLight=#8f7ff0, primaryHover=#f0edff, primaryBg=#f0edff
  background=#f4f0ff, surface=#ffffff, surfaceAlt=#fafafa
  border=#d4c5ff, divider=#e0d6ff
  bubbleMine=#d4c5ff, bubbleOther=#ffffff
  bubbleMineText=#333333, bubbleOtherText=#333333
  textPrimary=#333333, textSecondary=#666666, textTertiary=#999999
  textInverse=#ffffff, success=#52c41a, warning=#faad14, danger=#ff4d4f
  headerBg=#6c5ce7, headerText=#ffffff
  tabActive=#6c5ce7, tabInactive=#666666, tabHoverBg=#f0edff, tabActiveBg=#ffffff

  **Theme 3 - 🍵 抹茶 (Matcha)**:
  primary=#9acd32, primaryLight=#b8e050, primaryHover=#f4fae0, primaryBg=#f4fae0
  background=#f7fce8, surface=#ffffff, surfaceAlt=#fafafa
  border=#d4ed8a, divider=#e2f0b0
  bubbleMine=#d4ed8a, bubbleOther=#ffffff
  bubbleMineText=#333333, bubbleOtherText=#333333
  textPrimary=#333333, textSecondary=#666666, textTertiary=#999999
  textInverse=#ffffff, success=#9acd32, warning=#faad14, danger=#ff4d4f
  headerBg=#9acd32, headerText=#ffffff
  tabActive=#9acd32, tabInactive=#666666, tabHoverBg=#f4fae0, tabActiveBg=#ffffff

  **Theme 4 - 🌊 海洋 (Ocean Teal)**:
  primary=#00b894, primaryLight=#33d9b2, primaryHover=#e0f8f4, primaryBg=#e0f8f4
  background=#f0faf8, surface=#ffffff, surfaceAlt=#fafafa
  border=#b2dfdb, divider=#c8e6de
  bubbleMine=#b2dfdb, bubbleOther=#ffffff
  bubbleMineText=#333333, bubbleOtherText=#333333
  textPrimary=#333333, textSecondary=#666666, textTertiary=#999999
  textInverse=#ffffff, success=#00b894, warning=#faad14, danger=#ff4d4f
  headerBg=#00b894, headerText=#ffffff
  tabActive=#00b894, tabInactive=#666666, tabHoverBg=#e0f8f4, tabActiveBg=#ffffff

  **Must NOT do**:
  - Don't forget `QSettings` organization name "ChatRoom" to match existing settings
  - `QSettings` keys: "theme/themeIndex", "theme/fontIndex"
  - Default values: themeIndex=0, fontIndex=0

  **Recommended Agent Profile**:
  - Category: `unspecified-high`
  - Skills: []

  **References**:
  - `client/src/qmlbridge/AppCore.cpp:39-50` - How AppCore creates controllers
  - QSettings docs: https://doc.qt.io/qt-6/qsettings.html

  **Acceptance Criteria**:
  - [ ] File exists at `client/src/qmlbridge/ThemeManager.cpp`
  - [ ] All getters return correct values for each theme/font
  - [ ] setThemeIndex emits themeChanged signal
  - [ ] setFontIndex emits fontChanged signal
  - [ ] QSettings saves and restores theme/font preference

  **QA Scenarios**:
  ```
  Scenario: Theme getters work
    Tool: Bash
    Steps: Include ThemeManager in a test compile
    Expected Result: No undefined reference errors

  Scenario: Font getters work
    Tool: Bash
    Steps: Check font configs have correct values
    Expected Result: 3 font configs with distinct values
  ```

  **Commit**: YES
  - Message: `feat(theme): implement ThemeManager with 5 themes and 3 fonts`
  - Files: `client/src/qmlbridge/ThemeManager.cpp`


- [ ] 3. Wire ThemeManager into AppCore + CMakeLists + main.cpp

  **What to do**:
  - Add `ThemeManager.h/.cpp` to `CLIENT_SOURCES` in `client/CMakeLists.txt`
  - Add `#include "qmlbridge/ThemeManager.h"` to `AppCore.h`
  - Add `Q_PROPERTY(ThemeManager* themeManager READ themeManager CONSTANT)` to AppCore
  - Add `ThemeManager* m_themeManager = nullptr;` member and `ThemeManager* themeManager() const;` accessor
  - Create ThemeManager in AppCore constructor: `m_themeManager = new ThemeManager(this);`
  - No need to expose as separate context property - it's accessed via `appCore.themeManager`

  **Must NOT do**:
  - Don't forget `QML_UNCREATABLE` if adding QML_ELEMENT to ThemeManager
  - Actually, ThemeManager doesn't need QML_ELEMENT since it's exposed via AppCore property

  **Recommended Agent Profile**:
  - Category: `unspecified-high`
  - Skills: []

  **References**:
  - `client/CMakeLists.txt:25-32` - Where other controllers are listed
  - `client/src/qmlbridge/AppCore.h:33-42` - Existing Q_PROPERTY pattern
  - `client/src/qmlbridge/AppCore.cpp:44-47` - Controller creation pattern

  **Acceptance Criteria**:
  - [ ] CMakeLists.txt updated with ThemeManager source files
  - [ ] AppCore.h has themeManager Q_PROPERTY
  - [ ] AppCore.cpp creates ThemeManager
  - [ ] Project compiles without errors

  **QA Scenarios**:
  ```
  Scenario: Build succeeds
    Tool: Bash
    Steps: cd build && cmake --build .
    Expected Result: Build succeeds, no errors
    Evidence: .sisyphus/evidence/task-3-build-success.txt

  Scenario: Run app without crash
    Tool: Bash
    Steps: Start ChatRoomClient, check it doesn't crash
    Expected Result: App starts and shows login page
  ```

  **Commit**: YES
  - Message: `feat(theme): integrate ThemeManager into AppCore and build system`
  - Files: `client/CMakeLists.txt`, `client/src/qmlbridge/AppCore.h`, `client/src/qmlbridge/AppCore.cpp`


- [ ] 4a. Download ZCOOL QingKe HuangYou font

  **What to do**:
  Download the ZCOOL QingKe HuangYou font from Google Fonts. This is a handwritten-style Chinese font (站酷快乐体).
  
  Steps:
  1. Download from: https://fonts.google.com/specimen/ZCOOL+QingKe+HuangYou
  2. Get the Regular .ttf file
  3. Save to `client/resources/fonts/ZCOOLQingKeHuangYou-Regular.ttf`
  
  Also download Caveat (English handwriting font to pair):
  1. Download from: https://fonts.google.com/specimen/Caveat
  2. Get Caveat Regular and Bold .ttf files
  3. Save to `client/resources/fonts/Caveat-Regular.ttf`, `client/resources/fonts/Caveat-Bold.ttf`

  **Must NOT do**:
  - Only use free/open-source fonts (SIL Open Font License)
  - Don't download unnecessary variants (weight/variable) - Regular only for Chinese, Regular+Bold for English

  **Recommended Agent Profile**:
  - Category: `quick`
  - Skills: []

  **Acceptance Criteria**:
  - [ ] ZCOOLQingKeHuangYou-Regular.ttf exists in `client/resources/fonts/`
  - [ ] Caveat-Regular.ttf and Caveat-Bold.ttf exist
  - [ ] Files are valid TrueType fonts (check file header)

  **QA Scenarios**:
  ```
  Scenario: Font files exist and are valid
    Tool: Bash
    Steps: Check each font file exists and has correct size (>10KB)
    Expected Result: 3 .ttf files, all non-empty, all valid TrueType
    Evidence: .sisyphus/evidence/task-4a-fonts-exist.txt
  ```

  **Commit**: YES (with 4b-4f as group)


- [ ] 4b. Download Noto Sans SC font

  **What to do**:
  Download Noto Sans SC (思源黑体) from Google Fonts. This is a clean sans-serif Chinese font.
  
  Steps:
  1. Download from: https://fonts.google.com/specimen/Noto+Sans+SC
  2. Get Regular and Bold .ttf files
  3. Save to `client/resources/fonts/NotoSansSC-Regular.ttf`, `client/resources/fonts/NotoSansSC-Bold.ttf`
  
  Also download Inter (English sans-serif to pair):
  1. Download from: https://fonts.google.com/specimen/Inter
  2. Get Inter Regular and Bold .ttf files
  3. Save to `client/resources/fonts/Inter-Regular.ttf`, `client/resources/fonts/Inter-Bold.ttf`

  **Recommended Agent Profile**:
  - Category: `quick`
  - Skills: []

  **Acceptance Criteria**:
  - [ ] All 4 .ttf files exist

  **Commit**: YES (with 4a, 4c-4f as group)


- [ ] 4c. Download M PLUS Rounded 1c font

  **What to do**:
  Download M PLUS Rounded 1c from Google Fonts. This is a cute rounded Japanese/Chinese font.
  
  Steps:
  1. Download from: https://fonts.google.com/specimen/M+PLUS+Rounded+1c
  2. Get Regular and Bold .ttf files
  3. Save to `client/resources/fonts/MPLUSRounded1c-Regular.ttf`, `client/resources/fonts/MPLUSRounded1c-Bold.ttf`
  
  Also download Nunito (English rounded to pair):
  1. Download from: https://fonts.google.com/specimen/Nunito
  2. Get Nunito Regular and Bold .ttf files
  3. Save to `client/resources/fonts/Nunito-Regular.ttf`, `client/resources/fonts/Nunito-Bold.ttf`

  **Recommended Agent Profile**:
  - Category: `quick`
  - Skills: []

  **Acceptance Criteria**:
  - [ ] All 4 .ttf files exist

  **Commit**: YES (with 4a, 4b, 4d-4f as group)


- [ ] 4d. Add font files to Qt resources

  **What to do**:
  - Create `client/resources/fonts/fonts.qrc` that lists all downloaded .ttf files
  - Add `resources/fonts/fonts.qrc` to `CLIENT_SOURCES` in `client/CMakeLists.txt`
  - Verify fonts compile into binary

  **fonts.qrc structure**:
  ```xml
  <RCC>
    <qresource prefix="/fonts">
      <file alias="ZCOOLQingKeHuangYou-Regular.ttf">ZCOOLQingKeHuangYou-Regular.ttf</file>
      <file alias="Caveat-Regular.ttf">Caveat-Regular.ttf</file>
      <file alias="Caveat-Bold.ttf">Caveat-Bold.ttf</file>
      <file alias="NotoSansSC-Regular.ttf">NotoSansSC-Regular.ttf</file>
      <file alias="NotoSansSC-Bold.ttf">NotoSansSC-Bold.ttf</file>
      <file alias="Inter-Regular.ttf">Inter-Regular.ttf</file>
      <file alias="Inter-Bold.ttf">Inter-Bold.ttf</file>
      <file alias="MPLUSRounded1c-Regular.ttf">MPLUSRounded1c-Regular.ttf</file>
      <file alias="MPLUSRounded1c-Bold.ttf">MPLUSRounded1c-Bold.ttf</file>
      <file alias="Nunito-Regular.ttf">Nunito-Regular.ttf</file>
      <file alias="Nunito-Bold.ttf">Nunito-Bold.ttf</file>
    </qresource>
  </RCC>
  ```

  **Recommended Agent Profile**:
  - Category: `quick`
  - Skills: []

  **Acceptance Criteria**:
  - [ ] fonts.qrc created
  - [ ] fonts.qrc added to CMakeLists.txt
  - [ ] Build succeeds

  **Commit**: YES (with 4a-4c, 4e-4f as group)


- [ ] 4e. Add FontLoader to main.qml

  **What to do**:
  Add FontLoader elements to main.qml that load fonts at startup based on themeManager settings.
  
  Structure in main.qml:
  ```qml
  // Font loaders for all 3 font families
  FontLoader { id: fontHandwritten; source: "qrc:/fonts/ZCOOLQingKeHuangYou-Regular.ttf" }
  FontLoader { id: fontHandwrittenEn; source: "qrc:/fonts/Caveat-Regular.ttf" }
  FontLoader { id: fontHandwrittenEnBold; source: "qrc:/fonts/Caveat-Bold.ttf" }
  
  FontLoader { id: fontStandard; source: "qrc:/fonts/NotoSansSC-Regular.ttf" }
  FontLoader { id: fontStandardBold; source: "qrc:/fonts/NotoSansSC-Bold.ttf" }
  FontLoader { id: fontStandardEn; source: "qrc:/fonts/Inter-Regular.ttf" }
  FontLoader { id: fontStandardEnBold; source: "qrc:/fonts/Inter-Bold.ttf" }
  
  FontLoader { id: fontRounded; source: "qrc:/fonts/MPLUSRounded1c-Regular.ttf" }
  FontLoader { id: fontRoundedBold; source: "qrc:/fonts/MPLUSRounded1c-Bold.ttf" }
  FontLoader { id: fontRoundedEn; source: "qrc:/fonts/Nunito-Regular.ttf" }
  FontLoader { id: fontRoundedEnBold; source: "qrc:/fonts/Nunito-Bold.ttf" }
  ```

  **How font switching works**:
  - ThemeManager.fontFamilyName returns the Chinese font name (e.g. "ZCOOL QingKe HuangYou")
  - ThemeManager.fontFamilyFallback returns the English font name (e.g. "Caveat")
  - The `FontLoader` registers the font family name globally in Qt
  - QML text elements use `font.family: appCore.themeManager.fontFamilyName` 
  - For chat text: `font.family: appCore.themeManager.fontFamilyName + ", " + appCore.themeManager.fontFamilyFallback`

  **Note**: The FontLoader approach loads all fonts at startup (11 FontLoaders is fine for performance). The ThemeManager just tells Text elements which registered family name to use.

  **Recommended Agent Profile**:
  - Category: `quick`
  - Skills: []

  **Acceptance Criteria**:
  - [ ] FontLoader elements added to main.qml
  - [ ] Fonts compile into binary and load at runtime

  **Commit**: YES (with 4a-4d, 4f as group)


- [ ] 4f. Wire font selection to font.family bindings

  **What to do**:
  - Add `fontFamilyName` and `fontFamilyFallback` Q_PROPERTY to ThemeManager
  - Each font style returns the correct font family name (matching the loaded FontLoader)
  - In QML Text elements: replace `font.family: ...` with `appCore.themeManager.fontFamilyName`
  - Or add a helper string property: `fontFamily` that is e.g. "ZCOOL QingKe HuangYou, Caveat"

  **Key**: In ThemeManager, add:
  ```qml
  Q_PROPERTY(QString fontFamily READ fontFamily NOTIFY fontChanged)
  QString fontFamily() const;  // returns "ZCOOL QingKe HuangYou, Caveat" or "Noto Sans SC, Inter" etc.
  ```

  So in QML: `font.family: appCore.themeManager.fontFamily`

  **Recommended Agent Profile**:
  - Category: `unspecified-low`
  - Skills: []

  **Acceptance Criteria**:
  - [ ] fontFamily Q_PROPERTY added to ThemeManager
  - [ ] Switching font changes text rendering across all UI

  **Commit**: YES (with 4a-4e as group)

  **What to do**:
  Add a "Theme" section and "Font" section to SettingsPage.qml between the Account section and the Logout button:

  ```
  // Theme section
  GroupBox {
      title: "Color Theme"
      GridLayout { columns: 3; ... }
      // Each theme is a Rectangle with the theme's primary color,
      // clicking calls appCore.themeManager.themeIndex = index
  }

  // Font section
  GroupBox {
      title: "Font Style"
      RowLayout { ... }
      // 3 buttons, one per font style
      // Highlight selected one
  }
  ```

  **Theme Grid Design**:
  - 5 color swatches in a horizontal row
  - Each swatch: 60x80 rounded rectangle showing primary color name
  - Clicking sets `appCore.themeManager.themeIndex = index`
  - Selected swatch has a border highlight using primary color

  **Font Style Design**:
  - 3 styled buttons in a row
  - "Default" (当前), "Large" (大号), "Compact" (紧凑)
  - Selected one has primary color background
  - Clicking sets `appCore.themeManager.fontIndex = index`

  **Must NOT do**:
  - Don't break existing Account section and Logout button
  - Don't remove the Clear History button
  - All theme/font colors use `appCore.themeManager.*` bindings

  **Recommended Agent Profile**:
  - Category: `visual-engineering`
  - Skills: []

  **References**:
  - `client/qml/pages/SettingsPage.qml` - current file to modify

  **Acceptance Criteria**:
  - [ ] Theme section shows 5 color swatches
  - [ ] Clicking a swatch changes the theme immediately
  - [ ] Font section shows 3 options
  - [ ] Clicking a font option changes the font immediately
  - [ ] Current selection is visually highlighted

  **QA Scenarios**:
  ```
  Scenario: Theme swatches visible
    Tool: Playwright
    Steps: Navigate to Settings, look for theme section
    Expected Result: 5 colored rectangles visible
    Evidence: .sisyphus/evidence/task-4-theme-swatches.png

  Scenario: Click theme swatch changes header color
    Tool: Playwright
    Steps: Click "WeChat Green" swatch, observe header color change
    Expected Result: Header color changes to green
  ```

  **Commit**: YES
  - Message: `feat(settings): add theme and font picker to SettingsPage`
  - Files: `client/qml/pages/SettingsPage.qml`


- [ ] 5. Update SettingsPage.qml with theme/font picker

  **What to do**:
  Add a "Theme" section and "Font" section to SettingsPage.qml between the Account section and the Logout button.

  **Theme Grid Design**:
  - 5 theme cards in a horizontal row
  - Each card: rounded rectangle showing theme's primary color swatch + theme name (emoji + name)
  - Clicking sets `appCore.themeManager.themeIndex = index`
  - Selected card has a bold border in theme's primary color
  
  **Font Style Design**:
  - 3 styled cards in a row
  - Each card shows a sample text in that font style (e.g. "Hello 你好" in the actual font)
  - Clicking sets `appCore.themeManager.fontIndex = index`
  - Selected card has a highlight border
  
  **Must NOT do**:
  - Don't break existing Account section and Logout button
  - Don't remove the Clear History button

  **Recommended Agent Profile**:
  - Category: `visual-engineering`
  - Skills: []

  **Acceptance Criteria**:
  - [ ] Theme section shows 5 color swatches
  - [ ] Clicking a swatch changes the theme immediately
  - [ ] Font section shows 3 options with preview text
  - [ ] Clicking a font option changes the font immediately
  - [ ] Current selection is visually highlighted

  **Commit**: YES


- [ ] 6. Update main.qml (window background, reconnect bar, font loading)

  **What to do**:
  Replace hardcoded colors with `appCore.themeManager.*` bindings:
  - Line 16: `color: "#f5f5f5"` → `color: appCore.themeManager.background`
  - Line 31: `color: "#fff7e6"` → use `appCore.themeManager.primaryBg`
  
  Also add FontLoader elements for all 3 font families (see task 4e for details).

  **Recommended Agent Profile**:
  - Category: `quick`
  - Skills: []

  **References**:
  - `client/qml/main.qml`

  **Acceptance Criteria**:
  - [ ] Window background updates when theme changes
  - [ ] Reconnect bar color updates when theme changes
  - [ ] All fonts load successfully

  **Commit**: YES (with tasks 7-9 as group)


- [ ] 7. Update LoginPage.qml + RegisterPage.qml colors + font

  **What to do**:
  Replace hardcoded colors with `appCore.themeManager.*`:  
  Add `font.family: appCore.themeManager.fontFamily` to text elements.

  **LoginPage.qml**:
  - `color: "#f0f2f5"` (bg) → `appCore.themeManager.background`
  - `color: "#1890ff"` (title) → `appCore.themeManager.primary`
  - `color: "#888"` (subtitle) → `appCore.themeManager.textTertiary`
  - `color: loginButton.enabled ? "#1890ff" : "#d9d9d9"` → `enabled ? appCore.themeManager.primary : appCore.themeManager.border`
  - `color: "#1890ff"` (register link) → `appCore.themeManager.primary`

  **RegisterPage.qml**:
  - `color: "#f0f2f5"` (bg) → `appCore.themeManager.background`
  - `color: "#333"` (title) → `appCore.themeManager.textPrimary`
  - `color: registerButton.enabled ? "#52c41a" : "#d9d9d9"` (register btn) → `enabled ? appCore.themeManager.success : appCore.themeManager.border`
  - `color: "#1890ff"` (back link) → `appCore.themeManager.primary`

  **Recommended Agent Profile**:
  - Category: `quick`
  - Skills: []

  **References**:
  - `client/qml/pages/LoginPage.qml`
  - `client/qml/pages/RegisterPage.qml`

  **Acceptance Criteria**:
  - [ ] Login page colors update when theme changes
  - [ ] Register page colors update when theme changes
  - [ ] Button colors match current theme

  **Commit**: YES (with tasks 5, 7-8 as group)


- [ ] 8. Update ChatPage.qml colors + font

  **What to do**:
  Replace hardcoded colors with theme bindings. Add font.family to all text elements.

  Key replacements:
  - `color: "#f0f2f5"` (page bg) → `appCore.themeManager.background`
  - `color: "#1890ff"` (header) → `appCore.themeManager.headerBg`
  - `color: "white"` (header text uses `textInverse`)
  - `color: friendReqBtn.hovered ? "#40a9ff" : "transparent"` → `appCore.themeManager.primaryLight` / transparent
  - `color: "#f7f7f7"` (tab bar) → `appCore.themeManager.surfaceAlt`
  - `color: leftTabBar.currentIndex === 0 ? "#1890ff" : "#666"` (tab text) → `currentIndex===0 ? appCore.themeManager.tabActive : appCore.themeManager.tabInactive`
  - `color: "#ebebeb"` (chat area bg) → `appCore.themeManager.background`
  - `color: "#333"` (text) → `appCore.themeManager.textPrimary`
  - `color: "#999"` (tertiary text) → `appCore.themeManager.textTertiary`
  - `color: "#ff4d4f"` (danger) → `appCore.themeManager.danger`
  - `color: parent.hovered ? "#e6f7ff" : "transparent"` → `appCore.themeManager.tabHoverBg` / transparent

  **Recommended Agent Profile**:
  - Category: `unspecified-high`
  - Skills: []

  **References**:
  - `client/qml/pages/ChatPage.qml`

  **Acceptance Criteria**:
  - [ ] Header color changes with theme
  - [ ] Tab colors change with theme
  - [ ] Background colors change with theme
  - [ ] All text colors use theme properties

  **Commit**: YES (with tasks 5-6, 8 as group)


- [ ] 9. Update MessageBubble.qml + GroupMessageBubble.qml

  **What to do**:
  Replace bubble colors with theme bindings. Add font.family to all text elements.

  **MessageBubble.qml**:
  - `color: isMine ? "#95ec69" : "white"` (bubble bg) → `isMine ? appCore.themeManager.bubbleMine : appCore.themeManager.bubbleOther`
  - `color: "#333"` (text) → `appCore.themeManager.textPrimary`
  - `border.color: "#e0e0e0"` → `appCore.themeManager.borderColor`
  - Status text colors already use runtime conditions - keep those
  - `color: "#999"` (timestamp) → `appCore.themeManager.textTertiary`
  - Apply `radius: appCore.themeManager.bubbleRadius`

  **GroupMessageBubble.qml**:
  - Same replacements as MessageBubble
  - `color: "#888"` (sender name) → `appCore.themeManager.textSecondary`
  - `leftPadding: appCore.themeManager.avatarItem.x...` ← keep hardcoded 52 for now

  **Recommended Agent Profile**:
  - Category: `visual-engineering`
  - Skills: []

  **References**:
  - `client/qml/components/MessageBubble.qml`
  - `client/qml/components/GroupMessageBubble.qml`

  **Acceptance Criteria**:
  - [ ] Bubble colors change with theme
  - [ ] Text colors change with theme/font
  - [ ] Bubble radius changes with font style

  **Commit**: YES (with tasks 5-7 as group)


- [ ] 10. Update Toast.qml + InputBar.qml + SearchBar.qml

  **What to do**:
  Replace hardcoded colors with theme bindings. Add font.family to text elements.
  - Background returns → use `appCore.themeManager.primaryBg`, etc.
  - But these are in JS functions. Need to use themeManager properties inside the functions.
  - Change from hardcoded returns to using `appCore.themeManager.XXX` in the function body

  Actually, simplest: change the function to use the themeManager properties:
  ```qml
  function getBgColor(type) {
      switch(type) {
          case "success": return appCore.themeManager.success
          case "error": return appCore.themeManager.danger
          ...
      }
  }
  ```

  But this won't re-evaluate when theme changes because it's a JS function. Better: use `Qt.binding()` or just inline the switch as a binding expression.

  **Recommended Agent Profile**:
  - Category: `quick`
  - Skills: []

  **References**:
  - `client/qml/components/Toast.qml`

  **Acceptance Criteria**:
  - [ ] Toast border and text colors update with theme

  **Commit**: YES (with tasks 10-12 as group)


- [ ] 11. Update ContactItem.qml, ConversationItem.qml, GroupConversationItem.qml

  **What to do**:
  Replace hardcoded colors with theme bindings. Add font.family to text elements.

  **InputBar.qml**: bg, border, text, button colors
  **SearchBar.qml**: bg, border, text colors
  **ContactItem.qml**: selection highlight, text colors
  **ConversationItem.qml**: selection, text, badge colors
  **GroupConversationItem.qml**: same as ConversationItem

  **Recommended Agent Profile**:
  - Category: `unspecified-low`
  - Skills: []

  **References**: Respective files in `client/qml/components/`

  **Acceptance Criteria**:
  - [ ] All component colors update with theme

  **Commit**: YES (with tasks 9, 11-12 as group)


- [ ] 12. Update all dialogs + FriendRequestPage.qml

  **What to do**:
  Replace hardcoded colors with theme bindings. Add font.family to text elements.
  - Button backgrounds (primary, danger, hover states)
  - Text colors
  - Border colors
  - List item alternating row colors (white/#fafafa)
  - Header colors for FriendRequestPage

  **Recommended Agent Profile**:
  - Category: `unspecified-low`
  - Skills: []

  **References**: Respective files in `client/qml/dialogs/` and `client/qml/pages/FriendRequestPage.qml`

  **Acceptance Criteria**:
  - [ ] Dialog colors update with theme

  **Commit**: YES (with tasks 9-10, 12 as group)


- [ ] 13. Font family pass - apply font family to ALL text elements

  **What to do**:
  Replace all hardcoded font families with `appCore.themeManager.fontFamily`.
  Target: every `Text`, `TextEdit`, `TextInput`, `Button`, etc. across all QML files.
  
  Pattern:
  ```qml
  // Before:
  Text { font.pixelSize: 14 }
  
  // After:
  Text { 
      font.pixelSize: appCore.themeManager.bodySize
      font.family: appCore.themeManager.fontFamily
  }
  ```
  
  Special cases:
  - ChatPage message list: use `bubbleSize` for bubble text
  - "ChatRoom" title on login: use `headerSize`
  - Timestamps, status text: use `smallSize`
  - Monospace areas (none in this project): skip

  For text elements inside `Button`, set `font.family` on the button directly or on `contentItem`.
  
  **Key**: All font references need BOTH `font.pixelSize` AND `font.family` updated from themeManager.

  **Recommended Agent Profile**:
  - Category: `unspecified-high`
  - Skills: []

  **Acceptance Criteria**:
  - [ ] All Text/TextEdit/TextInput/Button elements use `appCore.themeManager.fontFamily`
  - [ ] Switching font in Settings immediately updates all text rendering
  - [ ] Chinese characters render correctly for each font style
  - [ ] English characters render correctly for each font style
  - `font.pixelSize: 14` → `appCore.themeManager.bodySize` in most content areas
  - `font.pixelSize: 12` → `appCore.themeManager.smallSize` for labels
  - `font.pixelSize: 16` → `appCore.themeManager.headerSize` for headers
  - `font.pixelSize: 13` → either bodySize or smallSize depending on context
  - `font.pixelSize: 10` (timestamps) → use smallSize or keep small
  
  Focus on: ChatPage.qml, LoginPage.qml, RegisterPage.qml, SettingsPage.qml, MessageBubble.qml, GroupMessageBubble.qml

  **Recommended Agent Profile**:
  - Category: `unspecified-low`
  - Skills: []

  **Acceptance Criteria**:
  - [ ] Switching to "Large" font increases all text sizes
  - [ ] Switching to "Compact" decreases text sizes and spacing

  **Commit**: YES (with tasks 9-11 as group)


---

## Final Verification Wave

- [ ] F1. **Plan Compliance + Build** — `unspecified-high`
  Read the plan. Verify every task was completed. Run `cmake --build .` and verify zero compile errors.
  Output: `Build [PASS/FAIL] | Tasks [N/N complete] | VERDICT`

- [ ] F2. **Feature QA** — `unspecified-high` (+ Playwright skill)
  Start the app. Navigate to Settings. Click each of the 5 themes. Screenshot each. Verify colors change on main page, chat page (if logged in), and Login page. Switch each of 3 fonts. Verify text sizes change.
  Output: `Themes [5/5 verified] | Fonts [3/3 verified] | Evidence saved`

- [ ] F3. **Persistence Test** — `unspecified-high`
  Set a non-default theme and font. Close app. Reopen app. Verify the same theme and font are still active.
  Output: `Persistence [PASS/FAIL]`

- [ ] F4. **Smoke Test** — `unspecified-high`
  Quick regression: Login page loads, Settings opens, Chat page works (if logged in). No crashes when switching themes rapidly.
  Output: `Smoke [PASS/FAIL]`

---

## Commit Strategy

- Task 1: `feat(theme): add ThemeManager header with theme/font data structures`
- Task 2: `feat(theme): implement ThemeManager with 5 themes and 3 fonts`
- Task 3: `feat(theme): integrate ThemeManager into AppCore and build system`
- Tasks 4a-4f: `feat(theme): download, register, and load 3 font families`
- Task 5: `feat(settings): add theme and font picker to SettingsPage`
- Tasks 6-9: `feat(theme): apply theme colors + fonts to main pages and message bubbles`
- Tasks 10-13: `feat(theme): apply theme to components, dialogs, and remaining UI`

---

## Success Criteria

### Verification Commands
```bash
cd build/Desktop_Qt_6_11_0_MinGW_64_bit-Debug
cmake --build . --target ChatRoomClient
ChatRoomClient.exe
```

### Final Checklist
- [ ] 5 themes fully functional with color changes visible across all UI
- [ ] 3 font styles functional with size changes visible
- [ ] Settings page has intuitive theme/font picker
- [ ] Theme persists across restarts
- [ ] No compile errors
- [ ] No runtime crashes when switching themes
