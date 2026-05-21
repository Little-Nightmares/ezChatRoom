#include "ThemeManager.h"
#include <QGuiApplication>
#include <QFont>

namespace chatroom::client {

static void applyGlobalFont(const FontConfig& cfg)
{
    QFont f;
    QStringList families = cfg.family.split(", ");
    if (!families.isEmpty())
        f.setFamilies(families);
    f.setPixelSize(cfg.bodySize);
    QGuiApplication::setFont(f);
}

// ---------------------------------------------------------------------------
// Theme data: 5 color palettes
// ---------------------------------------------------------------------------
static const ThemePalette s_themes[] = {
    // Theme 0 - Sakura Pink
    {
        u8"\U0001F338 \u6A31\u82B1",
        QColor("#ff6b81"), QColor("#ff8a9e"), QColor("#fff0f3"), QColor("#fff0f3"),
        QColor("#fff5f7"), QColor("#ffffff"), QColor("#fafafa"),
        QColor("#ffdde3"), QColor("#ffe8ec"),
        QColor("#ffdde3"), QColor("#ffffff"),
        QColor("#333333"), QColor("#333333"),
        QColor("#333333"), QColor("#666666"), QColor("#999999"),
        QColor("#ffffff"),
        QColor("#52c41a"), QColor("#faad14"), QColor("#fff7e6"), QColor("#ff4d4f"), QColor("#ffccc7"),
        QColor("#ff6b81"), QColor("#ffffff"),
        QColor("#ff6b81"), QColor("#666666"), QColor("#fff0f3"), QColor("#ffffff"),
    },
    // Theme 1 - Forest Green
    {
        u8"\U0001F332 \u68EE\u6797",
        QColor("#2d8a4e"), QColor("#4ecb71"), QColor("#edf7f0"), QColor("#edf7f0"),
        QColor("#f0f7f0"), QColor("#ffffff"), QColor("#fafafa"),
        QColor("#c8e6c9"), QColor("#dcedc8"),
        QColor("#b8e6b8"), QColor("#ffffff"),
        QColor("#333333"), QColor("#333333"),
        QColor("#333333"), QColor("#666666"), QColor("#999999"),
        QColor("#ffffff"),
        QColor("#2d8a4e"), QColor("#faad14"), QColor("#fff7e6"), QColor("#ff4d4f"), QColor("#ffccc7"),
        QColor("#2d8a4e"), QColor("#ffffff"),
        QColor("#2d8a4e"), QColor("#666666"), QColor("#edf7f0"), QColor("#ffffff"),
    },
    // Theme 2 - Aurora Purple
    {
        u8"\U0001F30C \u6781\u5149",
        QColor("#6c5ce7"), QColor("#8f7ff0"), QColor("#f0edff"), QColor("#f0edff"),
        QColor("#f4f0ff"), QColor("#ffffff"), QColor("#fafafa"),
        QColor("#d4c5ff"), QColor("#e0d6ff"),
        QColor("#d4c5ff"), QColor("#ffffff"),
        QColor("#333333"), QColor("#333333"),
        QColor("#333333"), QColor("#666666"), QColor("#999999"),
        QColor("#ffffff"),
        QColor("#52c41a"), QColor("#faad14"), QColor("#fff7e6"), QColor("#ff4d4f"), QColor("#ffccc7"),
        QColor("#6c5ce7"), QColor("#ffffff"),
        QColor("#6c5ce7"), QColor("#666666"), QColor("#f0edff"), QColor("#ffffff"),
    },
    // Theme 3 - Matcha
    {
        u8"\U0001F375 \u62B9\u8336",
        QColor("#9acd32"), QColor("#b8e050"), QColor("#f4fae0"), QColor("#f4fae0"),
        QColor("#f7fce8"), QColor("#ffffff"), QColor("#fafafa"),
        QColor("#d4ed8a"), QColor("#e2f0b0"),
        QColor("#d4ed8a"), QColor("#ffffff"),
        QColor("#333333"), QColor("#333333"),
        QColor("#333333"), QColor("#666666"), QColor("#999999"),
        QColor("#ffffff"),
        QColor("#9acd32"), QColor("#faad14"), QColor("#fff7e6"), QColor("#ff4d4f"), QColor("#ffccc7"),
        QColor("#9acd32"), QColor("#ffffff"),
        QColor("#9acd32"), QColor("#666666"), QColor("#f4fae0"), QColor("#ffffff"),
    },
    // Theme 4 - Ocean Teal
    {
        u8"\U0001F30A \u6D77\u6D0B",
        QColor("#00b894"), QColor("#33d9b2"), QColor("#e0f8f4"), QColor("#e0f8f4"),
        QColor("#f0faf8"), QColor("#ffffff"), QColor("#fafafa"),
        QColor("#b2dfdb"), QColor("#c8e6de"),
        QColor("#b2dfdb"), QColor("#ffffff"),
        QColor("#333333"), QColor("#333333"),
        QColor("#333333"), QColor("#666666"), QColor("#999999"),
        QColor("#ffffff"),
        QColor("#00b894"), QColor("#faad14"), QColor("#fff7e6"), QColor("#ff4d4f"), QColor("#ffccc7"),
        QColor("#00b894"), QColor("#ffffff"),
        QColor("#00b894"), QColor("#666666"), QColor("#e0f8f4"), QColor("#ffffff"),
    },
};

static const int s_themeCount = sizeof(s_themes) / sizeof(s_themes[0]);

// ---------------------------------------------------------------------------
// Font data: 3 font styles
// ---------------------------------------------------------------------------
static const FontConfig s_fonts[] = {
    // Font 0 - Handwritten
    {
        u8"\U0001F4DD \u624B\u5199\u4F53",
        "ZCOOL QingKe HuangYou, Caveat",
        "Caveat, ZCOOL QingKe HuangYou",
        14, 12, 14, 16, 8, 8,
    },
    // Font 1 - Standard Sans
    {
        u8"\U0001F3AF \u6807\u51C6\u4F53",
        "Noto Sans SC, Inter",
        "Inter, Noto Sans SC",
        14, 12, 14, 16, 8, 8,
    },
    // Font 2 - Rounded
    {
        u8"\U0001FAE7 \u5706\u6DA6\u4F53",
        "M PLUS Rounded 1c, Nunito",
        "Nunito, M PLUS Rounded 1c",
        14, 12, 14, 16, 8, 8,
    },
};

static const int s_fontCount = sizeof(s_fonts) / sizeof(s_fonts[0]);

// ---------------------------------------------------------------------------
// ThemeManager
// ---------------------------------------------------------------------------
ThemeManager::ThemeManager(QObject* parent)
    : QObject(parent)
{
    QSettings settings("ChatRoom", "ChatRoom");
    m_themeIndex = qBound(0, settings.value("theme/themeIndex", 0).toInt(), s_themeCount - 1);
    m_fontIndex  = qBound(0, settings.value("theme/fontIndex", 0).toInt(), s_fontCount - 1);

    // Apply saved font globally to all QML text elements
    applyGlobalFont(currentFont());
}

int ThemeManager::themeIndex() const { return m_themeIndex; }
int ThemeManager::fontIndex() const  { return m_fontIndex; }

void ThemeManager::setThemeIndex(int index)
{
    if (index == m_themeIndex) return;
    m_themeIndex = qBound(0, index, s_themeCount - 1);
    saveSettings();
    emit themeChanged();
}

void ThemeManager::setFontIndex(int index)
{
    if (index == m_fontIndex) return;
    m_fontIndex = qBound(0, index, s_fontCount - 1);
    saveSettings();
    applyGlobalFont(currentFont());
    emit fontChanged();
}

// ---------------------------------------------------------------------------
// Theme color accessors
// ---------------------------------------------------------------------------
const ThemePalette& ThemeManager::currentTheme() const { return s_themes[m_themeIndex]; }
const FontConfig&   ThemeManager::currentFont() const  { return s_fonts[m_fontIndex]; }

const ThemePalette& ThemeManager::theme(int index) { return s_themes[index]; }
const FontConfig&   ThemeManager::fontCfg(int index) { return s_fonts[index]; }

QString ThemeManager::themeName()  const { return currentTheme().name; }
QColor  ThemeManager::primary()     const { return currentTheme().primary; }
QColor  ThemeManager::primaryLight()  const { return currentTheme().primaryLight; }
QColor  ThemeManager::primaryHover()  const { return currentTheme().primaryHover; }
QColor  ThemeManager::primaryBg()     const { return currentTheme().primaryBg; }
QColor  ThemeManager::background()    const { return currentTheme().background; }
QColor  ThemeManager::surface()       const { return currentTheme().surface; }
QColor  ThemeManager::surfaceAlt()    const { return currentTheme().surfaceAlt; }
QColor  ThemeManager::borderColor()   const { return currentTheme().border; }
QColor  ThemeManager::divider()       const { return currentTheme().divider; }
QColor  ThemeManager::bubbleMine()    const { return currentTheme().bubbleMine; }
QColor  ThemeManager::bubbleOther()   const { return currentTheme().bubbleOther; }
QColor  ThemeManager::bubbleMineText()  const { return currentTheme().bubbleMineText; }
QColor  ThemeManager::bubbleOtherText() const { return currentTheme().bubbleOtherText; }
QColor  ThemeManager::textPrimary()   const { return currentTheme().textPrimary; }
QColor  ThemeManager::textSecondary() const { return currentTheme().textSecondary; }
QColor  ThemeManager::textTertiary()  const { return currentTheme().textTertiary; }
QColor  ThemeManager::textInverse()   const { return currentTheme().textInverse; }
QColor  ThemeManager::success()       const { return currentTheme().success; }
QColor  ThemeManager::warning()       const { return currentTheme().warning; }
QColor  ThemeManager::danger()        const { return currentTheme().danger; }
QColor  ThemeManager::warningLight()  const { return currentTheme().warningLight; }
QColor  ThemeManager::dangerLight()   const { return currentTheme().dangerLight; }
QColor  ThemeManager::headerBg()      const { return currentTheme().headerBg; }
QColor  ThemeManager::headerText()    const { return currentTheme().headerText; }
QColor  ThemeManager::tabActive()     const { return currentTheme().tabActive; }
QColor  ThemeManager::tabInactive()   const { return currentTheme().tabInactive; }
QColor  ThemeManager::tabHoverBg()    const { return currentTheme().tabHoverBg; }
QColor  ThemeManager::tabActiveBg()   const { return currentTheme().tabActiveBg; }

// ---------------------------------------------------------------------------
// Font accessors
// ---------------------------------------------------------------------------
QString ThemeManager::fontFamily()        const { return currentFont().family.section(", ", 0, 0); }
int    ThemeManager::bodySize()           const { return currentFont().bodySize; }
int    ThemeManager::smallSize()          const { return currentFont().smallSize; }
int    ThemeManager::bubbleSize()         const { return currentFont().bubbleSize; }
int    ThemeManager::headerSize()         const { return currentFont().headerSize; }
int    ThemeManager::messageSpacing()     const { return currentFont().messageSpacing; }
int    ThemeManager::bubbleRadius()       const { return currentFont().bubbleRadius; }

// ---------------------------------------------------------------------------
// List helpers for Settings page
// ---------------------------------------------------------------------------
QStringList ThemeManager::themeNames() const
{
    QStringList names;
    for (int i = 0; i < s_themeCount; ++i)
        names << s_themes[i].name;
    return names;
}

QStringList ThemeManager::fontNames() const
{
    QStringList names;
    for (int i = 0; i < s_fontCount; ++i)
        names << s_fonts[i].name;
    return names;
}

// ---------------------------------------------------------------------------
// Persistence
// ---------------------------------------------------------------------------
void ThemeManager::saveSettings()
{
    QSettings settings("ChatRoom", "ChatRoom");
    settings.setValue("theme/themeIndex", m_themeIndex);
    settings.setValue("theme/fontIndex", m_fontIndex);
}

} // namespace chatroom::client
