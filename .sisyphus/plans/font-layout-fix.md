# Font & Layout Fix

## TL;DR
> Fix two persistent bugs: (1) font switching not working because QML font.family doesn't support comma-separated family lists, (2) Settings page content overflows its popup container.

---

## Problems

### Problem 1: Font switching doesn't work
`ThemeManager::fontFamily()` returns `"ZCOOL QingKe HuangYou, Caveat"` (comma-separated). QML's `font.family` treats this as a **single font name** `"ZCOOL QingKe HuangYou, Caveat"` — which doesn't exist in the font database. Result: text always uses default font regardless of selection.

### Problem 2: SettingsPage overflows
`SettingsPopup` in ChatPage is `width: 350, height: 300`. With Fusion style's GroupBox padding and all content (Account + Theme swatches + Font cards + buttons), the page needs ~550px vertically.

---

## TODOs

- [ ] 1. Fix fontFamily() to return single font name

  **What to do**: In `client/src/qmlbridge/ThemeManager.cpp`, change the `fontFamily()` getter to return only the first font family name before the comma:
  ```cpp
  // Before:
  QString ThemeManager::fontFamily() const { return currentFont().family; }
  // After:
  QString ThemeManager::fontFamily() const { return currentFont().family.section(", ", 0, 0); }
  ```
  This makes `fontFamily()` return `"ZCOOL QingKe HuangYou"` instead of `"ZCOOL QingKe HuangYou, Caveat"`. Qt's text engine handles CJK/Latin fallback internally.
  
  Keep `applyGlobalFont()` unchanged — C++ `QFont::setFamilies()` correctly handles the comma-separated list.

  **Must NOT do**: Do not modify FontConfig.family values — `applyGlobalFont()` still needs the full list.

  **QA Scenarios**:
  ```
  Scenario: fontFamily returns single name
    Tool: Bash (REPL)
    Steps:
      1. Verify in Qt debug output that setFontIndex(0) → fontFamily() returns "ZCOOL QingKe HuangYou" (no comma)
      2. Check that QML StyledText binding resolves the single name correctly (no QML errors)
    ```

- [ ] 2. Fix SettingsPopup size in ChatPage.qml

  **What to do**: In `client/qml/pages/ChatPage.qml`, find the `SettingsPopup` definition (around line 780) and increase its size:
  ```qml
  Popup {
      id: settingsPopup
      width: Math.min(parent.width * 0.85, 450)
      height: Math.min(parent.height * 0.85, 520)
      anchors.centerIn: parent
      modal: true
      closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
  ```
  This makes the popup scale with parent but cap at a reasonable max.

  **QA Scenarios**:
  ```
  Scenario: Settings popup fits content
    Tool: Playwright
    Steps:
      1. Launch app and log in
      2. Click settings button
      3. Verify popup is large enough to see all content without scrolling
    Expected: Popup shows full content (Account, Theme, Font, buttons)
    ```

- [ ] 3. Add ScrollView to SettingsPage content

  **What to do**: In `client/qml/pages/SettingsPage.qml`, wrap the settings content ColumnLayout in a ScrollView so content scrolls if the popup is resized smaller:
  ```qml
  // Replace the inner ColumnLayout (the settings content) with:
  ScrollView {
      Layout.fillWidth: true
      Layout.fillHeight: true
      clip: true
      ScrollBar.vertical.policy: ScrollBar.AsNeeded
      
      ColumnLayout {
          width: parent.width
          Layout.margins: 16
          spacing: 16
          // ...existing GroupBoxes and buttons...
      }
  }
  ```
  
  **Pattern Reference**: Look at `ChatPage.qml` lines 544-604 for ListView scroll bar pattern.
  
  **Must NOT do**: Don't modify the header Rectangle or outer structure.

  **QA Scenarios**:
  ```
  Scenario: Content scrolls when popup resized
    Tool: Playwright
    Steps:
      1. Log in, open settings
      2. Verify scrollbar appears when content exceeds visible area
      3. Scroll down to see Logout button
    Expected: All content reachable via scroll
    ```
